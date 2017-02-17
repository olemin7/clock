/*
 * CDisplayClock.cpp
 *
 *  Created on: Feb 17, 2017
 *      Author: ominenko
 */

#include "CDisplayClock.h"

const char CDisplayClock::formatMin[] PROGMEM = "%02u:%02u";
const TimeChangeRule CDisplayClock::myDST PROGMEM = {"DST", Last, Sun,
                                                     Mar,   3,    +3 * 60};
const TimeChangeRule CDisplayClock::mySTD PROGMEM = {"STD", Last, Sun,
                                                     Oct,   4,    +2 * 60};

CDisplayClock::CDisplayClock() : myTZ(myDST, mySTD) {
  memset(buffMin, 0, sizeof(buffMin));
}
time_t CDisplayClock::getLocalTime() { return myTZ.toLocal(now()); }

bool CDisplayClock::isChangedMin() {
  char tmp[sizeof(buffMin)];
  getStrMin(tmp);
  return memcmp(tmp, buffMin, sizeof(buffMin)) ? true : false;
}
int CDisplayClock::getStrMin(char *Buff) {
  time_t local = getLocalTime();
  return sprintf_P(Buff, formatMin, hour(local), minute(local));
}
char *CDisplayClock::getStrMin() {
  getStrMin(buffMin);
  return buffMin;
}
