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
public:
	CLightDetectResistor();
	int get();
	virtual ~CLightDetectResistor();
};

#endif /* CLOCK_CLIGHTDETECTRESISTOR_H_ */
