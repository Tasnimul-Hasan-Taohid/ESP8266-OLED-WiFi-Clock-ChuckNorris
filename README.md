# ESP8266 OLED WiFi Clock & Chuck Norris Joke Display

A complete Arduino/ESP8266 project featuring a WiFi provisioning captive portal, a live NTP clock on a 0.96" OLED, and Chuck Norris jokes fetched every 5 minutes from the internet.

---

## Features

| Feature | Details |
|---|---|
| 📡 WiFi Provisioning | AP mode with mobile-friendly captive portal |
| 🕐 NTP Clock | Real-time HH:MM:SS + date, configurable timezone |
| 😂 Chuck Norris Jokes | Fetched every 5 min from chucknorris.io API |
| 💾 EEPROM Persistence | WiFi credentials survive power cycles |
| 🔧 Factory Reset | Hold FLASH button 3 seconds to wipe credentials |
| 🔄 Error Recovery | Auto-fallback to AP mode on WiFi loss |
| 🚫 Non-blocking | State machine — no blocking `delay()` in main loop |

---

## Hardware

### Components

- **Board:** ESP8266 (NodeMCU or Wemos D1 Mini)
- **Display:** 0.96" OLED, SSD1306 driver, 128×64 px, I2C
- **Power:** Micro-USB 5V / 2A recommended

### Wiring

| ESP8266 Pin | OLED Pin | Description |
|---|---|---|
| D1 (GPIO5) | SCL | I2C Clock |
| D2 (GPIO4) | SDA | I2C Data |
| 3V3 | VCC | Power |
| GND | GND | Ground |

---

## Project Structure

```
ESP8266-OLED-WiFi-Clock-ChuckNorris/
├── main.ino               ← Setup, loop, state machine
├── config.h               ← All constants and pin defines
├── display_manager.h
├── display_manager.cpp    ← All OLED drawing functions
├── wifi_manager.h
├── wifi_manager.cpp       ← AP mode, captive portal, EEPROM
├── ntp_manager.h
├── ntp_manager.cpp        ← NTP time sync and formatting
├── joke_manager.h
├── joke_manager.cpp       ← Chuck Norris API fetch + timing
└── README.md
```

---

## Library Installation

Open Arduino IDE → Sketch → Include Library → Manage Libraries, then install:

| Library | Author | Notes |
|---|---|---|
| Adafruit GFX | Adafruit | Graphics primitives |
| Adafruit SSD1306 | Adafruit | OLED driver |
| ArduinoJson | Benoit Blanchon | Use **v6 or later** |
| NTPClient | Fabrice Weinberg | Time sync |

**Built-in (no install needed):**
- ESP8266WiFi
- ESP8266WebServer
- DNSServer
- ESP8266HTTPClient
- EEPROM

---

## First-Time Setup

### 1. Flash the Firmware

1. Open `main.ino` in Arduino IDE
2. **Tools → Board** → `NodeMCU 1.0 (ESP-12E Module)` or `LOLIN(Wemos) D1 Mini`
3. **Tools → Port** → select your COM port
4. Click **Upload**

### 2. Initial Boot — AP Mode

The device starts a WiFi hotspot. Your OLED shows:

```
WiFi Setup
SSID: OLED-Setup
Pass: 12345678
IP: 192.168.4.1
```

### 3. Connect from Your Phone

- Open WiFi settings
- Connect to **OLED-Setup** (password: `12345678`)
- A captive portal opens automatically
- If not, navigate to `http://192.168.4.1` in your browser

### 4. Enter Your WiFi Credentials

- Select your home network from the dropdown
- Enter your WiFi password
- Tap **Connect**

### 5. Normal Operation

Once connected, the OLED switches to clock mode automatically.

---

## OLED Display Layouts

### AP Mode
```
┌──────────────────────────┐
│ WiFi Setup               │
│ SSID: OLED-Setup         │
│ Pass: 12345678           │
│ IP: 192.168.4.1          │
│ Open browser to          │
│ 192.168.4.1              │
└──────────────────────────┘
```

### Connecting (animated)
```
┌──────────────────────────┐
│                          │
│       Connecting         │
│         . . .            │
│                          │
└──────────────────────────┘
```

### Clock Mode (normal operation)
```
┌──────────────────────────┐
│  14:32:45  ← large font  │
│  Sat 28 Mar 2026         │
│  WiFi: -45 dBm           │
│  Joke in 182s            │
└──────────────────────────┘
```

### Joke Display (30 seconds, auto-scrolls)
```
┌──────────────────────────┐
│ Chuck Norris:            │
│                          │
│ Chuck doesn't need to    │
│ Google. The answers      │
│ come to him.         (1/2│
└──────────────────────────┘
```

---

## Configuration

Edit `config.h` to customise:

```cpp
// Timezone — change this for your location
#define UTC_OFFSET_SEC    21600    // UTC+6 (Bangladesh)

// Common timezones:
// UTC+0  London     →       0
// UTC+1  Berlin     →    3600
// UTC+5:30 India    →   19800
// UTC+6  Bangladesh →   21600
// UTC-5  EST        →  -18000
// UTC-8  PST        →  -28800

// Joke fetch interval (default 5 min)
#define JOKE_INTERVAL     300000

// How long to show a joke (default 30s)
#define JOKE_DISPLAY_MS   30000

// WiFi connect timeout (default 20s)
#define CONNECT_TIMEOUT   20000

// Factory reset hold time (default 3s)
#define RESET_HOLD_MS     3000

// Toggle serial debug output
#define ENABLE_DEBUG      true
```

---

## State Machine

```
         ┌───────────┐
    ┌───▶│  AP_MODE  │◀────────────┐
    │    └─────┬─────┘             │
    │          │ credentials       │
    │          ▼                   │
    │    ┌───────────┐             │
    │    │CONNECTING │             │
    │    └─────┬─────┘             │
    │   timeout│    │ success      │
    │          │    ▼              │
    │          │ ┌──────────┐      │
    │          │ │CONNECTED │──────┤
    │          │ └──────────┘ lost │
    │          ▼                   │
    │    ┌───────────┐             │
    └────│   ERROR   │─────────────┘
         └───────────┘
```

---

## Web Endpoints (AP Mode Only)

| Endpoint | Method | Description |
|---|---|---|
| `/` | GET | Setup page with SSID picker |
| `/connect` | POST | Submit WiFi credentials |
| `/status` | GET | Connection status (auto-refresh) |
| `/reset` | POST | Factory reset (wipes EEPROM) |
| `/scan` | GET | List nearby networks (JSON) |

---

## Troubleshooting

**Display not working**
- Check SDA → D2 (GPIO4) and SCL → D1 (GPIO5)
- Verify I2C address is `0x3C` in `config.h`
- Check 3.3V power to OLED (do **not** use 5V)

**Can't connect to OLED-Setup network**
- Try forgetting and reconnecting from phone settings
- Check the device has power and is booting (check serial at 115200 baud)

**Can't open captive portal**
- Manually navigate to `http://192.168.4.1`
- Disable mobile data on your phone (Android sometimes ignores captive portals)

**WiFi won't connect**
- SSID and password are case-sensitive
- Hold the FLASH button for 3 seconds to factory reset, then try again
- Check serial output for `[CONNECTING]` debug messages

**Jokes not appearing**
- Confirm internet works on your WiFi
- Check serial output for `[JOKE]` HTTP response codes
- If API is down, device shows `"Chuck is busy saving the world right now..."`

**Device keeps rebooting**
- Use a proper 2A USB adapter (not a PC USB port)
- Check for short circuits on the I2C wires

---

## API Reference

**Chuck Norris Jokes**
```
GET https://api.chucknorris.io/jokes/random

Response:
{
  "value": "Chuck Norris can access private members of a class.",
  ...
}
```

**NTP**
```
Server:   pool.ntp.org
Protocol: UDP port 123
```

---

## License

MIT License — free to use, modify, and distribute.

---

## Credits

- **ESP8266** — Espressif Systems
- **OLED library** — Adafruit SSD1306
- **Jokes API** — [chucknorris.io](https://api.chucknorris.io)
- **Time** — [NTP Pool Project](https://www.ntppool.org)
- **JSON** — ArduinoJson by Benoit Blanchon
