# Presence Relay - Build & Flash Guide
## Crostini Linux Environment (ChromeOS / Zephyr / nRF Connect SDK)

---

## PHASE 0: Crostini Prerequisites

These commands install base system deps inside your Crostini container.
Run once on a fresh setup.

```bash
# Update package lists
sudo apt update && sudo apt upgrade -y

# Essential build tools
sudo apt install -y \
    git cmake ninja-build gperf \
    ccache dfu-util device-tree-compiler \
    python3-pip python3-setuptools python3-wheel \
    python3-venv \
    libsdl2-dev \
    wget curl unzip \
    usbutils

# Verify cmake version (must be >= 3.20)
cmake --version
```

---

## PHASE 1: Install nRF Connect SDK via `west`

```bash
# Create a dedicated workspace (NOT inside your home drive shortcut;
# Crostini ext4 filesystem performs better than the Linux folder mount)
mkdir -p ~/ncs && cd ~/ncs

# Install west (nRF/Zephyr meta-tool)
pip3 install --user west

# Add ~/.local/bin to PATH if not already there
echo 'export PATH=$HOME/.local/bin:$PATH' >> ~/.bashrc
source ~/.bashrc

# Verify west is accessible
west --version

# Initialize workspace pointing at nRF Connect SDK v2.6.1
# (matches the version pinned in west.yml)
west init -m https://github.com/nrfconnect/sdk-nrf --mr v2.6.1

# Pull all dependencies (Zephyr, nrf, mcuboot, etc.)
# This downloads ~5-8 GB; run over good WiFi or ethernet via Crostini
west update

# Export Zephyr CMake package
west zephyr-export

# Install Python requirements for all repos
pip3 install --user -r zephyr/scripts/requirements.txt
pip3 install --user -r nrf/scripts/requirements.txt
pip3 install --user -r bootloader/mcuboot/scripts/requirements.txt

echo "=== nRF Connect SDK v2.6.1 installed ==="
```

---

## PHASE 2: Install GNU Arm Embedded Toolchain

```bash
cd ~/ncs

# Download ARM GCC toolchain (12.3 Rel1 - compatible with NCS 2.6.x)
wget https://developer.arm.com/-/media/Files/downloads/gnu/12.3.rel1/binrel/arm-gnu-toolchain-12.3.rel1-x86_64-arm-none-eabi.tar.xz

# Extract
tar xf arm-gnu-toolchain-12.3.rel1-x86_64-arm-none-eabi.tar.xz

# Add to PATH
echo 'export ZEPHYR_TOOLCHAIN_VARIANT=gnuarmemb' >> ~/.bashrc
echo "export GNUARMEMB_TOOLCHAIN_PATH=$HOME/ncs/arm-gnu-toolchain-12.3.rel1-x86_64-arm-none-eabi" >> ~/.bashrc
source ~/.bashrc

# Verify
arm-none-eabi-gcc --version
```

---

## PHASE 3: USB Passthrough for Flashing (CRITICAL for Crostini)

ChromeOS Crostini does not pass USB through by default.
You need to grant the Linux container USB device access.

```
ChromeOS Settings → Advanced → Developers → Linux development environment
→ Manage USB devices
→ Enable your nRF9160 board (shows as "J-Link" or "nRF91xx DK")
```

After enabling, verify inside Crostini:
```bash
lsusb
# Should show: SEGGER J-Link or Nordic Semiconductor

# Install J-Link tools (download .deb from Segger)
# https://www.segger.com/downloads/jlink/
# Choose: "J-Link Software and Documentation Pack" → Linux 64-bit .deb

# After downloading to ~/Downloads (accessible at /mnt/chromeos/MyFiles/Downloads/)
sudo dpkg -i /mnt/chromeos/MyFiles/Downloads/JLink_Linux_V794_x86_64.deb

# Verify
JLinkExe -version
```

---

## PHASE 4: Clone and Build the Firmware

```bash
# Copy your project into the NCS workspace apps directory
# (or develop directly under ~/ncs/nrf/applications/)
cd ~/ncs
cp -r /path/to/presence_relay ./presence_relay

# Set ZEPHYR_BASE (west zephyr-export should have done this,
# but set explicitly if build fails with "Zephyr not found")
export ZEPHYR_BASE=~/ncs/zephyr

# --- BUILD FOR ACTINIUS ICARUS ---
# The _ns suffix = Non-Secure build (NSPE; requires SPM flashed separately)
west build -b actinius_icarus_ns presence_relay \
    --build-dir build_icarus

# --- OR BUILD FOR NORDIC nRF9160-DK ---
west build -b nrf9160dk_nrf9160_ns presence_relay \
    --build-dir build_dk

# --- OR BUILD FOR NORDIC THINGY:91 ---
west build -b thingy91_nrf9160_ns presence_relay \
    --build-dir build_thingy91

# Successful build output will show:
# [100%] Linking C executable zephyr/zephyr.elf
# Memory region        Used Size  Region Size  %age Used
#      FLASH:       xxxxx B       1 MB         xx%
#        RAM:       xxxxx B     256 KB         xx%
```

---

## PHASE 5: Build and Flash SPM (Secure Partition Manager)

The SPM is the TrustZone secure image that MUST be flashed to the
Secure Processing Environment before your application will run.
Nordic provides it as a pre-built sample.

```bash
# Build SPM for your board (example: Actinius Icarus)
cd ~/ncs
west build -b actinius_icarus \
    nrf/samples/spm \
    --build-dir build_spm_icarus

# This generates build_spm_icarus/zephyr/zephyr.hex
# which is the secure partition image
```

**TF-M Alternative (production-grade):**
For formal security certification, replace SPM with TF-M:
```bash
# Add to prj.conf and rebuild:
# CONFIG_BUILD_WITH_TFM=y
# CONFIG_TFM_PROFILE_TYPE_MINIMAL=y
# West will automatically build TF-M alongside your app
```

---

## PHASE 6: Flash to Hardware

### Option A: Flash Everything via west (recommended)
```bash
# Flash SPM first (secure partition)
west flash --build-dir build_spm_icarus

# Flash application (non-secure partition)
west flash --build-dir build_icarus
```

### Option B: Flash merged hex manually with nrfjprog
```bash
# Install nRF Command Line Tools
# Download from: https://www.nordicsemi.com/Products/Development-tools/nrf-command-line-tools
# Install .deb: sudo dpkg -i nrf-command-line-tools_*.deb

# Merge SPM + app hex files
mergehex \
    --merge build_spm_icarus/zephyr/zephyr.hex \
           build_icarus/zephyr/zephyr.hex \
    --output merged_firmware.hex

# Erase and program
nrfjprog --family NRF91 --eraseall
nrfjprog --family NRF91 --program merged_firmware.hex --verify
nrfjprog --family NRF91 --reset
```

### Option C: DFU via USB (if board supports it, e.g., Thingy:91)
```bash
# Install mcumgr (Zephyr DFU client)
pip3 install --user mcumgr-client

# Or use nRF Connect for Desktop app on a ChromeOS-accessible path
```

---

## PHASE 7: Serial Monitor (Debug Output)

```bash
# Find your device
ls /dev/ttyACM* /dev/ttyUSB*

# Open serial terminal at 115200 baud
# Install minicom if not present
sudo apt install -y minicom

minicom -D /dev/ttyACM0 -b 115200

# Or use screen:
screen /dev/ttyACM0 115200

# Expected boot output:
# *** Booting Zephyr OS build v3.5.x ***
# [00:00:00.100] <inf> presence_relay: =========================================
# [00:00:00.101] <inf> presence_relay:  Zero-Knowledge Presence Relay v1.0
# [00:00:00.102] <inf> presence_relay:  Civic OS Project / Santa Cruz County
# [00:00:00.103] <inf> presence_relay:  Privacy mode: MAXIMUM (no PII transmitted)
# [00:00:00.104] <inf> presence_relay: =========================================
# [00:00:00.200] <inf> presence_relay: Button interrupt configured on pin 6
# [00:00:00.201] <inf> presence_relay: CA certificate loaded (tag: 42)
# [00:00:00.202] <inf> presence_relay: System ready. Press button to send presence signal.
```

---

## PHASE 8: Provision eSIM / SIM

For Actinius Icarus with onboard eSIM:
- Use Actinius DevKit companion app or the Actinius console
- Provision with a supported LTE-M/NB-IoT carrier (Twilio, 1NCE, Hologram)
- 1NCE offers 10-year IoT SIM plans at ~$10 lifetime for 500MB/SMS

For external nano-SIM (nRF9160-DK):
- Any LTE-M/NB-IoT capable SIM works (T-Mobile, AT&T in US)
- T-Mobile has best LTE-M coverage in Santa Cruz County

---

## PHASE 9: Server-Side Configuration

Your sovereign backend needs to:

1. Accept `POST /api/v1/presence` with `Content-Type: application/json`
2. Validate the bearer token against your token registry
3. Log receipt timestamp and optionally notify assigned social worker
4. Return HTTP 200/204 on success

**Minimal Python/FastAPI backend skeleton:**
```python
from fastapi import FastAPI, HTTPException, Header
from pydantic import BaseModel
import datetime

app = FastAPI()

# In production: load from env or secrets manager
VALID_TOKENS = {"REPLACE_WITH_DEPLOYMENT_TOKEN"}

class PresencePayload(BaseModel):
    status: str
    token: str

@app.post("/api/v1/presence")
async def receive_presence(
    payload: PresencePayload,
    authorization: str = Header(None)
):
    token = payload.token
    if token not in VALID_TOKENS:
        raise HTTPException(status_code=401, detail="Invalid token")
    
    # Log without PII - only timestamp and token hash
    print(f"Presence verified: {datetime.datetime.utcnow().isoformat()}")
    
    # Trigger social worker notification here
    # (email, SMS, internal queue - your choice)
    
    return {"acknowledged": True}
```

---

## Troubleshooting (Crostini-Specific)

| Issue | Fix |
|---|---|
| `west` not found after install | `source ~/.bashrc` or open new terminal tab |
| USB device not visible in `/dev/` | Re-enable in ChromeOS Settings → Linux USB |
| `cmake` version too old | `sudo apt install -y cmake` pulls 3.18; use pip: `pip3 install cmake` |
| Build fails "Zephyr not found" | `export ZEPHYR_BASE=~/ncs/zephyr` then retry |
| `nrfjprog` "No device found" | Check USB passthrough; try unplugging/replugging after enabling |
| LTE not connecting | Verify SIM is provisioned; check `AT+CEREG?` via serial |
| TLS handshake fails | CA cert PEM may have wrong linebreaks; use `unix2dos` to check |

---

## Privacy Audit Checklist

Before deployment, verify:
- [ ] `PRESENCE_TOKEN` replaced with actual deployment token
- [ ] `SERVER_HOST` points to your sovereign backend (not a cloud SaaS)
- [ ] `server_ca_cert[]` contains your actual server CA cert PEM
- [ ] Serial logging disabled or reduced for field deployment (`CONFIG_LOG_DEFAULT_LEVEL=0`)
- [ ] GPS/GNSS never initialized (confirm `CONFIG_NRF_CLOUD_AGPS=n` etc.)
- [ ] No cloud telemetry SDKs linked (nRF Cloud, AWS IoT - confirm not in west.yml)
- [ ] Token rotation procedure documented for social services team
