 /***************************************************
 * IRremote for ESP8266
 *
 * Based on the IRremote library for Arduino by Ken Shirriff 
 * Version 0.11 August, 2009
 * Copyright 2009 Ken Shirriff
 * For details, see http://arcfn.com/2009/08/multi-protocol-infrared-remote-library.html
 *
 * Modified by Paul Stoffregen <paul@pjrc.com> to support other boards and timers
 *
 * Interrupt code based on NECIRrcv by Joe Knapp
 * http://www.arduino.cc/cgi-bin/yabb2/YaBB.pl?num=1210243556
 * Also influenced by http://zovirl.com/2008/11/12/building-a-universal-remote-with-an-arduino/
 *
 * JVC and Panasonic protocol added by Kristian Lauszus (Thanks to zenwheel and other people at the original blog post)
 * Whynter A/C ARC-110WD added by Francesco Meschia
 *
 * 09/23/2015 : Samsung pulse parameters updated by Sebastien Warin to be compatible with EUxxD6200 
 *
 *  GPL license, all text above must be included in any redistribution
 ****************************************************/

#ifndef IRremoteint_h
#define IRremoteint_h

#if defined(ARDUINO) && ARDUINO >= 100
#include <Arduino.h>
#else
#include <WProgram.h>
#endif


#define NEC_HDR_MARK	9000
#define NEC_HDR_SPACE	4500
#define NEC_BIT_MARK	560
#define NEC_ONE_SPACE	1690
#define NEC_ZERO_SPACE	560
#define NEC_RPT_SPACE	2250

#define NEC_RPT_TIMEOUT	108000
#define NEC_MSG_LENGTH  67500

#define TOLERANCE 35  // percent tolerance in measurements
#define LTOL (1.0 - TOLERANCE/100.) 
#define UTOL (1.0 + TOLERANCE/100.) 

#define _GAP 5000 // Minimum map between transmissions
#define GAP_TICKS (_GAP/USECPERTICK)

#define TICKS_LOW(us) (int) (((us)*LTOL/USECPERTICK))
#define TICKS_HIGH(us) (int) (((us)*UTOL/USECPERTICK + 1))

// receiver states
enum{
 STATE_IDLE=     2,
 STATE_MARK     ,
 STATE_SPACE    ,
 STATE_DATA_MARK     ,
 STATE_DATA     ,
 STATE_REPEAT   ,
 STATE_END_OF_MSG
};

// information for the interrupt handler
typedef struct {
  uint8_t recvpin;           // pin for IR data from detector
  uint8_t rcvstate;          // state machine

  uint32_t PrevValue;
  uint32_t Value;
  uint32_t buffer;
  uint8_t  BitExpected;//32 default
  bool 	   isRepeat;
  bool 	   isValueReady;
  uint32_t Rpt_TimeOut;// time out on repeat current command

} 
irparams_t;

// Defined in IRremote.cpp
extern volatile irparams_t irparams;

#define TOPBIT 0x80000000

#define NEC_BITS 32

#endif
