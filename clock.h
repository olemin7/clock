/*
 * clock.h
 *
 *  Created on: 23 ????. 2017 ?.
 *      Author: User
 */

#ifndef CLOCK_CLOCK_H_
#define CLOCK_CLOCK_H_

#include <SPI.h>
#include <Adafruit_GFX.h>
#include <Max72xxPanel.h>

#include <ESP8266WiFi.h>
#include <WiFiUdp.h>
#include <WiFiClient.h>

#include <TimeLib.h>

#include <Wire.h> // must be included here so that Arduino library object file references work
#include <RtcDS3231.h>
#include <pgmspace.h>
#include <time.h>
#include <PubSubClient.h>

#include "NTPtime.h"
#include "CLightDetectResistor.h"
#include "CDisplayClock.h"
#include "CIntensity.h"
#include "FreeMono9pt7b.h"
#include "DHT.h"

#include "libpack.h"
#include "ota.h"

#include "secret.h"
//#ifndef SECRET_H_
//#define SECRET_H_
//const char* ssid = "";
//const char* password = "@ea";
//const char* mqtt_server = "";
//const char* Write_API_Key="";
//const int mqtt_port=;
//const int channelID = ;
//const char* update_username = "";
//const char* update_password = "";
//#endif

#define DEBUG



#endif /* CLOCK_CLOCK_H_ */
