#include "display_manager.h"

DisplayManager::DisplayManager()
  : _display(OLED_WIDTH, OLED_HEIGHT, &Wire, OLED_RESET) {}

bool DisplayManager::begin() {
  Wire.begin(OLED_SDA, OLED_SCL);
  if (!_display.begin(SSD1306_SWITCHCAPVCC, OLED_I2C_ADDR)) {
    Serial.println("[DISPLAY] SSD1306 allocation failed");
    return false;
  }
  _display.clearDisplay();
  _display.setTextColor(SSD1306_WHITE);
  _display.display();
  Serial.println("[DISPLAY] Initialized OK");
  return true;
}

void DisplayManager::clear() {
  _display.clearDisplay();
  _display.display();
}

// ─── AP Mode Screen ────────────────────────────────────────────
void DisplayManager::showAPMode() {
  _display.clearDisplay();
  _display.setTextSize(1);
  _display.setCursor(0, 0);
  _display.println("WiFi Setup");
  _display.println("SSID: " AP_SSID);
  _display.println("Pass: " AP_PASSWORD);
  _display.println("IP: " AP_IP_STR);
  _display.println("");
  _display.println("Open browser to");
  _display.println("192.168.4.1");
  _display.display();
}

// ─── Connecting Screen (animated dots) ────────────────────────
void DisplayManager::showConnecting(int dotCount) {
  _display.clearDisplay();
  _display.setTextSize(1);
  _drawCenteredText("Connecting", 20);

  String dots = "";
  for (int i = 0; i < (dotCount % 4); i++) dots += ". ";
  _drawCenteredText(dots, 36);

  _display.display();
}

// ─── Clock Screen ──────────────────────────────────────────────
void DisplayManager::showClock(const String& timeStr, const String& dateStr, int rssi, int secondsUntilJoke) {
  _display.clearDisplay();

  // Large time (size 2 = 12px tall)
  _display.setTextSize(2);
  _drawCenteredText(timeStr, 0, 2);

  // Date
  _display.setTextSize(1);
  _drawCenteredText(dateStr, 20);

  // WiFi RSSI
  String rssiStr = "WiFi: " + String(rssi) + " dBm";
  _drawCenteredText(rssiStr, 36);

  // Countdown to joke
  String jokeStr = "Joke in " + String(secondsUntilJoke) + "s";
  _drawCenteredText(jokeStr, 48);

  _display.display();
}

// ─── Joke Screen ───────────────────────────────────────────────
void DisplayManager::showJoke(const String& joke, int page, int totalPages) {
  _display.clearDisplay();
  _display.setTextSize(1);
  _display.setCursor(0, 0);
  _display.println("Chuck Norris:");
  _display.println(""); // blank line

  // ~21 chars per line, ~4 lines available (rows 2–5)
  int charsPerPage = 21 * 4;
  String pageText = _getJokePage(joke, page - 1, charsPerPage);
  _printWrapped(pageText, 0, 16, 21, 9);

  // Page indicator bottom right
  String pageIndicator = "(" + String(page) + "/" + String(totalPages) + ")";
  _display.setCursor(128 - (pageIndicator.length() * 6), 56);
  _display.print(pageIndicator);

  _display.display();
}

// ─── Success / Info Screen ─────────────────────────────────────
void DisplayManager::showSuccess(const String& msg) {
  _display.clearDisplay();
  _display.setTextSize(1);
  _drawCenteredText("", 16);
  _drawCenteredText(msg, 28);
  _display.display();
}

// ─── Error Screen ──────────────────────────────────────────────
void DisplayManager::showError(const String& msg) {
  _display.clearDisplay();
  _display.setTextSize(1);
  _drawCenteredText("! ERROR !", 16);
  _drawCenteredText(msg, 32);
  _drawCenteredText("Restarting...", 48);
  _display.display();
}

// ─── Helpers ───────────────────────────────────────────────────
void DisplayManager::_drawCenteredText(const String& text, int y, uint8_t size) {
  _display.setTextSize(size);
  int16_t x1, y1;
  uint16_t w, h;
  _display.getTextBounds(text, 0, 0, &x1, &y1, &w, &h);
  int x = (OLED_WIDTH - w) / 2;
  if (x < 0) x = 0;
  _display.setCursor(x, y);
  _display.print(text);
}

void DisplayManager::_printWrapped(const String& text, int x, int y, int maxWidth, int lineHeight) {
  int cursorX = x;
  int cursorY = y;
  String word = "";

  for (int i = 0; i <= (int)text.length(); i++) {
    char c = (i < (int)text.length()) ? text[i] : ' ';

    if (c == ' ' || c == '\n' || i == (int)text.length()) {
      if (cursorX + (int)word.length() * 6 > x + maxWidth * 6) {
        cursorX = x;
        cursorY += lineHeight;
        if (cursorY > OLED_HEIGHT - lineHeight) break;
      }
      _display.setCursor(cursorX, cursorY);
      _display.print(word);
      cursorX += word.length() * 6 + 6;
      word = "";
      if (c == '\n') {
        cursorX = x;
        cursorY += lineHeight;
      }
    } else {
      word += c;
    }
  }
}

String DisplayManager::_getJokePage(const String& joke, int page, int charsPerPage) {
  int start = page * charsPerPage;
  if (start >= (int)joke.length()) return "";
  return joke.substring(start, min((int)joke.length(), start + charsPerPage));
}
