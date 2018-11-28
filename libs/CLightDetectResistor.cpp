/*
 * CLightSensor.cpp
 *
 *  Created on: 11 ???. 2017 ?.
 *      Author: User
 */
#include <ESP8266WiFi.h>
#include "CLightDetectResistor.h"

CLightDetectResistor::CLightDetectResistor() {
	pinMode(A0, INPUT);
}

int CLightDetectResistor::get(){
    const unsigned long now = millis();
    if (now < nextRead) {
        return cacheVal;
    }
    nextRead = now + maxRefresh;
    cacheVal = analogRead(A0);
    return cacheVal;
}
CLightDetectResistor::~CLightDetectResistor() {
	// TODO Auto-generated destructor stub
}

