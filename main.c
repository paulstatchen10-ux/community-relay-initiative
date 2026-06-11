/*
 * Zero-Knowledge Presence Relay Firmware
 * Target: Nordic nRF9160 SiP (Actinius Icarus / Nordic Thingy:91)
 * RTOS: Zephyr RTOS via nRF Connect SDK
 *
 * Architecture:
 *   - PSM deep sleep until GPIO button interrupt
 *   - Wakes, connects LTE-M/NB-IoT via eSIM
 *   - Sends HTTPS POST with fixed "Presence Verified" token ONLY
 *   - NO location, NO device ID beyond token, NO telemetry
 *   - Modem comms isolated in SPE (Secure Processing Environment)
 *     via Arm TrustZone / SPM; app layer runs in NSPE
 *   - Returns to PSM after handshake completes or fails (timeout)
 *
 * Privacy Guarantees:
 *   - Token is static and rotatable server-side; conveys zero PII
 *   - Payload schema: { "status": "presence_verified", "token": "<BEARER>" }
 *   - No GPS, no IMEI, no cell-tower triangulation data transmitted
 *
 * Author: Paul Statchen / Civic OS Project
 * License: Apache 2.0
 */

#include <zephyr/kernel.h>
#include <zephyr/device.h>
#include <zephyr/drivers/gpio.h>
#include <zephyr/sys/printk.h>
#include <zephyr/sys/reboot.h>
#include <zephyr/net/socket.h>
#include <zephyr/net/tls_credentials.h>
#include <zephyr/logging/log.h>
#include <modem/nrf_modem_lib.h>
#include <modem/lte_lc.h>
#include <modem/modem_info.h>
#include <net/http_client.h>
#include <nrf_modem_at.h>

LOG_MODULE_REGISTER(presence_relay, LOG_LEVEL_INF);

/* -----------------------------------------------------------------------
 * Configuration - Update these for your deployment
 * ----------------------------------------------------------------------- */

/* Server endpoint - replace with your sovereign backend */
#define SERVER_HOST         "your-backend.example.org"
#define SERVER_PORT         443
#define SERVER_PATH         "/api/v1/presence"

/*
 * Presence token: a pre-shared bearer token agreed with the server.
 * This is the ONLY identifier transmitted. Rotate this server-side
 * periodically. The token encodes nothing about the user or location.
 * Store this in secure storage (Protected Storage API) in production.
 */
#define PRESENCE_TOKEN      "REPLACE_WITH_DEPLOYMENT_TOKEN"

/* LTE network mode: LTE_LC_LTE_MODE_LTEM or LTE_LC_LTE_MODE_NBIOT */
#define PREFERRED_LTE_MODE  LTE_LC_LTE_MODE_LTEM

/* Wake timeout: how long we stay awake before forcing return to PSM */
#define WAKE_TIMEOUT_S      60

/* PSM parameters: T3412 (periodic TAU), T3324 (active timer)
 * T3324 = "000 00001" = 2 seconds active window after data
 * T3412 = "001 00001" = 1 hour periodic TAU
 * These are requested values; network grants what it supports.
 */
#define PSM_TAU_REQUEST     "00100001"   /* T3412: 1 hour */
#define PSM_ACTIVE_REQUEST  "00000001"   /* T3324: 2 seconds */

/* TLS credential tag for the server CA certificate */
#define TLS_CA_TAG          42

/* Button GPIO - adjust for your board */
#define BUTTON_NODE         DT_ALIAS(sw0)
#define BUTTON_PIN          DT_GPIO_PIN(BUTTON_NODE, gpios)
#define BUTTON_FLAGS        DT_GPIO_FLAGS(BUTTON_NODE, gpios)
#define BUTTON_CTRL         DT_GPIO_CTLR(BUTTON_NODE, gpios)

/* -----------------------------------------------------------------------
 * Server CA Certificate (PEM)
 * Replace with the actual CA cert for your server.
 * This is embedded at compile time - update for production.
 * ----------------------------------------------------------------------- */
static const char server_ca_cert[] =
    "-----BEGIN CERTIFICATE-----\n"
    "REPLACE_WITH_YOUR_SERVER_CA_CERT_PEM\n"
    "-----END CERTIFICATE-----\n";

/* -----------------------------------------------------------------------
 * Globals & Synchronization Primitives
 * ----------------------------------------------------------------------- */
static struct gpio_callback button_cb_data;
static K_SEM_DEFINE(button_sem, 0, 1);
static K_SEM_DEFINE(lte_connected_sem, 0, 1);
static atomic_t lte_connected = ATOMIC_INIT(0);

/* -----------------------------------------------------------------------
 * HTTP Payload Buffer
 * No dynamic allocation - fixed stack buffers only.
 * ----------------------------------------------------------------------- */
#define HTTP_RECV_BUF_SIZE  512
#define HTTP_PAYLOAD_SIZE   128
static uint8_t http_recv_buf[HTTP_RECV_BUF_SIZE];
static char    http_payload[HTTP_PAYLOAD_SIZE];

/* -----------------------------------------------------------------------
 * Button ISR - fires from GPIO interrupt, signals main thread
 * Runs in ISR context; keep it minimal.
 * ----------------------------------------------------------------------- */
static void button_isr_handler(const struct device *dev,
                               struct gpio_callback *cb,
                               uint32_t pins)
{
    ARG_UNUSED(dev);
    ARG_UNUSED(cb);
    ARG_UNUSED(pins);

    /* Signal main thread; main loop handles debounce */
    k_sem_give(&button_sem);
}

/* -----------------------------------------------------------------------
 * GPIO Init - Configure button as interrupt source
 * ----------------------------------------------------------------------- */
static int button_init(void)
{
    const struct device *btn_dev = DEVICE_DT_GET(BUTTON_CTRL);

    if (!device_is_ready(btn_dev)) {
        LOG_ERR("Button GPIO device not ready");
        return -ENODEV;
    }

    int ret = gpio_pin_configure(btn_dev, BUTTON_PIN,
                                 GPIO_INPUT | BUTTON_FLAGS);
    if (ret < 0) {
        LOG_ERR("Failed to configure button GPIO: %d", ret);
        return ret;
    }

    ret = gpio_pin_interrupt_configure(btn_dev, BUTTON_PIN,
                                       GPIO_INT_EDGE_TO_ACTIVE);
    if (ret < 0) {
        LOG_ERR("Failed to configure button interrupt: %d", ret);
        return ret;
    }

    gpio_init_callback(&button_cb_data, button_isr_handler,
                       BIT(BUTTON_PIN));
    gpio_add_callback(btn_dev, &button_cb_data);

    LOG_INF("Button interrupt configured on pin %d", BUTTON_PIN);
    return 0;
}

/* -----------------------------------------------------------------------
 * LTE Event Handler - tracks network attachment state
 * ----------------------------------------------------------------------- */
static void lte_event_handler(const struct lte_lc_evt *const evt)
{
    switch (evt->type) {
    case LTE_LC_EVT_NW_REG_STATUS:
        if (evt->nw_reg_status == LTE_LC_NW_REG_REGISTERED_HOME ||
            evt->nw_reg_status == LTE_LC_NW_REG_REGISTERED_ROAMING) {
            LOG_INF("LTE registered (status: %d)", evt->nw_reg_status);
            atomic_set(&lte_connected, 1);
            k_sem_give(&lte_connected_sem);
        } else if (evt->nw_reg_status == LTE_LC_NW_REG_NOT_REGISTERED) {
            LOG_WRN("LTE not registered");
            atomic_set(&lte_connected, 0);
        }
        break;

    case LTE_LC_EVT_PSM_UPDATE:
        LOG_INF("PSM update: TAU=%d s, Active=%d s",
                evt->psm_cfg.tau, evt->psm_cfg.active_time);
        break;

    case LTE_LC_EVT_EDRX_UPDATE:
        /* eDRX is secondary power-save mechanism; log but don't rely on it */
        LOG_DBG("eDRX update received");
        break;

    case LTE_LC_EVT_RRC_UPDATE:
        LOG_DBG("RRC mode: %s",
                evt->rrc_mode == LTE_LC_RRC_MODE_CONNECTED
                    ? "Connected" : "Idle");
        break;

    case LTE_LC_EVT_CELL_UPDATE:
        /*
         * PRIVACY NOTE: Cell update gives us cell ID and TAC.
         * We deliberately do NOT log or transmit this.
         * It is received by the modem internally for network ops only.
         */
        break;

    default:
        break;
    }
}

/* -----------------------------------------------------------------------
 * Modem Init - bring up LTE-M with PSM configured
 * This runs in the NSPE (Non-Secure Processing Environment / app layer).
 * The actual modem firmware runs in SPE (Secure Processing Environment)
 * and is inaccessible to the application. TrustZone enforces this split.
 * ----------------------------------------------------------------------- */
static int modem_init_and_connect(void)
{
    int ret;

    LOG_INF("Initializing modem library...");
    ret = nrf_modem_lib_init();
    if (ret) {
        LOG_ERR("Modem lib init failed: %d", ret);
        return ret;
    }

    /* Set preferred LTE mode */
    ret = lte_lc_system_mode_set(
        (PREFERRED_LTE_MODE == LTE_LC_LTE_MODE_LTEM)
            ? LTE_LC_SYSTEM_MODE_LTEM_NBIOT
            : LTE_LC_SYSTEM_MODE_NBIOT,
        PREFERRED_LTE_MODE);
    if (ret) {
        LOG_ERR("Failed to set LTE mode: %d", ret);
        return ret;
    }

    /* Request PSM - network may grant different values */
    ret = lte_lc_psm_req(true);
    if (ret) {
        LOG_ERR("PSM request failed: %d", ret);
        /* Non-fatal: continue without PSM guarantee */
    }

    /* Set PSM timer strings via AT commands */
    ret = nrf_modem_at_printf("AT+CPSMS=1,,,\"%s\",\"%s\"",
                               PSM_TAU_REQUEST, PSM_ACTIVE_REQUEST);
    if (ret) {
        LOG_WRN("AT CPSMS command failed (non-fatal): %d", ret);
    }

    /* Register event handler and initiate LTE connection */
    ret = lte_lc_init_and_connect_async(lte_event_handler);
    if (ret) {
        LOG_ERR("LTE connect async failed: %d", ret);
        return ret;
    }

    LOG_INF("Waiting for LTE registration (timeout: %d s)...",
            WAKE_TIMEOUT_S);

    /* Block until registered or timeout */
    ret = k_sem_take(&lte_connected_sem, K_SECONDS(WAKE_TIMEOUT_S));
    if (ret == -EAGAIN) {
        LOG_ERR("LTE registration timed out");
        return -ETIMEDOUT;
    }

    LOG_INF("LTE connected");
    return 0;
}

/* -----------------------------------------------------------------------
 * TLS Credential Setup - load CA cert into modem security store
 * ----------------------------------------------------------------------- */
static int tls_setup(void)
{
    int ret;

    ret = tls_credential_add(TLS_CA_TAG,
                             TLS_CREDENTIAL_CA_CERTIFICATE,
                             server_ca_cert,
                             sizeof(server_ca_cert));
    if (ret == -EEXIST) {
        /* Already loaded from a previous wake cycle */
        LOG_DBG("CA cert already in credential store");
        return 0;
    }
    if (ret < 0) {
        LOG_ERR("Failed to add CA cert: %d", ret);
        return ret;
    }

    LOG_INF("CA certificate loaded (tag: %d)", TLS_CA_TAG);
    return 0;
}

/* -----------------------------------------------------------------------
 * HTTP Response Callback
 * ----------------------------------------------------------------------- */
static void http_response_cb(struct http_response *rsp,
                             enum http_final_call final_data,
                             void *user_data)
{
    ARG_UNUSED(user_data);

    if (final_data == HTTP_DATA_FINAL) {
        LOG_INF("Server response: HTTP %d", rsp->http_status_code);
        if (rsp->http_status_code == 200 ||
            rsp->http_status_code == 201 ||
            rsp->http_status_code == 204) {
            LOG_INF("Presence relay: ACK received from server");
        } else {
            LOG_WRN("Server returned non-success: %d",
                    rsp->http_status_code);
        }
    }
}

/* -----------------------------------------------------------------------
 * Send Presence Token via HTTPS POST
 *
 * Payload: { "status": "presence_verified", "token": "<BEARER>" }
 *
 * This is the COMPLETE payload. No location. No device fingerprint.
 * No timestamp (server records receipt time server-side if needed).
 * ----------------------------------------------------------------------- */
static int send_presence_token(void)
{
    int ret;
    int sock = -1;
    struct addrinfo *addr;
    struct addrinfo hints = {
        .ai_family   = AF_INET,
        .ai_socktype = SOCK_STREAM,
    };

    /* Build JSON payload */
    int payload_len = snprintk(http_payload, sizeof(http_payload),
        "{\"status\":\"presence_verified\",\"token\":\"%s\"}",
        PRESENCE_TOKEN);

    if (payload_len < 0 || payload_len >= sizeof(http_payload)) {
        LOG_ERR("Payload buffer overflow");
        return -ENOMEM;
    }

    /* Resolve server hostname */
    ret = getaddrinfo(SERVER_HOST, NULL, &hints, &addr);
    if (ret) {
        LOG_ERR("DNS resolution failed for %s: %d", SERVER_HOST, ret);
        return -EIO;
    }

    /* Set port on resolved address */
    ((struct sockaddr_in *)addr->ai_addr)->sin_port = htons(SERVER_PORT);

    /* Open TLS socket */
    sec_tag_t sec_tag_list[] = { TLS_CA_TAG };

    sock = socket(addr->ai_family, SOCK_STREAM, IPPROTO_TLS_1_2);
    if (sock < 0) {
        LOG_ERR("Socket creation failed: %d", errno);
        freeaddrinfo(addr);
        return -errno;
    }

    /* Apply TLS options */
    ret = setsockopt(sock, SOL_TLS, TLS_SEC_TAG_LIST,
                     sec_tag_list, sizeof(sec_tag_list));
    if (ret < 0) {
        LOG_ERR("setsockopt TLS_SEC_TAG_LIST failed: %d", errno);
        goto cleanup;
    }

    ret = setsockopt(sock, SOL_TLS, TLS_HOSTNAME,
                     SERVER_HOST, strlen(SERVER_HOST));
    if (ret < 0) {
        LOG_ERR("setsockopt TLS_HOSTNAME failed: %d", errno);
        goto cleanup;
    }

    /* TLS_PEER_VERIFY: enforce server certificate validation */
    int verify = TLS_PEER_VERIFY_REQUIRED;
    ret = setsockopt(sock, SOL_TLS, TLS_PEER_VERIFY,
                     &verify, sizeof(verify));
    if (ret < 0) {
        LOG_ERR("setsockopt TLS_PEER_VERIFY failed: %d", errno);
        goto cleanup;
    }

    /* Connect */
    ret = connect(sock, addr->ai_addr, addr->ai_addrlen);
    freeaddrinfo(addr);
    addr = NULL;

    if (ret < 0) {
        LOG_ERR("TLS connect failed: %d", errno);
        goto cleanup;
    }

    LOG_INF("TLS handshake complete with %s:%d", SERVER_HOST, SERVER_PORT);

    /* Build HTTP request */
    const char *headers[] = {
        "Content-Type: application/json\r\n",
        "Authorization: Bearer " PRESENCE_TOKEN "\r\n",
        "Connection: close\r\n",
        NULL
    };

    struct http_request req = {
        .method          = HTTP_POST,
        .url             = SERVER_PATH,
        .host            = SERVER_HOST,
        .protocol        = "HTTP/1.1",
        .payload         = http_payload,
        .payload_len     = payload_len,
        .header_fields   = headers,
        .response        = http_response_cb,
        .recv_buf        = http_recv_buf,
        .recv_buf_len    = sizeof(http_recv_buf),
    };

    /* Send - 10 second timeout */
    ret = http_client_req(sock, &req, 10000, NULL);
    if (ret < 0) {
        LOG_ERR("HTTP request failed: %d", ret);
    }

cleanup:
    if (sock >= 0) {
        close(sock);
    }
    if (addr) {
        freeaddrinfo(addr);
    }
    return ret;
}

/* -----------------------------------------------------------------------
 * Return to PSM Deep Sleep
 * ----------------------------------------------------------------------- */
static void enter_psm_sleep(void)
{
    LOG_INF("Requesting PSM sleep...");

    /* Disconnect gracefully; modem will enter PSM */
    lte_lc_offline();

    /*
     * For lowest power after data transfer:
     * Put modem into flight mode - it will wake on next button press
     * which re-triggers the full wake/connect/send cycle.
     */
    lte_lc_func_mode_set(LTE_LC_FUNC_MODE_DEACTIVATE_LTE);

    LOG_INF("Modem deactivated. System entering sleep.");
    LOG_INF("Waiting for next button press...");

    /*
     * The MCU stays in a low-power wait state here.
     * The GPIO interrupt will fire on next button press,
     * signal the semaphore, and break the k_sem_take below.
     */
}

/* -----------------------------------------------------------------------
 * Main Entry Point
 * ----------------------------------------------------------------------- */
void main(void)
{
    int ret;

    LOG_INF("=================================================");
    LOG_INF(" Zero-Knowledge Presence Relay v1.0");
    LOG_INF(" Civic OS Project / Santa Cruz County");
    LOG_INF(" Privacy mode: MAXIMUM (no PII transmitted)");
    LOG_INF("=================================================");

    /*
     * TrustZone Note:
     * The nRF9160 boots with the Secure Processing Environment (SPE)
     * active. The Nordic Secure Partition Manager (SPM) or TF-M
     * (Trusted Firmware-M) is flashed separately and configures
     * TrustZone peripheral partitioning at boot.
     *
     * This application runs entirely in the Non-Secure Processing
     * Environment (NSPE). It cannot directly access:
     *   - Modem UART (UARTE0/UARTE1 in secure partition)
     *   - KMU (Key Management Unit)
     *   - CC312 (Arm CryptoCell)
     *   - Flash regions marked secure
     *
     * All modem AT commands route through the IPC shared memory
     * interface (nrf_modem) which is the designed secure channel.
     * The application layer physically cannot exfiltrate raw modem
     * data; it only receives what the modem library exposes via API.
     */

    /* Initialize button GPIO interrupt */
    ret = button_init();
    if (ret) {
        LOG_ERR("FATAL: Button init failed. Halting.");
        return;
    }

    /* Load TLS credentials once at boot */
    ret = tls_setup();
    if (ret) {
        LOG_ERR("FATAL: TLS setup failed. Halting.");
        return;
    }

    LOG_INF("System ready. Press button to send presence signal.");

    /* ---------------------------------------------------------------
     * Main Loop: sleep -> wake on button -> connect -> send -> sleep
     * --------------------------------------------------------------- */
    while (1) {

        /* Block indefinitely waiting for button press */
        LOG_INF("Entering low-power wait (GPIO interrupt armed)");
        k_sem_take(&button_sem, K_FOREVER);

        /* Debounce: ignore rapid re-presses for 500ms */
        k_sleep(K_MSEC(500));

        /* Drain any extra presses during debounce window */
        while (k_sem_take(&button_sem, K_NO_WAIT) == 0) {}

        LOG_INF("Button press detected - initiating presence relay");

        /* Connect LTE */
        ret = modem_init_and_connect();
        if (ret) {
            LOG_ERR("LTE connect failed (%d) - returning to sleep", ret);
            /* Re-initialize modem lib for next attempt */
            nrf_modem_lib_shutdown();
            continue;
        }

        /* Send presence token */
        ret = send_presence_token();
        if (ret < 0) {
            LOG_ERR("Presence send failed (%d)", ret);
        } else {
            LOG_INF("Presence token delivered successfully");
        }

        /* Shutdown modem library before sleeping */
        nrf_modem_lib_shutdown();

        /*
         * Brief LED indication of success/failure here
         * (add LED GPIO control as needed for your board)
         */

        /* Return to PSM */
        enter_psm_sleep();
    }
}
