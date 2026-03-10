# Mobile App — Pet Feeder Companion (Separate Repository)

> **Repository:** Create as `petfeeder-mobile` (separate repo)

## Tech Stack (Production-Grade, In-Demand)

| Layer | Technology | Why |
|-------|-----------|-----|
| **Framework** | React Native + Expo (SDK 52+) | Cross-platform iOS/Android, massive ecosystem, most in-demand mobile framework |
| **Language** | TypeScript | Type safety, better DX, industry standard |
| **Navigation** | Expo Router (file-based) | Modern, Next.js-style routing for RN |
| **State Management** | Zustand | Lightweight, scalable, minimal boilerplate |
| **BLE Communication** | react-native-ble-plx | Best-maintained BLE library for React Native |
| **Cloud/API** | Azure IoT Hub SDK or REST | Match firmware's Azure backend |
| **Real-time** | Azure SignalR or WebSocket | Live dashboard updates |
| **Charts** | Victory Native or react-native-chart-kit | Historical data visualization |
| **UI Library** | Tamagui or NativeWind (Tailwind) | Production-level, performant styling |
| **Auth** | Clerk or Supabase Auth | Modern auth with social login |
| **Database** | Supabase (PostgreSQL) or Azure Cosmos DB | Historical data storage |
| **Push Notifications** | Expo Notifications | Feeding alerts, low-food warnings |

## Architecture

```
petfeeder-mobile/
├── app/                        # Expo Router screens
│   ├── (tabs)/
│   │   ├── index.tsx           # Dashboard (home)
│   │   ├── history.tsx         # Feeding history
│   │   ├── schedule.tsx        # Feeding schedule management
│   │   ├── settings.tsx        # Device & app settings
│   │   └── _layout.tsx         # Tab navigation layout
│   ├── device/
│   │   ├── connect.tsx         # BLE device scanning & pairing
│   │   ├── calibrate.tsx       # Load cell calibration wizard
│   │   └── [id].tsx            # Individual device detail
│   ├── auth/
│   │   ├── login.tsx
│   │   └── register.tsx
│   └── _layout.tsx             # Root layout
├── components/
│   ├── ui/                     # Reusable UI components
│   │   ├── Card.tsx
│   │   ├── Button.tsx
│   │   ├── Badge.tsx
│   │   └── Chart.tsx
│   ├── dashboard/
│   │   ├── FoodLevelGauge.tsx
│   │   ├── BowlWeightCard.tsx
│   │   ├── FeedingStatusCard.tsx
│   │   ├── MotionIndicator.tsx
│   │   └── QuickFeedButton.tsx
│   └── device/
│       ├── BLEScanner.tsx
│       └── DeviceCard.tsx
├── hooks/
│   ├── useBLE.ts               # BLE connection management
│   ├── useDevice.ts            # Device state from BLE/cloud
│   ├── useFeedingHistory.ts    # Historical data queries
│   └── useRealTimeData.ts      # Live sensor data
├── stores/
│   ├── deviceStore.ts          # Zustand store for device state
│   ├── authStore.ts
│   └── settingsStore.ts
├── services/
│   ├── ble/
│   │   ├── bleService.ts       # BLE connection & communication
│   │   ├── bleProtocol.ts      # Command/status JSON protocol
│   │   └── bleConstants.ts     # UUIDs matching firmware
│   ├── api/
│   │   ├── apiClient.ts        # REST API client
│   │   ├── feedingApi.ts
│   │   └── deviceApi.ts
│   └── notifications/
│       └── pushService.ts
├── utils/
│   ├── formatters.ts
│   └── constants.ts
├── types/
│   ├── device.ts
│   ├── feeding.ts
│   └── sensor.ts
├── assets/
├── app.json
├── package.json
└── tsconfig.json
```

## Key Screens

### 1. Dashboard
- **Real-time food level** — gauge/progress bar (FULL/HALF/EMPTY)
- **Bowl weight** — live grams display
- **Next feeding** — countdown timer
- **Pet detection** — motion indicator
- **Quick Feed** — one-tap manual dispense
- **Connection status** — BLE/WiFi/MQTT indicators
- **Daily summary** — grams dispensed today

### 2. Feeding History
- **Timeline view** — all feeding events (auto + manual)
- **Daily/weekly/monthly charts** — consumption trends
- **Export** — CSV download for vet records
- **Filters** — by date range, feeding type

### 3. Schedule Management
- **Edit feeding times** — add/remove/modify schedule
- **Portion size** — adjust grams per feeding
- **Daily limit** — set maximum daily grams
- **Toggle auto-feeding** — enable/disable

### 4. Device Settings
- **BLE pairing** — scan and connect to feeder
- **WiFi configuration** — update WiFi credentials via BLE
- **Calibration** — guided load cell calibration
- **Firmware info** — version, uptime, diagnostics
- **Notifications** — configure alerts (low food, missed feeding, etc.)

## BLE Protocol

The app communicates with ESP32 via BLE using JSON:

**Send commands:**
```json
{"cmd": "feed"}
{"cmd": "refill"}
{"cmd": "tare"}
{"cmd": "schedule", "times": [480, 720, 1080, 1320]}
{"cmd": "wifi", "ssid": "...", "password": "..."}
```

**Receive status (every 2s via BLE notify):**
```json
{
  "food": "FULL",
  "bowl": "PARTIAL",
  "weight": 85.5,
  "distance": 5.2,
  "dispensing": false,
  "refill": false,
  "dailyFood": 75.0,
  "motion": true,
  "status": "Ready to feed",
  "wifi": true,
  "mqtt": true,
  "time": "14:30:00",
  "nextFeed": "18:00:00"
}
```

## Getting Started

```bash
# Create the repo
npx create-expo-app petfeeder-mobile --template tabs
cd petfeeder-mobile

# Install core dependencies
npx expo install react-native-ble-plx
npm install zustand
npm install @tamagui/core tamagui
npm install victory-native

# Development
npx expo start

# Build
eas build --platform all
```

## UI/UX Guidelines

- **Design System:** Use consistent spacing (4px grid), rounded corners, soft shadows
- **Colors:** Green (full), Blue (half), Red (empty), Purple (dispensing), Yellow (refill)
- **Dark Mode:** Support system theme preference
- **Haptics:** Tap feedback on buttons
- **Animations:** Smooth transitions for gauge values, card reveals
- **Accessibility:** Screen reader labels, minimum touch targets (44×44pt)
- **Offline Mode:** Cache last-known device state when BLE disconnects
