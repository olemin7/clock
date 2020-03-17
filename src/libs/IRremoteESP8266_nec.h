/*
 * dimable_led1.h
 *
 *  Created on: Jan 21, 2016
 *      Author: minenko
 *
 lib used

 * https://github.com/markszabo/IRremoteESP8266
https://github.com/markszabo/IRremoteESP8266.git
 need update IRremoteInt.h
 #define TOLERANCE 35  // percent tolerance in measurements

 docs https://techdocs.altium.com/display/FPGA/NEC+Infrared+Transmission+Protocol
 */

#ifndef _IRremote_ESP8266_NEC_H
#define _IRremote_ESP8266_NEC_H

// Results returned from the decoder
class decode_results {
public:
  int decode_type; // NEC, SONY, RC5, UNKNOWN
  union { // This is used for decoding Panasonic and Sharp data
    unsigned int panasonicAddress;
    unsigned int sharpAddress;
  };
  unsigned long value; // Decoded value
  int bits; // Number of bits in decoded value
  volatile unsigned int *rawbuf; // Raw intervals in .5 us ticks
  int rawlen; // Number of records in rawbuf.
};

// Values for decode_type
#define NEC 1


// Decoded value for NEC when a repeat code is received
#define REPEAT 0xffffffff


#define IR_NEC_0 0x0FDB04F
#define IR_NEC_1 0x0FD00FF
#define IR_NEC_2 0x0FD807F
#define IR_NEC_3 0x0FD40BF
#define IR_NEC_4 0x0FD20DF
#define IR_NEC_5 0x0FDA05F
#define IR_NEC_6 0x0FD609F
#define IR_NEC_7 0x0FD10EF
#define IR_NEC_8 0x0FD906F
#define IR_NEC_9 0x0FD50AF
#define IR_NEC_STAR   0x0FD30CF
#define IR_NEC_DRASH  0x0FD708F
#define IR_NEC_Up     0x0FD8877
#define IR_NEC_Down   0x0FD9867
#define IR_NEC_Left   0x0FD28D7
#define IR_NEC_Right  0x0FD6897
#define IR_NEC_Ok     0x0FDA857

// main class for receiving IR
class IRrecv
{
public:
  IRrecv(int recvpin);
  bool decode(decode_results *results);
  void enableIRIn();
  void disableIRIn();
  void resume();
  private:
  int compare(unsigned int oldval, unsigned int newval);
};

char IRdecode_NEC2char(unsigned long value);//return 0 in not mach
// Some useful constants
#define USECPERTICK 50  // microseconds per clock interrupt tick

// Marks tend to be 100us too long, and spaces 100us too short
// when received due to sensor lag.
#define MARK_EXCESS 0//100

#endif
