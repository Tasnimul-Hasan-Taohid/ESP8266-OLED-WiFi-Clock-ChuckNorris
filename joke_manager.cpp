#include "joke_manager.h"

JokeManager::JokeManager()
  : _lastFetchTime(0), _firstFetch(true) {}

void JokeManager::begin() {
  _lastFetchTime = 0;
  _firstFetch = true;
  Serial.println("[JOKE] Manager initialized");
}

bool JokeManager::fetchJoke(String& jokeOut) {
  if (WiFi.status() != WL_CONNECTED) {
    Serial.println("[JOKE] WiFi not connected, skipping fetch");
    return false;
  }

  WiFiClientSecure client;
  client.setInsecure(); // Skip SSL cert verification (fine for this use case)

  HTTPClient http;
  String url = "https://" JOKE_API_HOST JOKE_API_PATH;

  Serial.print("[JOKE] Fetching: ");
  Serial.println(url);

  http.begin(client, url);
  http.addHeader("User-Agent", "ESP8266-OLED-Clock/1.0");
  http.setTimeout(8000);

  int httpCode = http.GET();
  Serial.print("[JOKE] HTTP response: ");
  Serial.println(httpCode);

  if (httpCode != HTTP_CODE_OK) {
    Serial.println("[JOKE] Fetch failed — HTTP error");
    http.end();
    _lastFetchTime = millis(); // Reset timer anyway
    _firstFetch = false;
    return false;
  }

  String payload = http.getString();
  http.end();

  // Parse JSON
  StaticJsonDocument<1024> doc;
  DeserializationError err = deserializeJson(doc, payload);
  if (err) {
    Serial.print("[JOKE] JSON parse error: ");
    Serial.println(err.c_str());
    _lastFetchTime = millis();
    _firstFetch = false;
    return false;
  }

  const char* value = doc["value"];
  if (!value || strlen(value) == 0) {
    Serial.println("[JOKE] Empty joke in response");
    _lastFetchTime = millis();
    _firstFetch = false;
    return false;
  }

  jokeOut = String(value);
  _lastFetchTime = millis();
  _firstFetch = false;

  Serial.print("[JOKE] Got joke: ");
  Serial.println(jokeOut.substring(0, 60) + "...");
  return true;
}

bool JokeManager::shouldFetchJoke() {
  if (_firstFetch) return false; // First joke is fetched manually on connect
  return (millis() - _lastFetchTime) >= JOKE_INTERVAL;
}

int JokeManager::getSecondsUntilNextJoke() {
  if (_firstFetch) return JOKE_INTERVAL / 1000;
  unsigned long elapsed = millis() - _lastFetchTime;
  if (elapsed >= JOKE_INTERVAL) return 0;
  return (JOKE_INTERVAL - elapsed) / 1000;
}
