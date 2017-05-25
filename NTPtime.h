/*
 * NTPtime.h
 *
 *  Created on: 4 ???. 2017 ?.
 *      Author: User
 */

#ifndef CLOCK_NTPTIME_H_
#define CLOCK_NTPTIME_H_
//#define DUBUG_CLOCK_NTPTIME

#include <ESP8266WiFi.h>
#include <WiFiUdp.h>
#include <timeLib.h>
typedef void (*tNTPtimeSync)(time_t);

class NTPtime {
private:
    unsigned long refreshPeriod = 60 * 60 * 1000;          // one hour
    unsigned long refreshTime = 0;
    const static long reRefreshPeriod = 10 * 1000;     // 10 sec

    IPAddress timeServerIP; // time.nist.gov NTP server address
    const char *ntpServerName = "time.nist.gov";
    tNTPtimeSync pSyncFunc = NULL;

    const static int NTP_PACKET_SIZE = 48; // NTP time stamp is in the first 48 bytes of the message

  byte packetBuffer[NTP_PACKET_SIZE]; // buffer to hold incoming packets

  const static unsigned int localPort =
      2390; // local port to listen for UDP packets
  WiFiUDP udp;
  int32 parceAsEpoch();
  int sendNTPpacket();

public:
    NTPtime();
    void init();
    time_t getTime();
    void setSyncInterval(unsigned long interval) {
        refreshPeriod = interval;
        refreshTime = 0;
    }
    void setSyncFunc(tNTPtimeSync func) {
        pSyncFunc = func;
    }
    void loop();
};

#endif /* CLOCK_NTPTIME_H_ */
