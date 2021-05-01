/*
 * NTPtime.h
 *
 *  Created on: 4 ???. 2017 ?.
 *      Author: User
 */

#ifndef CLOCK_NTPTIME_H_
#define CLOCK_NTPTIME_H_
#include <ESP8266WiFi.h>
#include <WiFiUdp.h>

class NTPtime {
private:
    constexpr static uint32_t resendRequest = 10 * 1000l;

    constexpr static auto ntpServerName = "time.nist.gov";
    constexpr static unsigned int localPort = 2390; // local port to listen for UDP packets

    byte packetBuffer[48]; // buffer to hold incoming packets
    uint32_t nextRequest = 0;

    WiFiUDP udp;
    time_t parceAsEpoch();
    int sendNTPpacket(const IPAddress &address);
    public:
    void init();
    void loop();
};

#endif /* CLOCK_NTPTIME_H_ */
