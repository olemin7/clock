/*
 * CDimableLed.cpp
 *
 *  Created on: Dec 6, 2017
 *      Author: ominenko
 */

#include "CDimableLed.h"
#include "./libs/TimeLib.h"
#include <iostream>
#include "./libs/logs.h"
using namespace std;

CIRSignal IRSignal;
CDimableLed dimableLed;
CWallSwitchSignal WallSwitchSignal;
CLedCmdSignal ledCmdSignal;

CIRSignal::CIRSignal():irrecv(GPIO_PIN_IRsensor, kCaptureBufferSize, kTimeout, false){};
void CIRSignal::begin(){
	irrecv.enableIRIn();  // Start up the IR receiver.
}

void CIRSignal::loop(){
	  decode_results results;
	  if (irrecv.decode(&results)) {  // We have captured something.
		  if(!results.repeat){
			  this->notify(results.value);
		  }
	     irrecv.resume();
	   }
}
bool CWallSwitchSignal::readRaw()const{
	return digitalRead(GPIO_PIN_WALL_SWITCH);
}

bool CWallSwitchSignal::getValue(){
	 const auto cur = millis();
	 const auto value = readRaw();
	    if (value == preVal_) {
	      event_timeout = 0;
	    }else if (event_timeout){
	    	if (cur > event_timeout){
				preVal_=value; //debounce ok
				DBG_PRINTLN("wall new_VAL ");
	    	}
	    }else{
	    	event_timeout=cur+TIMEOUT_WALL_SWITCH;
	    }
	return preVal_;
}

void CWallSwitchSignal::begin(){
	pinMode(GPIO_PIN_WALL_SWITCH, INPUT_PULLUP);
	SignalChange<bool>::begin();
    preVal_ = false;
    event_timeout = 0;
}
/***
 *
 */
void CLedCmdSignal::onCmd(const led_cmd_t &cmd){
	DBG_PRINT("LED CMD ");
	DBG_PRINTLN(cmd);
	switch(cmd){
	case CMD_LED_OFF:
		ledValue=LED_VAL_OFF;
		break;
	case CMD_LED_NIGHT:
		ledValue=LED_VAL_NIGHT;
		break;
	case CMD_LED_NIGHT_HIGHT:
		ledValue=LED_VAL_NIGHT_HIGHT;
		break;
	case CMD_LED_HIGHT:
		ledValue=LED_VAL_HIGHT;
		break;
	case CMD_LED_TOGGLE_STATE:
		ledValue=(LED_VAL_OFF == ledValue) ? LED_VAL_HIGHT : LED_VAL_OFF;
		break;
	default:
		DBG_PRINTLN("LED no handler\n");
		return;
	}
	this->notify(ledValue);
}

void CLedCmdSignal::onIRcmd(const uint64_t &cmd){

}
void CLedCmdSignal::onWallcmd(const bool &state){
	onCmd(CMD_LED_TOGGLE_STATE);
}

class CLed_Handler {
	int ledValue=-1;
	void setLedValue(int Value) {
		ledValue = Value;
    analogWrite(GPIO_POUT_LED, Value);
    Serial.print("DIMABLE_LED_VAL ");
    Serial.println(Value);
	}
public:
  void setup() {
    pinMode(GPIO_POUT_LED, OUTPUT);
  }

};

/***
 *
 */

void CDimableLed::setup(){
	pinMode(GPIO_POUT_LED, OUTPUT);
	ledCmdSignal.onSignal([](const int val){
	    analogWrite(GPIO_POUT_LED, val);
	    DBG_PRINT("DIMABLE_LED_VAL ");
	    DBG_PRINTLN(val);
	});
	ledCmdSignal.onCmd(CMD_LED_OFF);
	IRSignal.onSignal([](const uint64_t &cmd){
		ledCmdSignal.onIRcmd(cmd);
	});
	WallSwitchSignal.onSignal([](const bool &state){
		ledCmdSignal.onWallcmd(state);
	});
	IRSignal.begin();
	WallSwitchSignal.begin();
}

void CDimableLed::loop() {
	IRSignal.loop();
	WallSwitchSignal.loop();
}

