/*
 * NTPtime.cpp
 *
 *  Created on: 4 ???. 2017 ?.
 *      Author: User
 */

#include "NTPtime.h"




NTPtime::NTPtime() {
}

void NTPtime::init(){
	 udp.begin(localPort);
         WiFi.hostByName(ntpServerName, timeServerIP);
}

int32 NTPtime::parceAsEpoch() {
  refreshed = millis();
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

int NTPtime::getTime(int32 &epoch) {
  sendNTPpacket();
  // wait to see if a reply is available
  int i = 100;
  do {
    if (0 == i) {
      return 1;
    }
    i--;
    delay(10);

  } while (0 == udp.parsePacket());
  // We've received a packet, read the data from it
  epoch = parceAsEpoch();
  return 0;
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

bool NTPtime::getTimeAsink(int32 &epoch) {
  const uint32 curms = millis();
#ifdef DUBUG_CLOCK_NTPTIME
  Serial.println();
  Serial.print(curms);
  Serial.print(" ");
  Serial.print(curms - refreshed);

#endif
  if ((refreshed + refreshPeriodDef) > curms) {
    return false; // time out is not passed
  }
#ifdef DUBUG_CLOCK_NTPTIME
  Serial.print(" ");
  Serial.print(curms - sendOn);
#endif
  if ((sendOn + sendDiscardPeriod) < curms) { // send or resend request
    sendNTPpacket();
    sendOn = millis();
    return false;
  }
// wait reply
#ifdef DUBUG_CLOCK_NTPTIME
  Serial.print("wait reply");
#endif
  if (0 == udp.parsePacket()) {
    return false;
  }
  sendOn = 0;
  refreshed = millis();
  epoch = parceAsEpoch();
  return true;
}

NTPtime::~NTPtime() {
	// TODO Auto-generated destructor stub
}

