/*
 * CDisplayClock.h
 *
 *  Created on: Feb 17, 2017
 *      Author: ominenko
 */

#ifndef CLOCK_CDISPLAYCLOCK_H_
#define CLOCK_CDISPLAYCLOCK_H_
#include <TimeLib.h>
#include <Timezone.h>

class CDisplayClock {
  char buffMin[6];
  static const char formatMin[];
  static const TimeChangeRule myDST;
  static const TimeChangeRule mySTD;
  Timezone myTZ;
  int getStrMin(char *Buff);

public:
  CDisplayClock();
  time_t getLocalTime();
  time_t toUTC(time_t local);
  bool isChangedMin();
  char *getStrMin();
  int getFullTime(char *Buff);
};

#endif /* CLOCK_CDISPLAYCLOCK_H_ */
