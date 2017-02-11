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
	return analogRead(A0);
}
CLightDetectResistor::~CLightDetectResistor() {
	// TODO Auto-generated destructor stub
}

