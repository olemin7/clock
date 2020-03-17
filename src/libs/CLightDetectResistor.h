/*
 * CLightSensor.h
 *
 *  Created on: 11 ???. 2017 ?.
 *      Author: User
 */

#ifndef CLOCK_CLIGHTDETECTRESISTOR_H_
#define CLOCK_CLIGHTDETECTRESISTOR_H_
/*
           PhotoR     10K
 +3    o---/\/\/--.--/\/\/---o GND
                  |
 Pin 0 o-----------
*/

class CLightDetectResistor {
    int cacheVal = 0;
    static const int maxRefresh = 100; // ms
    unsigned long nextRead = 0;
public:
	CLightDetectResistor();
	int get();
	virtual ~CLightDetectResistor();
};

#endif /* CLOCK_CLIGHTDETECTRESISTOR_H_ */
