# Web Flasher & Calibration Tool (Separate Repository)

> **Repository:** Create as `petfeeder-web` (separate repo)

## Overview

A web application that lets users:
1. **Select a project/firmware** from available embedded firmware variants
2. **Flash firmware** directly to their ESP32 via USB (Web Serial API)
3. **Calibrate sensors** (load cell, ultrasonic) through a guided wizard
4. **Configure WiFi/cloud credentials** before flashing

No desktop software required — works entirely in the browser via USB-C/USB connection.

## Tech Stack

| Layer | Technology | Why |
|-------|-----------|-----|
| **Framework** | Next.js 15 (App Router) | SEO, SSR, best React framework |
| **Language** | TypeScript | Type safety |
| **Styling** | Tailwind CSS + shadcn/ui | Production-level components |
| **Firmware Flashing** | Web Serial API + esptool-js | Flash ESP32 from browser |
| **State** | Zustand | Simple, scalable |
| **Database** | Supabase or Vercel Postgres | Store firmware binaries, user configs |
| **Auth** | Clerk or NextAuth.js | User accounts |
| **Hosting** | Vercel | Zero-config deployment |

## Architecture

```
petfeeder-web/
├── app/
│   ├── page.tsx                  # Landing page
│   ├── projects/
│   │   ├── page.tsx              # Browse available firmware projects
│   │   └── [slug]/
│   │       ├── page.tsx          # Project detail + flash button
│   │       ├── flash/
│   │       │   └── page.tsx      # Flashing wizard
│   │       └── calibrate/
│   │           └── page.tsx      # Calibration wizard
│   ├── dashboard/
│   │   └── page.tsx              # User's devices & history
│   └── layout.tsx
├── components/
│   ├── ui/                       # shadcn components
│   ├── flash/
│   │   ├── SerialConnect.tsx     # USB connection prompt
│   │   ├── FlashProgress.tsx     # Progress bar during flash
│   │   ├── FirmwareSelector.tsx  # Choose firmware variant
│   │   └── CredentialForm.tsx    # WiFi/API config before flash
│   ├── calibrate/
│   │   ├── LoadCellWizard.tsx    # Step-by-step calibration
│   │   ├── UltrasonicWizard.tsx
│   │   └── SerialMonitor.tsx     # Live serial output
│   └── projects/
│       ├── ProjectCard.tsx
│       └── ProjectGrid.tsx
├── lib/
│   ├── serial/
│   │   ├── serialService.ts      # Web Serial API wrapper
│   │   ├── espFlasher.ts         # esptool-js integration
│   │   └── serialMonitor.ts      # Read serial output
│   ├── firmware/
│   │   ├── firmwareBuilder.ts    # Inject credentials into firmware
│   │   └── firmwareStore.ts      # Fetch firmware binaries
│   └── supabase/
│       └── client.ts
├── public/
│   └── firmware/                  # Pre-compiled firmware binaries
│       ├── petfeeder-v2.0.0.bin
│       └── manifest.json
├── package.json
├── next.config.ts
└── tailwind.config.ts
```

## User Flow

### Flash Firmware
```
1. User visits /projects and selects "Pet Feeder"
2. Clicks "Flash to Device"
3. Enters WiFi SSID, password, Azure IoT credentials
4. Connects ESP32 via USB-C cable
5. Browser prompts for serial port access
6. Credentials are injected into firmware binary
7. Firmware is flashed to ESP32 (progress bar)
8. Device reboots and connects to WiFi
9. Success screen with next steps
```

### Calibrate Sensors
```
1. User visits /projects/pet-feeder/calibrate
2. Connects ESP32 via USB
3. Guided wizard:
   a. "Remove all weight from scale" → Tare
   b. "Place 100g weight" → Read
   c. Shows calibration factor
   d. "Verify: place known weight" → Confirm
4. Factor is saved to device EEPROM
```

## Web Serial API Requirements

- **Browser:** Chrome 89+, Edge 89+, Opera 76+ (no Firefox/Safari)
- **Connection:** USB-C or USB-Micro to ESP32
- **Baud Rate:** 115200
- **Protocol:** The web app sends serial commands and reads responses

### Serial Commands (Calibration Mode)
```
> TARE
< OK:TARED
> CALIBRATE:100.0
< OK:FACTOR:49400
> VERIFY
< OK:WEIGHT:99.8
> SAVE
< OK:SAVED
```

## Key Features

### Firmware Project Browser
- Grid of available firmware projects (Pet Feeder, future projects)
- Version history
- Hardware requirements list
- Difficulty rating

### Flash Wizard
- Step-by-step with clear instructions
- Auto-detect ESP32 board type
- Credential injection (compile-time or runtime EEPROM write)
- Progress bar with stage labels (Connecting → Erasing → Writing → Verifying)
- Error recovery (retry, reset instructions)

### Calibration Tool
- Live serial monitor output
- Interactive guided steps with illustrations
- Save calibration to device
- Export calibration report

### Dashboard (Logged-in Users)
- List of flashed devices
- Firmware update notifications
- Configuration backup/restore

## Getting Started

```bash
# Create the repo
npx create-next-app@latest petfeeder-web --typescript --tailwind --app
cd petfeeder-web

# Install dependencies
npm install esptool-js
npx shadcn@latest init
npm install zustand

# Development
npm run dev

# Deploy
vercel deploy
```

## Browser Compatibility Note

Web Serial API is only available in Chromium-based browsers. Display a compatibility notice for unsupported browsers with a link to Chrome download.
