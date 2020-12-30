/*
 * wifiHandle.h
 *
 *  30 dec-2020
 *      Author: ominenko
 */

#pragma once
#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <string>
#include <vector>

void wifiHandle_loop();
void wifiHandle_sendlist(ESP8266WebServer &server);
void wifiHandle_connect(ESP8266WebServer &server);
void setup_wifi(const String &ssid, const String &pwd, const String &host_name, const WiFiMode_t &wifi_mode);
