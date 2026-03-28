#include "config.h"
#include "display_manager.h"
#include "wifi_manager.h"
#include "ntp_manager.h"
#include "joke_manager.h"

// ─── Global Managers ───────────────────────────────────────────
DisplayManager display;
WiFiManager    wifiMgr;
NTPManager     ntpMgr;
JokeManager    jokeMgr;

// ─── State Machine ─────────────────────────────────────────────
SystemState   currentState     = STATE_AP_MODE;
unsigned long stateChangeTime  = 0;

// Connecting animation
unsigned long lastDotTime = 0;
int           dotCount    = 0;

// Joke display
bool          showingJoke        = false;
unsigned long jokeDisplayStart   = 0;
String        currentJoke        = "";

// Factory reset button
bool          resetBtnPressed    = false;
unsigned long resetBtnPressTime  = 0;

// ─── Forward Declarations ──────────────────────────────────────
void changeState(SystemState s);
void checkResetButton();
void runAPMode();
void runConnecting();
void runConnected();
void runError();

// ──────────────────────────────────────────────────────────────
void setup() {
  Serial.begin(DEBUG_BAUD);
  delay(100);
  Serial.println("\n\nESP8266 OLED WiFi Clock & Chuck Norris Display");
  Serial.println("================================================");

  // Init display first so user sees something immediately
  if (!display.begin()) {
    Serial.println("[SETUP] FATAL: Display init failed");
    while (1) yield();
  }

  // Init managers
  wifiMgr.begin();
  jokeMgr.begin();

  // Factory reset button: active LOW (GPIO0 = FLASH)
  pinMode(GPIO_RESET_BUTTON, INPUT);

  // Try saved credentials
  String savedSSID, savedPass;
  if (wifiMgr.loadCredentials(savedSSID, savedPass)) {
    Serial.println("[SETUP] Credentials found — connecting");
    wifiMgr.connectToWiFi(savedSSID, savedPass);
    changeState(STATE_CONNECTING);
  } else {
    Serial.println("[SETUP] No credentials — starting AP");
    wifiMgr.startAPMode();
    changeState(STATE_AP_MODE);
  }

  Serial.println("[SETUP] Done");
}

void loop() {
  checkResetButton();

  switch (currentState) {
    case STATE_AP_MODE:   runAPMode();    break;
    case STATE_CONNECTING: runConnecting(); break;
    case STATE_CONNECTED: runConnected(); break;
    case STATE_ERROR:     runError();     break;
  }

  yield();
}

// ─── Factory Reset Button ──────────────────────────────────────
void checkResetButton() {
  if (digitalRead(GPIO_RESET_BUTTON) == LOW) {
    if (!resetBtnPressed) {
      resetBtnPressed   = true;
      resetBtnPressTime = millis();
      Serial.println("[RESET] Button held...");
    } else if (millis() - resetBtnPressTime >= RESET_HOLD_MS) {
      Serial.println("[RESET] Factory reset triggered!");
      wifiMgr.clearCredentials();
      display.showError("Factory Reset");
      delay(2000);
      ESP.restart();
    }
  } else {
    resetBtnPressed = false;
  }
}

// ─── State: AP Mode ────────────────────────────────────────────
void runAPMode() {
  wifiMgr.handleAPRequests();
  display.showAPMode();
  delay(10);
}

// ─── State: Connecting ─────────────────────────────────────────
void runConnecting() {
  wl_status_t status = WiFi.status();

  if (status == WL_CONNECTED) {
    Serial.println("[CONNECTING] WiFi joined!");
    Serial.print("[CONNECTING] IP: ");
    Serial.println(WiFi.localIP());

    wifiMgr.stopAPMode();
    delay(300);

    ntpMgr.begin();
    delay(500);

    // Fetch first joke immediately
    if (jokeMgr.fetchJoke(currentJoke)) {
      Serial.println("[CONNECTING] First joke ready");
      showingJoke      = true;
      jokeDisplayStart = millis();
    } else {
      currentJoke  = "Chuck is busy saving the world right now...";
      showingJoke  = true;
      jokeDisplayStart = millis();
    }

    display.showSuccess("Connected!");
    delay(1000);
    changeState(STATE_CONNECTED);
    return;
  }

  // Timeout check
  if (millis() - stateChangeTime > CONNECT_TIMEOUT) {
    Serial.println("[CONNECTING] Timeout — back to AP mode");
    wifiMgr.clearCredentials();
    wifiMgr.startAPMode();
    changeState(STATE_AP_MODE);
    return;
  }

  // Animated dots
  if (millis() - lastDotTime > 500) {
    dotCount++;
    lastDotTime = millis();
  }
  display.showConnecting(dotCount);
  delay(10);
}

// ─── State: Connected ──────────────────────────────────────────
void runConnected() {
  // Check WiFi still alive
  if (WiFi.status() != WL_CONNECTED) {
    Serial.println("[CONNECTED] WiFi lost — going to ERROR");
    changeState(STATE_ERROR);
    return;
  }

  ntpMgr.update();

  if (showingJoke) {
    // Paginate long jokes (scroll every 2s)
    unsigned long elapsed = millis() - jokeDisplayStart;
    int charsPerPage = 21 * 4;
    int totalPages   = max(1, (int)((currentJoke.length() + charsPerPage - 1) / charsPerPage));
    int page         = (elapsed / 2000) % totalPages;
    display.showJoke(currentJoke, page + 1, totalPages);

    if (elapsed >= JOKE_DISPLAY_MS) {
      Serial.println("[CONNECTED] Joke done — back to clock");
      showingJoke = false;
    }
  } else {
    // Clock display
    if (ntpMgr.isTimeSet()) {
      display.showClock(
        ntpMgr.getTimeString(),
        ntpMgr.getDateString(),
        wifiMgr.getRSSI(),
        jokeMgr.getSecondsUntilNextJoke()
      );
    } else {
      display.showSuccess("Syncing time...");
    }

    // Time to fetch a new joke?
    if (jokeMgr.shouldFetchJoke()) {
      Serial.println("[CONNECTED] Fetching new joke...");
      if (!jokeMgr.fetchJoke(currentJoke)) {
        currentJoke = "Chuck is busy saving the world right now...";
      }
      showingJoke      = true;
      jokeDisplayStart = millis();
    }
  }

  delay(100);
}

// ─── State: Error ──────────────────────────────────────────────
void runError() {
  display.showError("WiFi Disconnected");
  delay(3000);
  Serial.println("[ERROR] Restarting AP mode...");
  wifiMgr.clearCredentials();
  wifiMgr.startAPMode();
  changeState(STATE_AP_MODE);
}

// ─── State Transition ──────────────────────────────────────────
void changeState(SystemState newState) {
  if (currentState == newState) return;
  currentState    = newState;
  stateChangeTime = millis();
  dotCount        = 0;
  lastDotTime     = 0;

  if (ENABLE_DEBUG) {
    const char* names[] = {"AP_MODE","CONNECTING","CONNECTED","ERROR"};
    Serial.print("[STATE] → ");
    Serial.println(names[(int)newState]);
  }
}
