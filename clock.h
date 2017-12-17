/*
 * clock.h
 *
 *  Created on: 23 ????. 2017 ?.
 *      Author: User
 */

#ifndef CLOCK_CLOCK_H_
#define CLOCK_CLOCK_H_

#include <SPI.h>
#include <Adafruit_GFX.h> //https://github.com/adafruit/Adafruit-GFX-Library.git
#include <Max72xxPanel.h> // https://github.com/markruys/arduino-Max72xxPanel.git

#include <ESP8266WiFi.h>

#include <Wire.h> // must be included here so that Arduino library object file references work
#include <pgmspace.h>

#include "NTPtime.h"
#include "CDisplayClock.h"
#include "FreeMono9pt7b.h"
#include "DHT.h"

#include "libpack.h"
#include "unsorted.h"
#include "ota.h"
#include "CLightDetectResistor.h"

#include "CMQTT.h"

#include "secret.h"

#include "CFilter.h"
#include "CDimableLed.h"

#define ROOM 1

#if 1==ROOM
#define ROOM_NAME "Parent"
#endif
#if 2==ROOM
#define ROOM_NAME "Children"
#endif

#define MQTT_TEMPERATURE (1+(ROOM-1)*2)
#define MQTT_HUMIDITY (MQTT_TEMPERATURE+1)
#define DEVICE_NAME "CLOCK_" ROOM_NAME

#define DEBUG



#endif /* CLOCK_CLOCK_H_ */