#pragma once

#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include "config.h"

class DisplayManager {
public:
  DisplayManager();

  bool begin();

  void showAPMode();
  void showConnecting(int dotCount);
  void showClock(const String& timeStr, const String& dateStr, int rssi, int secondsUntilJoke);
  void showJoke(const String& joke, int page, int totalPages);
  void showSuccess(const String& msg);
  void showError(const String& msg);
  void clear();

private:
  Adafruit_SSD1306 _display;

  void _drawCenteredText(const String& text, int y, uint8_t size = 1);
  void _printWrapped(const String& text, int x, int y, int maxWidth, int lineHeight);
  String _getJokePage(const String& joke, int page, int charsPerPage);
};
