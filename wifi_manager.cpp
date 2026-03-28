#include "wifi_manager.h"

WiFiManager* WiFiManager::_instance = nullptr;

WiFiManager::WiFiManager()
  : _server(80), _apActive(false) {
  _instance = this;
}

void WiFiManager::begin() {
  EEPROM.begin(EEPROM_SIZE);
  WiFi.mode(WIFI_STA);
  WiFi.disconnect();
  delay(100);
  Serial.println("[WIFI] Manager initialized");
}

// ─── AP Mode ───────────────────────────────────────────────────
void WiFiManager::startAPMode() {
  WiFi.mode(WIFI_AP);
  IPAddress apIP(192, 168, 4, 1);
  WiFi.softAPConfig(apIP, apIP, IPAddress(255, 255, 255, 0));
  WiFi.softAP(AP_SSID, AP_PASSWORD);

  // DNS: redirect all domains to 192.168.4.1 (captive portal)
  _dnsServer.start(53, "*", apIP);

  _setupRoutes();
  _server.begin();
  _apActive = true;

  Serial.println("[WIFI] AP started: " AP_SSID);
  Serial.print("[WIFI] AP IP: ");
  Serial.println(WiFi.softAPIP());
}

void WiFiManager::stopAPMode() {
  if (_apActive) {
    _server.stop();
    _dnsServer.stop();
    WiFi.softAPdisconnect(true);
    _apActive = false;
    Serial.println("[WIFI] AP stopped");
  }
}

void WiFiManager::handleAPRequests() {
  _dnsServer.processNextRequest();
  _server.handleClient();
}

// ─── Routes ────────────────────────────────────────────────────
void WiFiManager::_setupRoutes() {
  _server.on("/", HTTP_GET,  [this]() { _handleRoot(); });
  _server.on("/connect", HTTP_POST, [this]() { _handleConnect(); });
  _server.on("/status", HTTP_GET,   [this]() { _handleStatus(); });
  _server.on("/reset", HTTP_POST,   [this]() { _handleReset(); });
  _server.on("/scan", HTTP_GET,     [this]() { _handleScan(); });
  _server.onNotFound([this]() {
    // Captive portal: redirect everything to setup page
    _server.sendHeader("Location", "http://" AP_IP_STR, true);
    _server.send(302, "text/plain", "");
  });
}

void WiFiManager::_handleRoot() {
  String ssidOptions = _scanNetworks();
  _server.send(200, "text/html", _buildSetupPage(ssidOptions));
}

void WiFiManager::_handleConnect() {
  String ssid     = _server.arg("ssid");
  String password = _server.arg("password");

  if (ssid.length() == 0) {
    _server.send(400, "text/html", "<h2>Error: SSID cannot be empty</h2>");
    return;
  }

  Serial.print("[WIFI] Received credentials for: ");
  Serial.println(ssid);

  saveCredentials(ssid, password);
  WiFi.mode(WIFI_AP_STA);
  WiFi.begin(ssid.c_str(), password.c_str());

  _server.sendHeader("Location", "/status", true);
  _server.send(302, "text/plain", "");
}

void WiFiManager::_handleStatus() {
  String statusText = (WiFi.status() == WL_CONNECTED) ? "Connected" : "Connecting";
  _server.send(200, "text/html", _buildStatusPage(statusText));
}

void WiFiManager::_handleReset() {
  clearCredentials();
  _server.send(200, "text/html", "<h2>Factory reset done. Restarting...</h2>");
  delay(1000);
  ESP.restart();
}

void WiFiManager::_handleScan() {
  String json = "[";
  int n = WiFi.scanNetworks();
  for (int i = 0; i < n; i++) {
    if (i > 0) json += ",";
    json += "\"" + WiFi.SSID(i) + "\"";
  }
  json += "]";
  _server.send(200, "application/json", json);
}

// ─── WiFi Connection ───────────────────────────────────────────
bool WiFiManager::connectToWiFi(const String& ssid, const String& password) {
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid.c_str(), password.c_str());
  Serial.print("[WIFI] Connecting to: ");
  Serial.println(ssid);
  return true; // non-blocking; caller checks WiFi.status()
}

int WiFiManager::getRSSI() {
  return WiFi.RSSI();
}

// ─── EEPROM ────────────────────────────────────────────────────
void WiFiManager::saveCredentials(const String& ssid, const String& password) {
  for (int i = 0; i < 64; i++) EEPROM.write(EEPROM_SSID_ADDR + i, 0);
  for (int i = 0; i < 64; i++) EEPROM.write(EEPROM_PASS_ADDR + i, 0);

  for (int i = 0; i < (int)ssid.length() && i < 63; i++)
    EEPROM.write(EEPROM_SSID_ADDR + i, ssid[i]);

  for (int i = 0; i < (int)password.length() && i < 63; i++)
    EEPROM.write(EEPROM_PASS_ADDR + i, password[i]);

  EEPROM.commit();
  Serial.println("[WIFI] Credentials saved to EEPROM");
}

bool WiFiManager::loadCredentials(String& ssid, String& password) {
  ssid = "";
  password = "";

  for (int i = 0; i < 63; i++) {
    char c = EEPROM.read(EEPROM_SSID_ADDR + i);
    if (c == 0) break;
    ssid += c;
  }
  for (int i = 0; i < 63; i++) {
    char c = EEPROM.read(EEPROM_PASS_ADDR + i);
    if (c == 0) break;
    password += c;
  }

  bool valid = ssid.length() > 0;
  Serial.print("[WIFI] Loaded credentials — SSID: ");
  Serial.println(valid ? ssid : "(none)");
  return valid;
}

void WiFiManager::clearCredentials() {
  for (int i = 0; i < EEPROM_SIZE; i++) EEPROM.write(i, 0);
  EEPROM.commit();
  Serial.println("[WIFI] EEPROM credentials cleared");
}

// ─── HTML Pages ────────────────────────────────────────────────
String WiFiManager::_scanNetworks() {
  int n = WiFi.scanNetworks();
  String options = "";
  for (int i = 0; i < n; i++) {
    options += "<option value=\"" + WiFi.SSID(i) + "\">" + WiFi.SSID(i) + " (" + String(WiFi.RSSI(i)) + " dBm)</option>";
  }
  return options;
}

String WiFiManager::_buildSetupPage(const String& ssidOptions) {
  return R"rawhtml(
<!DOCTYPE html>
<html>
<head>
  <meta charset="UTF-8">
  <meta name="viewport" content="width=device-width, initial-scale=1">
  <title>Connect OLED to WiFi</title>
  <style>
    * { box-sizing: border-box; }
    body { font-family: -apple-system, sans-serif; background: #1a1a2e; color: #eee; margin: 0; padding: 20px; }
    .card { background: #16213e; border-radius: 16px; padding: 24px; max-width: 400px; margin: auto; }
    h1 { color: #e94560; margin-top: 0; font-size: 1.4em; }
    label { display: block; margin-bottom: 6px; color: #a0a0c0; font-size: 0.9em; }
    select, input { width: 100%; padding: 12px; border-radius: 8px; border: 1px solid #333; background: #0f3460; color: #fff; font-size: 1em; margin-bottom: 16px; }
    button { width: 100%; padding: 14px; background: #e94560; color: #fff; border: none; border-radius: 8px; font-size: 1em; cursor: pointer; font-weight: bold; }
    button:active { background: #c73652; }
    .note { font-size: 0.8em; color: #666; margin-top: 12px; text-align: center; }
  </style>
</head>
<body>
  <div class="card">
    <h1>📡 Connect OLED to WiFi</h1>
    <form action="/connect" method="POST">
      <label>Select Network</label>
      <select name="ssid">)rawhtml" + ssidOptions + R"rawhtml(</select>
      <label>WiFi Password</label>
      <input type="password" name="password" placeholder="Enter password">
      <button type="submit">Connect</button>
    </form>
    <p class="note">Device will restart after connecting</p>
  </div>
</body>
</html>
)rawhtml";
}

String WiFiManager::_buildStatusPage(const String& status) {
  bool connected = (status == "Connected");
  return "<!DOCTYPE html><html><head>"
         "<meta charset='UTF-8'>"
         "<meta name='viewport' content='width=device-width, initial-scale=1'>"
         + String(connected ? "" : "<meta http-equiv='refresh' content='3'>") +
         "<title>Status</title>"
         "<style>body{font-family:-apple-system,sans-serif;background:#1a1a2e;color:#eee;display:flex;align-items:center;justify-content:center;height:100vh;margin:0;}"
         ".card{background:#16213e;border-radius:16px;padding:32px;text-align:center;}"
         "h2{color:" + String(connected ? "#4ecca3" : "#e94560") + ";}"
         ".dot{display:inline-block;width:10px;height:10px;border-radius:50%;background:#e94560;animation:pulse 1s infinite alternate;margin:0 3px;}"
         "@keyframes pulse{from{opacity:0.3}to{opacity:1}}"
         "</style></head><body>"
         "<div class='card'>"
         "<h2>" + status + "</h2>" +
         (connected ? "<p>Your OLED clock is now running!</p>" :
                      "<p><span class='dot'></span><span class='dot' style='animation-delay:.2s'></span><span class='dot' style='animation-delay:.4s'></span></p><p>Trying to connect...</p>") +
         "</div></body></html>";
}
