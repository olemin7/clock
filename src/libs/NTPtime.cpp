/*
 * NTPtime.cpp
 *
 *  Created on: 4 ???. 2017 ?.
 *      Author: User
 */

#include "NTPtime.h"
#include "time.h"
#include <TimeLib.h>
#include "logs.h"
using namespace std;

void NTPtime::init() {
    setSyncInterval(SECS_PER_DAY);
    udp.begin(localPort);
}

time_t NTPtime::parceAsEpoch() {
    udp.read(packetBuffer, sizeof(packetBuffer)); // read the packet into the buffer

    // this is NTP time (seconds since Jan 1 1900):
    unsigned long secsSince1900;
    // convert four bytes starting at location 40 to a long integer
    secsSince1900 = (unsigned long) packetBuffer[40] << 24;
    secsSince1900 |= (unsigned long) packetBuffer[41] << 16;
    secsSince1900 |= (unsigned long) packetBuffer[42] << 8;
    secsSince1900 |= (unsigned long) packetBuffer[43];

    constexpr unsigned long seventyYears = 2208988800UL;
    // subtract seventy years:
    return secsSince1900 - seventyYears;
}

void NTPtime::loop() {
    if (timeSet != timeStatus()) {
        if (WL_CONNECTED != WiFi.status()) {
            return;
        }
        while (udp.parsePacket()) {
            const auto value = parceAsEpoch();
            DBG_OUT << "ntp GMT" << hour(value) << ":" << minute(value) << ":" << second(value) << endl;
            setTime(value);
            nextRequest = 0;
            return;
        }

        const long now = millis();
        if (now > nextRequest) {
            IPAddress ntpServerIP;
            if (WiFi.hostByName(ntpServerName, ntpServerIP)) {
                sendNTPpacket(ntpServerIP);
            } else {
                DBG_OUT << "ntpServerIP err for " << ntpServerName << endl;
            }
            nextRequest = now + resendRequest;
        }
    }
}

int NTPtime::sendNTPpacket(const IPAddress &address) {
// all NTP fields have been given values, now
// you can send a packet requesting a timestamp:
// set all bytes in the buffer to 0
    memset(packetBuffer, 0, sizeof(packetBuffer));
// Initialize values needed to form NTP request
// (see URL above for details on the packets)
    packetBuffer[0] = 0b11100011;    // LI, Version, Mode
    packetBuffer[1] = 0;    // Stratum, or type of clock
    packetBuffer[2] = 6;    // Polling Interval
    packetBuffer[3] = 0xEC;    // Peer Clock Precision
// 8 bytes of zero for Root Delay & Root Dispersion
    packetBuffer[12] = 49;
    packetBuffer[13] = 0x4E;
    packetBuffer[14] = 49;
    packetBuffer[15] = 52;
    udp.beginPacket(address, 123);    // NTP requests are to port 123
    udp.write(packetBuffer, sizeof(packetBuffer));
    return udp.endPacket();
}

