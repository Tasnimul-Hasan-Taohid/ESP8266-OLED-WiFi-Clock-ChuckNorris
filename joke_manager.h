#pragma once

#include <ESP8266HTTPClient.h>
#include <ESP8266WiFi.h>
#include <ArduinoJson.h>
#include "config.h"

class JokeManager {
public:
  JokeManager();

  void begin();

  bool fetchJoke(String& jokeOut);
  bool shouldFetchJoke();
  int  getSecondsUntilNextJoke();

private:
  unsigned long _lastFetchTime;
  bool _firstFetch;
};
