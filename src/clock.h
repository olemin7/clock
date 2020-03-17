/*
 * clock.h
 *
 *  Created on: 23 ????. 2017 ?.
 *      Author: User
 */

#ifndef CLOCK_CLOCK_H_
#define CLOCK_CLOCK_H_

#define ROOM 2

#if 1==ROOM
#define ROOM_NAME "Parent"
#define _USE_DIMABLE_LED_
#define LED_MATRIX_ROTATION 1
#define MQTT_ENABLE
#endif
#if 2==ROOM
#define ROOM_NAME "Children"
#define LED_MATRIX_ROTATION 3
#define MQTT_ENABLE
#endif


#include <SPI.h>
#include "./libs/Max72xxPanel.h" // https://github.com/markruys/arduino-Max72xxPanel.git
#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <ESP8266mDNS.h>
#include <ESP8266HTTPUpdateServer.h>

#include <Wire.h> // must be included here so that Arduino library object file references work
#include <pgmspace.h>

#include "./libs/NTPtime.h"
#include "FreeMono9pt7b.h"

#include "DHTesp.h"

//#include "libpack.h"
#include "./libs/unsorted.h"
#include "./libs/CLightDetectResistor.h"

#include "./libs/CMQTT.h"

#if 1
#include "secret.h_ex"
#else
#include "secret.h"
#endif

#ifdef _USE_DIMABLE_LED_
#include "CDimableLed.h"
#endif

#define MQTT_TEMPERATURE (1+(ROOM-1)*2)
#define MQTT_HUMIDITY (MQTT_TEMPERATURE+1)
#define DEVICE_NAME "CLOCK_" ROOM_NAME

//#define DEBUG


#endif /* CLOCK_CLOCK_H_ */
