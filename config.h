#pragma once

// ─── OLED Display ───────────────────────────────────────────────
#define OLED_SDA          4        // D2
#define OLED_SCL          5        // D1
#define OLED_WIDTH        128
#define OLED_HEIGHT       64
#define OLED_RESET        -1
#define OLED_I2C_ADDR     0x3C

// ─── WiFi Access Point ──────────────────────────────────────────
#define AP_SSID           "OLED-Setup"
#define AP_PASSWORD       "12345678"
#define AP_IP_STR         "192.168.4.1"

// ─── NTP / Time ─────────────────────────────────────────────────
#define NTP_SERVER        "pool.ntp.org"
#define UTC_OFFSET_SEC    21600    // UTC+6 (Bangladesh). Change for your timezone.
                                   // London=0, Berlin=3600, India=19800, EST=-18000, PST=-28800

// ─── Chuck Norris Jokes ─────────────────────────────────────────
#define JOKE_API_HOST     "api.chucknorris.io"
#define JOKE_API_PATH     "/jokes/random"
#define JOKE_INTERVAL     300000   // 5 minutes in ms
#define JOKE_DISPLAY_MS   30000    // show joke for 30 seconds

// ─── Timeouts ───────────────────────────────────────────────────
#define CONNECT_TIMEOUT   20000    // 20 seconds to connect to WiFi
#define RESET_HOLD_MS     3000     // hold FLASH button 3s for factory reset

// ─── EEPROM ─────────────────────────────────────────────────────
#define EEPROM_SIZE       128
#define EEPROM_SSID_ADDR  0        // bytes 0–63  → SSID
#define EEPROM_PASS_ADDR  64       // bytes 64–127 → Password

// ─── Hardware ───────────────────────────────────────────────────
#define GPIO_RESET_BUTTON 0        // FLASH button = GPIO0

// ─── Debug ──────────────────────────────────────────────────────
#define ENABLE_DEBUG      true
#define DEBUG_BAUD        115200

// ─── State Machine ──────────────────────────────────────────────
enum SystemState {
  STATE_AP_MODE,
  STATE_CONNECTING,
  STATE_CONNECTED,
  STATE_ERROR
};
