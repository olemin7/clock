/*
 * CRTCWraper.h
 *
 *  Created on: 11 ???. 2017 ?.
 *      Author: User
 */

#ifndef CLOCK_CRTCWRAPER_H_
#define CLOCK_CRTCWRAPER_H_
#include <Wire.h> // must be included here so that Arduino library object file references work
#include <RtcDS3231.h>
#include <pgmspace.h>

class CRTCWraper {
	RtcDS3231<TwoWire> Rtc;
public:
	CRTCWraper();
	int setTime(const int32 time);
	int getTime(int32 &time);
	float GetTemperature(){
		return Rtc.GetTemperature().AsFloat();
	}
	virtual ~CRTCWraper();
};

#endif /* CLOCK_CRTCWRAPER_H_ */
