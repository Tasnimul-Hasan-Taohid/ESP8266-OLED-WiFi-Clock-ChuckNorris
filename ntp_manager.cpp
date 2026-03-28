#include "ntp_manager.h"

NTPManager::NTPManager() : _ntp(nullptr) {}

void NTPManager::begin() {
  if (_ntp) {
    delete _ntp;
  }
  _ntp = new NTPClient(_udp, NTP_SERVER, UTC_OFFSET_SEC, 60000);
  _ntp->begin();
  _ntp->update();
  Serial.println("[NTP] Client started");
}

void NTPManager::update() {
  if (_ntp) _ntp->update();
}

bool NTPManager::isTimeSet() {
  if (!_ntp) return false;
  unsigned long epoch = _ntp->getEpochTime();
  // Year must be > 2020 to be valid
  return epoch > 1577836800UL;
}

unsigned long NTPManager::getEpoch() {
  if (!_ntp) return 0;
  return _ntp->getEpochTime();
}

String NTPManager::getTimeString() {
  if (!_ntp) return "--:--:--";
  int h = _ntp->getHours();
  int m = _ntp->getMinutes();
  int s = _ntp->getSeconds();

  char buf[9];
  snprintf(buf, sizeof(buf), "%02d:%02d:%02d", h, m, s);
  return String(buf);
}

String NTPManager::getDateString() {
  if (!_ntp) return "--- -- --- ----";
  unsigned long epoch = _ntp->getEpochTime();

  int day, month, year, dayOfWeek;
  _epochToDate(epoch, day, month, year, dayOfWeek);

  char buf[20];
  snprintf(buf, sizeof(buf), "%s %02d %s %04d",
    _dayName(dayOfWeek).c_str(), day, _monthName(month).c_str(), year);
  return String(buf);
}

// ─── Helpers ───────────────────────────────────────────────────

String NTPManager::_dayName(int d) {
  const char* days[] = {"Sun","Mon","Tue","Wed","Thu","Fri","Sat"};
  if (d < 0 || d > 6) return "???";
  return days[d];
}

String NTPManager::_monthName(int m) {
  const char* months[] = {"","Jan","Feb","Mar","Apr","May","Jun","Jul","Aug","Sep","Oct","Nov","Dec"};
  if (m < 1 || m > 12) return "???";
  return months[m];
}

void NTPManager::_epochToDate(unsigned long epoch, int& day, int& month, int& year, int& dayOfWeek) {
  // Days since Unix epoch
  unsigned long days = epoch / 86400UL;
  dayOfWeek = (days + 4) % 7; // 0=Sun

  // Calculate year
  year = 1970;
  while (true) {
    bool leap = (year % 4 == 0 && (year % 100 != 0 || year % 400 == 0));
    unsigned long daysInYear = leap ? 366 : 365;
    if (days < daysInYear) break;
    days -= daysInYear;
    year++;
  }

  // Calculate month
  bool leap = (year % 4 == 0 && (year % 100 != 0 || year % 400 == 0));
  int daysInMonth[] = {31, (int)(leap ? 29 : 28), 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};
  month = 0;
  for (int i = 0; i < 12; i++) {
    if ((int)days < daysInMonth[i]) { month = i + 1; break; }
    days -= daysInMonth[i];
  }
  day = (int)days + 1;
}
