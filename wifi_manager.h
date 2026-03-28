#pragma once

#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <DNSServer.h>
#include <EEPROM.h>
#include "config.h"

class WiFiManager {
public:
  WiFiManager();

  void begin();

  // AP mode
  void startAPMode();
  void stopAPMode();
  void handleAPRequests();

  // WiFi connection
  bool connectToWiFi(const String& ssid, const String& password);

  // EEPROM credential management
  bool loadCredentials(String& ssid, String& password);
  void saveCredentials(const String& ssid, const String& password);
  void clearCredentials();

  // Status
  int getRSSI();

private:
  ESP8266WebServer _server;
  DNSServer _dnsServer;
  bool _apActive;

  void _setupRoutes();
  void _handleRoot();
  void _handleConnect();
  void _handleStatus();
  void _handleReset();
  void _handleScan();

  String _scanNetworks();
  String _buildSetupPage(const String& ssidOptions);
  String _buildStatusPage(const String& status);

  static WiFiManager* _instance;
};
