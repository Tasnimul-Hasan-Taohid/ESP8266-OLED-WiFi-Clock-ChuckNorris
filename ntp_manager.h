#pragma once

#include <NTPClient.h>
#include <WiFiUdp.h>
#include "config.h"

class NTPManager {
public:
  NTPManager();

  void begin();
  void update();

  bool isTimeSet();
  String getTimeString();   // HH:MM:SS
  String getDateString();   // e.g. "Sat 28 Mar 2026"
  unsigned long getEpoch();

private:
  WiFiUDP _udp;
  NTPClient* _ntp;

  String _dayName(int dayOfWeek);
  String _monthName(int month);
  void _epochToDate(unsigned long epoch, int& day, int& month, int& year, int& dayOfWeek);
};
