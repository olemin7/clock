/*
 * CDisplayClock.h
 *
 *  Created on: Feb 17, 2017
 *      Author: ominenko
 */

#ifndef CLOCK_CDISPLAYCLOCK_H_
#define CLOCK_CDISPLAYCLOCK_H_
#include <Timezone.h>

class CDisplayClock {
	char buffMin[6];
	static const char formatMin[];
	static const TimeChangeRule myDST;
	static const TimeChangeRule mySTD;
	Timezone myTZ;
	time_t getLocalTime();
	int getStrMin(char *Buff);
public:
	CDisplayClock();
	bool isChangedMin();
	char* getStrMin();
};

#endif /* CLOCK_CDISPLAYCLOCK_H_ */
