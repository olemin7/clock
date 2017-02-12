/*
 * CRTCWraper.cpp
 *
 *  Created on: 11 ???. 2017 ?.
 *      Author: User
 */

#include "CRTCWraper.h"



CRTCWraper::CRTCWraper():Rtc(Wire) {

	//--------RTC SETUP ------------
	Rtc.Begin();

	// if you are using ESP-01 then uncomment the line below to reset the pins to
	// the available pins for SDA, SCL
	// Wire.begin(0, 2); // due to limited pins, use pin 0 and 2 for SDA, SCL

	RtcDateTime compiled = RtcDateTime(__DATE__, __TIME__);
	Serial.println();

	if (!Rtc.IsDateTimeValid())
	{
	    // Common Cuases:
	    //    1) first time you ran and the device wasn't running yet
	    //    2) the battery on the device is low or even missing

	    Serial.println("RTC lost confidence in the DateTime!");

	    // following line sets the RTC to the date & time this sketch was compiled
	    // it will also reset the valid flag internally unless the Rtc device is
	    // having an issue

	    Rtc.SetDateTime(compiled);
	}

	if (!Rtc.GetIsRunning())
	{
	    Serial.println("RTC was not actively running, starting now");
	    Rtc.SetIsRunning(true);
	}

	RtcDateTime now = Rtc.GetDateTime();
	if (now < compiled)
	{
	    Serial.println("RTC is older than compile time!  (Updating DateTime)");
	    Rtc.SetDateTime(compiled);
	}
	else if (now > compiled)
	{
	    Serial.println("RTC is newer than compile time. (this is expected)");
	}
	else if (now == compiled)
	{
	    Serial.println("RTC is the same as compile time! (not expected but all is fine)");
	}

	// never assume the Rtc was last configured by you, so
	// just clear them to your needed state
	Rtc.Enable32kHzPin(false);
	Rtc.SetSquareWavePin(DS3231SquareWavePin_ModeNone);

}
int CRTCWraper::setTime(const int32 time){
	RtcDateTime dt;
	dt.InitWithEpoch32Time(time);
	Rtc.SetDateTime(dt);
	return 0;
}
int CRTCWraper::getTime(int32 &time){
	if(!Rtc.IsDateTimeValid())
		return 1;
	time=Rtc.GetDateTime().Epoch32Time();
	return 0;
}

CRTCWraper::~CRTCWraper() {
	// TODO Auto-generated destructor stub
}

