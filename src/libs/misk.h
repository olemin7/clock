/*
 * misk.h
 *
 *  upd 30 dec-2020
 *      Author: ominenko
 */

#pragma once
#include <stdint.h>
#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <string>
#include <ostream>
#include "./libs/TimeLib.h"

std::ostream& operator<<(std::ostream &os, const String &str);
std::string timeStatus_toStr(const timeStatus_t&);

void LED_ON();
void LED_OFF();
void blink();

String getMimeType(String path);
void hw_info(std::ostream &out);
void LittleFS_info(std::ostream &out);

bool isExtMach(const std::string &name, const std::string &ext);

struct CompareCStrings
{
    bool operator()(const char *lhs, const char *rhs) const
            {
        return strcmp(lhs, rhs) < 0;
    }
};
std::string getResetInfo();
std::string to_string(uint32_t ul);

/*
 *  on start 0 during timeout 1
 */
bool isSafeMode(const uint8_t pin, unsigned long timeout);
