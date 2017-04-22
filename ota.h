/*
 * ota.h
 *
 *  Created on: 22 ????. 2017 ?.
 *      Author: User
 */

#ifndef CLOCK_OTA_H_
#define CLOCK_OTA_H_
#include <ESP8266mDNS.h>
#include <WiFiUdp.h>
#include <ArduinoOTA.h>

void ota_begin(const char *hostname,const char *pwd);
void ota_loop();

#endif /* CLOCK_OTA_H_ */
