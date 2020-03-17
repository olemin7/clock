/*
 * NTPtime.h
 *
 *  Created on: 4 ???. 2017 ?.
 *      Author: User
 */

#ifndef CLOCK_NTPTIME_H_
#define CLOCK_NTPTIME_H_
//#define DUBUG_CLOCK_NTPTIME
#include <sys/types.h> // for __time_t_defined, but avr libc lacks sys/types.h

#include <functional>

#include <ESP8266WiFi.h>
#include <WiFiUdp.h>

class NTPtime {
private:
  std::function<void(const time_t&)> pCallback;
  const uint32_t refreshPeriod = 24 * 60 * 60 * 1000;          // one 24 hour
  const uint32_t resendRequest = 1 * 1000l;     // 5 sec
  const uint32_t ipChangeTimeout = 60 * 1000;     //
  const uint32_t errTimeout = 10 * 60 * 1000;
  uint32_t nextSynk = 0;
  uint32_t nextRequest = 0;
  uint32_t timeoutIP = 0;

  IPAddress timeServerIP; // time.nist.gov NTP server address
  const char *ntpServerName = "time.nist.gov";

  const static int NTP_PACKET_SIZE = 48; // NTP time stamp is in the first 48 bytes of the message

  byte packetBuffer[NTP_PACKET_SIZE]; // buffer to hold incoming packets

  const static unsigned int localPort = 2390; // local port to listen for UDP packets
  WiFiUDP udp;
  int32 parceAsEpoch();
  int sendNTPpacket();
  bool initServerIP();
public:
  void init();
  void loop();
  void setCallback(std::function<void(const time_t&)> pF) {
    pCallback = pF;
  }
};

#endif /* CLOCK_NTPTIME_H_ */
