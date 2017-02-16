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
	int32 convertTime(int32 time,int32 offset,bool summerAuto);
	CRTCWraper();
	int setTime(const int32 time);
	int getTime(int32 &time);
	int getTime(int32 &time,int32 offset,bool summerAuto);
	RtcDS3231& getRtc(){return Rtc;}
	float GetTemperature(){
		return Rtc.GetTemperature().AsFloat();
	}
	virtual ~CRTCWraper();
};

#endif /* CLOCK_CRTCWRAPER_H_ */
