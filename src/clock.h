/*
 * clock.h
 *
 *  Created on: 23 ????. 2017 ?.
 *      Author: User
 */

#ifndef CLOCK_CLOCK_H_
#define CLOCK_CLOCK_H_

#include <Arduino.h>
#include <SPI.h>
#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <ESP8266mDNS.h>
#include <ESP8266HTTPUpdateServer.h>
#include <LittleFS.h>

#include <Wire.h> // must be included here so that Arduino library object file references work
#include <pgmspace.h>
#include <stdio.h>
#include <iostream>
#include <sstream>
#include <string>
#include <ctime>

#include <NTPtime.h>
#include <Max72xxPanel.h> // https://github.com/markruys/arduino-Max72xxPanel.git
#include "FreeMono9pt7b.h"

#include "DHTesp.h"

#include <CMQTT.h>
#include <misk.h>
#include <wifiHandle.h>
#include <TimeLib.h>
#include <Timezone.h>
#include <logs.h>
#include <CConfig.h>
#include "CLDRSignal.h"

#include "CDimableLed.h"

#define DEBUG

#define SERIAL_BAUND 115200
#define SERVER_PORT_WEB 80
#define WIFI_CONNECT_TIMEOUT 20000 //ms
#endif /* CLOCK_CLOCK_H_ */
