/*
 * NTPtime.cpp
 *
 *  Created on: 4 ???. 2017 ?.
 *      Author: User
 */

#include "NTPtime.h"
#include "time.h"
#include "TimeLib.h"

void NTPtime::init(){
    udp.begin(localPort);
    initServerIP();
}
bool NTPtime::initServerIP() {
    if (WL_CONNECTED != WiFi.status()) {
        return false;
    }
    if (1 != WiFi.hostByName(ntpServerName, timeServerIP)) {
        Serial.print("Error.can't resolve IP ");
        Serial.println(ntpServerName);
        return false;
    }
    return true;
}

int32 NTPtime::parceAsEpoch() {
  udp.read(packetBuffer, NTP_PACKET_SIZE); // read the packet into the buffer

  // the timestamp starts at byte 40 of the received packet and is four bytes,
  // or two words, long. First, esxtract the two words:

  unsigned long highWord = word(packetBuffer[40], packetBuffer[41]);
  unsigned long lowWord = word(packetBuffer[42], packetBuffer[43]);
  // combine the four bytes (two words) into a long integer
  // this is NTP time (seconds since Jan 1 1900):
  unsigned long secsSince1900 = highWord << 16 | lowWord;
  const unsigned long seventyYears = 2208988800UL;
  // subtract seventy years:
  return secsSince1900 - seventyYears;
}
void NTPtime::loop() {
  const long now = millis();
  if (now < nextSynk) {
    return;
  }
  if (WL_CONNECTED != WiFi.status()) {
    return;
  }
  if (0 == timeoutIP) {
    timeoutIP = now + ipChangeTimeout;
  }

  if (INADDR_NONE == timeServerIP || now > timeoutIP) {
    nextRequest = 0;
    timeoutIP = 0;
    if (false == initServerIP()) {
      Serial.println("initServerIP setting err");
      nextSynk = now + errTimeout;
      return;
    }
  }

  if (now > nextRequest) {
    sendNTPpacket();
    nextRequest = now + resendRequest;
  }
  if (udp.parsePacket()) {
    const auto value = parceAsEpoch();
    Serial.printf("ntp GMT %02u:%02u:%02u done\n", hour(value), minute(value), second(value));
    if (nullptr != pCallback) {
      (pCallback)(value);
    }
    nextSynk = now + refreshPeriod;
    nextRequest = 0;
    timeoutIP = 0;
    return;
  }
}

int NTPtime::sendNTPpacket() {
  // all NTP fields have been given values, now
  // you can send a packet requesting a timestamp:
  // set all bytes in the buffer to 0
  memset(packetBuffer, 0, NTP_PACKET_SIZE);
  // Initialize values needed to form NTP request
  // (see URL above for details on the packets)
  packetBuffer[0] = 0b11100011;   // LI, Version, Mode
  packetBuffer[1] = 0;     // Stratum, or type of clock
  packetBuffer[2] = 6;     // Polling Interval
  packetBuffer[3] = 0xEC;  // Peer Clock Precision
  // 8 bytes of zero for Root Delay & Root Dispersion
  packetBuffer[12]  = 49;
  packetBuffer[13]  = 0x4E;
  packetBuffer[14]  = 49;
  packetBuffer[15]  = 52;
  udp.beginPacket(timeServerIP, 123); // NTP requests are to port 123
  udp.write(packetBuffer, NTP_PACKET_SIZE);
  return udp.endPacket();
}

