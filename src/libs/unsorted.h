/*
 *  Created on: Jan 21, 2016
 *      Author: minenko
 *
 * common not sorted function
 * https://github.com/amin7/libpack.git
 */
#ifndef _UNSORTED_H_
#define _UNSORTED_H_

#include <EEPROM.h>
#include <Stream.h>

void setup_wifi(const char *ssid, const char *password, const char *aHostname);
void wifi_loop();

void sw_info(char const *fwName,Stream &Out);
void hw_info(Stream &Out);
void wifiList(Stream &Out);

#endif
//eof
