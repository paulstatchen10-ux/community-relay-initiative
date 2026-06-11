# community-relay-initiative
The Community Relay &amp; Enrollment (CRE) Initiative is a decentralized, open-source policy and hardware framework designed to prevent procedural Medi-Cal disenrollment among unhoused populations. It utilizes a trusted community network and a custom, one-button eSIM device to securely verify presence and link individuals to remote county caseworkers.
# The Community Relay & Enrollment (CRE) Initiative 

## What is this?
The Community Relay Initiative is a decentralized, "Fourth Office" policy proposal and hardware framework designed to bridge the last-mile gap in public health enrollment. It utilizes community-led outreach and custom, privacy-preserving IoT devices to help vulnerable and unhoused populations maintain their Medi-Cal (Medicaid) coverage.

## The Problem: The Mail-Based Disenrollment Crisis
The current public health renewal infrastructure relies heavily on physical mail and standardized digital portals. For individuals without a fixed address, a smartphone, or consistent internet access, this bureaucracy is a barrier to care. 
* **The "Property Rights" Limitation**: Government caseworkers cannot easily perform field enrollments due to the legal, liability, and safety risks of operating on private or contested property. Bureaucracy requires individuals to come to a government building, which the most vulnerable cannot do.
* **The Result**: Procedural disenrollment. People lose life-saving care not because they are no longer eligible, but because they cannot be reached.

## The Solution: A Zero-Knowledge "Fourth Office" Relay
This initiative proposes a localized, community-based "relay" model. We deploy an intermediary—a trusted community support group—to locate the individual and deliver a temporary, secure hardware bridge (the device) to facilitate an immediate, in-person renewal with a remote government caseworker.

### 1. The Hardware: "Presence over Surveillance"
The project advocates for the use of a custom, credit-card-sized device designed strictly for enrollment relay:
* **Tamper-Proof eSIM**: Embedded IoT SIM technology (utilizing LTE-M/NB-IoT modules like the Nordic nRF9160 SiP) that cannot be removed, repurposed, or hacked.
* **Single-Function Interface**: A one-button design. The device does not browse the web or make standard phone calls. It establishes an encrypted, direct line to a designated county caseworker.
* **HIPAA Compliance**: The device facilitates a legally protected conversation. Under HIPAA and California's CMIA, Medi-Cal enrollment data is Protected Health Information (PHI). The device ensures this transaction remains secure and isolated.

### 2. The Architecture: The Zero-Knowledge Buffer
By separating the "search" from the "service," we protect all parties:
* **The Relay Team (Community)**: Handles the physical navigation of property and locating the individual. They act as the human interface.
* **The Caseworker (Government)**: Remains in a secure, compliant government facility. 
* **The Audit Trail**: The system verifies the *presence* of the individual and the *completion* of the renewal, without transmitting granular geolocation data to the government, preserving the individual's privacy while satisfying bureaucratic audit requirements.

## Warnings and Unintended Consequences
To ensure ethical deployment, this framework actively works against common technological pitfalls in social services:
* **The "Digital Poorhouse" Effect**: Technology in social services often pivots from access to surveillance. This device is physically and logically isolated to ensure it cannot be used for law enforcement tracking. It is a tool for *access*, not a monitor.
* **Emergency Exceptions (Title 22)**: While the device protects standard geolocation data, any healthcare-adjacent deployment in California must balance privacy with Title 22 emergency preparedness. The system is designed to allow life-saving intervention if explicitly triggered, without defaulting to continuous public surveillance.
* **System "Heat" (Anomaly Detection)**: To prevent fraud, the network employs threshold monitoring. If an anomalous number of enrollments originates from a single relay node, the system flags it for community audit rather than automatic rejection.

## How to Contribute
This is an open-source policy and technical framework. We are seeking collaboration on:
* **Hardware Specs**: Identifying the most cost-effective, bulk-order IoT eSIM boards for the prototype.
* **Policy Integration**: Mapping this workflow into the **CalAIM Community Supports** framework for local county opt-in.
* **Funding Models**: Leveraging Medi-Cal Capacity Grants to fund the non-profit Technical Architect managing device distribution.

## License
This project is licensed under the MIT License.
