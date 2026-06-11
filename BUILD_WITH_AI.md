# BUILD_WITH_AI.md
## Community Relay & Enrollment (CRE) Initiative
### Your Complete Guide to Building This Project with AI Assistance

---

> **What this document is:**
> The CRE Initiative is a real, open-source civic infrastructure project.
> This device prevents unhoused people from losing their Medi-Cal health
> coverage simply because they can't receive mail or get to a government office.
>
> This guide gives you everything you need to build any part of this project
> yourself — from the physical hardware button to the server that receives
> the signal — using AI as your coding partner.
>
> You don't need to be a programmer. You need curiosity and a computer.

---

## How This Works: The Big Picture

```
┌─────────────────────────────────────────────────────────────────┐
│                    THE COMPLETE SYSTEM                          │
│                                                                 │
│  [UNHOUSED PERSON]                                              │
│       │                                                         │
│       ▼                                                         │
│  [PRESSES BUTTON]  ← Credit-card sized device, no screen       │
│       │               Nordic nRF9160 chip, eSIM built in        │
│       │                                                         │
│       ▼                                                         │
│  [LTE-M SIGNAL]    ← Travels over cellular network             │
│       │               No GPS. No location. Token only.          │
│       │                                                         │
│       ▼                                                         │
│  [YOUR SERVER]     ← Runs on a Raspberry Pi, laptop, or cloud  │
│       │               Receives: {"status":"presence_verified"}  │
│       │                                                         │
│       ▼                                                         │
│  [SOCIAL WORKER]   ← Gets notified. Makes contact.             │
│                       Medi-Cal enrollment happens remotely.     │
│                                                                 │
│  PRIVACY GUARANTEE: The government sees "someone needs help"    │
│  NOT "this specific person is at this GPS coordinate"           │
└─────────────────────────────────────────────────────────────────┘
```

---

## Choose Your Path

Find your situation below and jump to that section.

```
Are you a student doing this for the first time?
│
├─► YES → Go to: SCENARIO A — First-Time Student Builder
│
└─► NO
    │
    ├─► Are you a teacher/mentor setting this up for a class?
    │   └─► YES → Go to: SCENARIO B — Classroom Deployment
    │
    ├─► Are you a developer who wants to run the full system?
    │   └─► YES → Go to: SCENARIO C — Full Stack Self-Build
    │
    └─► Are you a community org or county looking to deploy this?
        └─► YES → Go to: SCENARIO D — Civic/Institutional Deployment
```

---

---

# SCENARIO A — First-Time Student Builder

**Who this is for:** High school or college student. You have a computer.
You may have never coded before. You want to build something real.

**What you'll build:** The computer-side software that receives signals
from the device and displays them on your screen.

**Time:** 1–3 hours depending on your setup.

---

## Step 1: Understand What You're Building

You are building the **receiver** — the software running on your laptop
that listens for a signal from the button device and shows you when
someone has pressed it.

You are NOT building the device itself yet (that comes later, or your
teacher may already have one ready).

---

## Step 2: Open Claude and Paste This Prompt

Go to **claude.ai** — create a free account if you don't have one.
Start a new conversation. Copy everything in the box below and paste it in.

---

### ✂️ STUDENT PROMPT — Copy and paste this entire block

```
Hi Claude. I'm a student working on a real civic tech project called the
Community Relay & Enrollment (CRE) Initiative. Here's what it does:

Unhoused people often lose their Medi-Cal health coverage because they
can't receive mail or get to a government office. This project gives
them a physical button — a small IoT device — they can press to signal
a social worker: "I'm here, I need help with my enrollment."

The device (built on a Nordic nRF9160 chip with an eSIM) sends a single
encrypted signal over LTE. No GPS. No name. No address. Just a token
that means "presence verified."

I need YOUR help to build the computer side — the software on my laptop
that receives this signal and shows me when someone has pressed the button.

Before you write any code, please ask me these questions one at a time:

1. What operating system am I using? (Windows, Mac, Chromebook, or Linux)
2. Do I have Python installed, and if so, what version?
3. Am I comfortable with the terminal/command line? (not at all / a little / yes)
4. How will my device communicate with my laptop?
   - USB cable (serial connection)
   - WiFi or local network
   - I'm not sure yet / I don't have the device yet

Based on my answers, please:
- Design the simplest version that will actually work on MY setup
- Walk me through installing anything I need step by step
- Write code I can actually read and understand, with comments explaining each part
- Show me how to test it even if I don't have the physical device yet
  (we can simulate a button press)

Privacy rules that cannot be changed:
- The software must NEVER request or store GPS location
- The software must NEVER store personal info about who pressed the button
- The only data recorded is: timestamp + "presence_verified"
- All logs must auto-delete after 30 days

I want to understand what I'm building, not just copy-paste code.
Ask me your first question.
```

---

## Step 3: Work Through It With Claude

Claude will ask you questions, then guide you through the build.
It will adjust to your skill level automatically.

**Tips:**
- If Claude uses a word you don't know, just ask: *"What does [word] mean?"*
- If something doesn't work, paste the exact error message into Claude
- Ask Claude to explain any line of code you don't understand
- Save your files as you go — ask Claude to help you set up a folder structure

---

## Step 4: Test Without the Device

You don't need the physical hardware to test your software.
Ask Claude: *"How do I simulate a button press to test this?"*
Claude will show you how to send a fake signal to your own software.

---

## Step 5: Share What You Built

Once it works, ask Claude:
*"Help me write a README explaining what this does and how I built it,
so I can post it on GitHub."*

Then upload your files to GitHub and link it from the main CRE repo.
Your version of this software helps everyone who builds it after you.

---

---

# SCENARIO B — Classroom Deployment

**Who this is for:** Teachers, mentors, after-school program leads.
You want a group of students to build this together as a project.

**Suggested structure:** 3–5 sessions of 60–90 minutes each.

---

## Session Flow

```
SESSION 1: The Problem and the Concept (no coding)
│
│  Activities:
│  - Read the README at github.com/paulstatchen10-ux/community-relay-initiative
│  - Discussion: What is procedural disenrollment? Why does it happen?
│  - Discussion: What is "zero-knowledge" design? Why does it matter?
│  - Each student writes: "What would I build if I were in charge of this?"
│
▼
SESSION 2: Set Up Your Environment
│
│  Each student (or pair) uses SCENARIO A → Step 2 prompt with Claude
│  Goal: Get the development environment working on each machine
│  Teacher circulates; Claude handles the technical questions
│
▼
SESSION 3: Build the Receiver Software
│
│  Continue the Claude session from Session 2
│  Goal: Have working code that can receive and display a signal
│  Test using the simulation method (no hardware needed)
│
▼
SESSION 4: Introduce the Hardware (Optional)
│
│  If you have an nRF9160 development board available:
│  - Demo the physical button
│  - Connect one student's software to the real device
│  - Group watches the signal travel from button press to screen
│
▼
SESSION 5: Present and Reflect
│
│  Each student/team presents:
│  - What they built
│  - One thing they'd add or improve
│  - How this connects to the policy problem it's solving
│
│  Optional: Submit a GitHub PR to the main CRE repo with their version
```

---

## Classroom Prompt (for teachers to share with students)

Same as Scenario A. The student prompt above is designed for any
skill level and will adapt based on what each student reports back.

---

## Hardware for Classrooms

If you want to demonstrate the physical device:
- **Most affordable dev board:** Nordic nRF9160-DK (~$100)
- **Most compact option:** Actinius Icarus (~$35 in quantity)
- **Beginner-friendly option:** Nordic Thingy:91 (~$100, no soldering)
- **SIM for testing:** 1NCE IoT SIM (~$10 for 10 years of data)

One device shared among a class of 30 students works fine for demonstration.
Students build the software side on their own machines.

---

---

# SCENARIO C — Full Stack Self-Build

**Who this is for:** Developers, civic technologists, systems architects.
You want to run the entire CRE system — hardware, firmware, backend server,
and alert system — on your own infrastructure.

**What you'll build:** The complete stack:
1. Firmware on the nRF9160 device (already in this repo)
2. Backend server to receive signals
3. Alert/notification system for social workers
4. Optional: Simple web dashboard

---

## Full Stack Architecture

```
┌──────────────────────────────────────────────────────────────────────┐
│                        COMPLETE STACK                                │
│                                                                      │
│  LAYER 1 — DEVICE FIRMWARE (this repo: main.c)                      │
│  ┌─────────────────────────────────────────────────────────────┐    │
│  │  nRF9160 chip → Zephyr RTOS → PSM sleep → button ISR →     │    │
│  │  LTE-M connect → HTTPS POST → token only → sleep again      │    │
│  └─────────────────────────────────────────────────────────────┘    │
│                              │ HTTPS POST                            │
│                              ▼                                       │
│  LAYER 2 — SOVEREIGN BACKEND (you build this)                       │
│  ┌─────────────────────────────────────────────────────────────┐    │
│  │  FastAPI (Python) or Express (Node.js)                       │    │
│  │  Receives POST → validates token → logs timestamp only       │    │
│  │  Runs on: Raspberry Pi / old laptop / VPS / county server    │    │
│  └─────────────────────────────────────────────────────────────┘    │
│                              │ triggers                              │
│                              ▼                                       │
│  LAYER 3 — ALERT SYSTEM (you build this)                            │
│  ┌─────────────────────────────────────────────────────────────┐    │
│  │  Sends notification to assigned social worker via:           │    │
│  │  - Email (SMTP) — simplest                                   │    │
│  │  - SMS (Twilio or similar)                                   │    │
│  │  - Slack/Teams webhook                                        │    │
│  │  - Custom dashboard (optional Layer 4)                       │    │
│  └─────────────────────────────────────────────────────────────┘    │
│                              │ optional                              │
│                              ▼                                       │
│  LAYER 4 — DASHBOARD (optional, you build this)                     │
│  ┌─────────────────────────────────────────────────────────────┐    │
│  │  Simple web page showing:                                    │    │
│  │  - Last signal received: [timestamp]                         │    │
│  │  - Status: pending / in-progress / resolved                  │    │
│  │  - No names, no locations — just case status                 │    │
│  └─────────────────────────────────────────────────────────────┘    │
└──────────────────────────────────────────────────────────────────────┘
```

---

## Full Stack Self-Build Prompt

Open a new Claude conversation and paste this entire block:

---

### ✂️ FULL STACK PROMPT — Copy and paste this entire block

```
Hi Claude. I'm building a complete deployment of the Community Relay &
Enrollment (CRE) Initiative — an open-source civic infrastructure project
that helps unhoused people maintain their Medi-Cal health coverage.

Here's the full technical context:

PROJECT REPO: https://github.com/paulstatchen10-ux/community-relay-initiative

WHAT ALREADY EXISTS IN THE REPO:
- main.c: Complete Zephyr RTOS firmware for Nordic nRF9160 SiP
  - PSM deep sleep, wakes on GPIO button interrupt
  - Sends HTTPS POST with ONLY a "presence_verified" token
  - No GPS, no IMEI, no PII transmitted
  - TrustZone SPE/NSPE split; modem isolated from app layer
  - TLS 1.2 with peer certificate verification
- prj.conf: Kconfig for LTE-M/NB-IoT, PSM, TLS
- CMakeLists.txt, west.yml, actinius_icarus_ns.overlay
- BUILD_GUIDE.md: Full Crostini/Chromebook build instructions

WHAT I NEED TO BUILD (tell me which parts you need to build):
[ ] Layer 2: Backend server (Python FastAPI preferred, or Node.js)
    - Accepts POST /api/v1/presence
    - Validates bearer token
    - Logs ONLY: timestamp + token_hash (never raw token or PII)
    - Auto-purges logs older than 30 days
    
[ ] Layer 3: Alert system
    - Notifies an assigned social worker when signal received
    - Method I want: [Email / SMS / Slack / Other]
    
[ ] Layer 4: Simple status dashboard
    - Web page showing signal history and case status
    - No names, no locations
    - Status per signal: received / worker_notified / enrolled / resolved

MY INFRASTRUCTURE:
- Server I'll run this on: [Raspberry Pi / old laptop / cloud VPS / other]
- Operating system: [Linux / Windows / Mac / ChromeOS]
- Do I have a domain name and SSL cert? [yes/no]
- Am I comfortable with Docker? [yes/no/learning]

HARD PRIVACY CONSTRAINTS — these cannot change:
1. Zero geolocation — never request, never store, never transmit coordinates
2. Zero PII — no names, no case numbers stored in this system
3. Token-only identification — the token identifies the DEVICE, not the person
4. Audit trail without surveillance — logs prove enrollment happened
   without revealing who, where, or when with precision
5. Data minimization — if it's not needed for enrollment, don't collect it
6. Sovereign hosting — this must run on hardware we control,
   not SaaS platforms with unknown data retention policies

DEPLOYMENT GOAL:
This will be presented to [Santa Cruz County / a local nonprofit / my school]
as a working prototype. It needs to be simple enough for a non-technical
social services team to operate day-to-day.

Before writing any code, ask me which layers I want to build first,
and confirm the details of my infrastructure. Then build one layer at a time,
testing each before moving to the next.
```

---

## After Your Build Session

Once you have a working system:

1. **Document your configuration** — ask Claude to generate a
   `DEPLOYMENT.md` file specific to your setup

2. **Rotate the token** — change `PRESENCE_TOKEN` in the firmware
   and update your server. Do this at least every 90 days.

3. **Test the full chain** — press the physical button, confirm your
   server receives it, confirm the social worker gets the notification

4. **Submit a PR** to the main repo with any improvements you made

---

---

# SCENARIO D — Civic / Institutional Deployment

**Who this is for:** County agencies, nonprofits, public health organizations,
CalAIM Community Supports coordinators.

---

## What You're Evaluating

This is a **prototype** policy and technical framework. It is not yet
a certified medical device or a HIPAA-compliant production system.

What it IS:
- A working proof of concept for "presence verification without surveillance"
- A technical implementation of a CalAIM-compatible community support workflow
- An Apache 2.0 open-source framework you can fork, modify, and own

What it NEEDS before institutional deployment:
- HIPAA/CMIA compliance audit (the architecture is designed for compliance;
  formal certification requires institutional review)
- County IT security review of the backend server configuration
- Social worker workflow integration (who gets the alert? what happens next?)
- Formal data retention and destruction policy aligned with county policy

---

## Institutional Deployment Prompt

For IT staff or technical consultants evaluating this for institutional use:

---

### ✂️ INSTITUTIONAL EVALUATION PROMPT — Copy and paste this entire block

```
Hi Claude. I'm evaluating the Community Relay & Enrollment (CRE) Initiative
for potential deployment by [our organization]. We are a [county agency /
nonprofit / public health org] in California.

Project repo: https://github.com/paulstatchen10-ux/community-relay-initiative

I need your help with a technical and compliance evaluation, then potentially
building a hardened version of the backend for institutional use.

EVALUATION TASKS — please address these in order:

1. HIPAA COMPLIANCE ANALYSIS
   Review the firmware (main.c) and the described backend architecture.
   Identify: What qualifies as PHI in this system? What does not?
   The signal itself (a token meaning "presence verified") — is it PHI?
   What safeguards are required for the backend server?

2. CALIFORNIA CMIA ANALYSIS
   The Confidentiality of Medical Information Act applies to Medi-Cal data.
   How does this system interact with CMIA requirements?
   What do caseworkers need to know before using this tool?

3. TITLE 22 EMERGENCY CARVE-OUT
   California Title 22 requires emergency preparedness in healthcare-adjacent
   systems. The current design has no emergency override.
   Recommend how to implement an emergency escalation path that:
   - Is opt-in only (never automatic)
   - Requires explicit double-confirmation from the individual
   - Does not compromise the default zero-knowledge architecture

4. THREAT MODEL REVIEW
   Who might want to misuse this system?
   - Law enforcement seeking location data (there is none to give)
   - Fraudulent enrollment (threshold anomaly detection is described in README)
   - Device theft/cloning (token rotation mitigates this)
   Identify any gaps in the current threat model.

5. INSTITUTIONAL BACKEND BUILD
   If I decide to proceed, help me build a hardened version of the backend
   server that meets institutional requirements:
   - Formal logging with tamper-evident audit trail
   - Role-based access control (caseworker vs supervisor vs auditor)
   - Automatic data retention enforcement (30-day purge)
   - Integration with our existing case management system (I'll describe it)

Our technical environment: [describe your servers, OS, existing systems]
Our compliance framework: [HIPAA / CMIA / county IT policy / other]

Let's start with the HIPAA compliance analysis.
```

---

---

# The Philosophy Behind This Design

This section is for anyone who wants to understand the *why*, not just the *how*.

## Neighbor vs. Citizen

This project distinguishes between two layers of human relationship:

**The Neighbor Layer** — universal, unconditional. Every person who presses
this button is a neighbor. The system owes them dignity, privacy, and care
regardless of documentation status, citizenship, or enrollment eligibility.

**The Citizen Layer** — jurisdictional, administrative. The county needs to
verify that an enrollment happened. The system satisfies this requirement
through the audit trail (timestamp + token) without collapsing the neighbor
into a data point.

The device is the physical expression of this distinction: it transmits
*presence* (I am here, I exist, I need help) without transmitting *location*
(here is exactly where I am for government records).

## "Presence Over Surveillance"

Every technology deployed in social services faces a fork in the road:
does this tool help the person it's supposed to serve, or does it
primarily generate data about them for institutions?

This design makes that choice structurally, not just as policy:
the firmware is *physically incapable* of transmitting GPS coordinates
because the GPS subsystem is never initialized. The TrustZone hardware
boundary ensures the application layer cannot access raw modem data.
Privacy is enforced by silicon, not by promises.

## The "Digital Poorhouse" Warning

The README flags this explicitly. Historically, technology introduced
to help poor populations often becomes infrastructure for surveilling them.
This project's architecture is designed to be surveillance-resistant
by default — not as a feature you can configure, but as the only mode
it operates in.

If a future version of this device ever transmits location data,
it is no longer this project. Fork it and call it something else.

---

---

# Quick Reference: All Prompts in One Place

| Scenario | Who | Prompt Section |
|---|---|---|
| First-time student | Any student, any computer | [STUDENT PROMPT](#scenario-a) |
| Classroom | Teacher/mentor | [STUDENT PROMPT](#scenario-a) — distribute to each student |
| Full stack dev | Developer/architect | [FULL STACK PROMPT](#scenario-c) |
| Institutional | County/nonprofit IT | [INSTITUTIONAL EVALUATION PROMPT](#scenario-d) |

---

# Contributing Your Build Back

After you've built your version of any part of this system:

1. Push your code to your own GitHub repo
2. Open a Pull Request to `paulstatchen10-ux/community-relay-initiative`
3. In your PR description, include:
   - What OS/hardware you built on
   - What you changed or improved
   - Any bugs you found in the base code
   - Your Claude conversation summary (what worked, what didn't)

Every contribution makes this easier for the next person.
A high schooler in another county, a caseworker who learned to code,
a nonprofit tech team with no budget — they're all building on what you build.

---

# Project Credits & License

**Lead Architect:** Paul Statchen, Civic OS Project, Seabright, Santa Cruz CA
**Hardware Firmware:** Nordic nRF9160 / Zephyr RTOS / nRF Connect SDK
**AI Build Partners:** Claude (Anthropic) · Gemini (Google) · ChatGPT (OpenAI)
**License:** Apache 2.0 — free to use, modify, redistribute, and deploy

Policy framework rooted in:
- U.S. Constitution · California Constitution
- CalAIM Community Supports Framework
- HIPAA / California CMIA
- Title 22 California Code of Regulations

---

*"The neighbor layer is universal. It applies to all beings regardless of
jurisdiction. Press the button. Someone will come."*

**GitHub:** https://github.com/paulstatchen10-ux/community-relay-initiative
