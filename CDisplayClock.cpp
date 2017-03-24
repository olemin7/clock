/*
 * CDisplayClock.cpp
 *
 *  Created on: Feb 17, 2017
 *      Author: ominenko
 */

#include "CDisplayClock.h"

const char CDisplayClock::formatMin[] PROGMEM = "%02u:%02u";
const char formatTimeFull[] PROGMEM = "%u-%02u-%02u %u:%02u:%02u";
const TimeChangeRule CDisplayClock::myDST PROGMEM = {"DST", Last, Sun,
                                                     Mar,   3,    +3 * 60};
const TimeChangeRule CDisplayClock::mySTD PROGMEM = {"STD", Last, Sun,
                                                     Oct,   4,    +2 * 60};

CDisplayClock::CDisplayClock() : myTZ(myDST, mySTD) {
  memset(buffMin, 0, sizeof(buffMin));
}
time_t CDisplayClock::getLocalTime() { return myTZ.toLocal(now()); }
time_t CDisplayClock::toUTC(time_t local){ return myTZ.toUTC(local); }
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
int CDisplayClock::getFullTime(char *Buff) {
	time_t local = getLocalTime();
	return sprintf_P(Buff, formatTimeFull, year(local),month(local),day(local),hour(local), minute(local),second(local));
}
