/*
 * CDimableLed.cpp
 *
 *  Created on: Dec 6, 2017
 *      Author: ominenko
 */

#include "CDimableLed.h"
#include <iostream>
#include "./libs/logs.h"
#include "./libs/TimeLib.h"
#include "./libs/Cconfig.h"
using namespace std;

constexpr auto LED_VAL_OFF = 0;
constexpr auto LED_VAL_MAX = 1023;//PWMRANGE;

CIRSignal IRSignal;
CDimableLed dimableLed;
CWallSwitchSignal WallSwitchSignal;
CLedCmdSignal ledCmdSignal;

CIRSignal::CIRSignal():irrecv(GPIO_PIN_IRsensor, kCaptureBufferSize, kTimeout, false){};

bool CIRSignal::getExclusive(uint64_t &val, const uint32_t timeout){
	const auto wait = millis()+timeout;
	decode_results results;
	while(wait>millis()){
	  if (irrecv.decode(&results)) {  // We have captured something.
		  irrecv.resume();
		  cout<<"excl IR =" << std::hex<<results.value <<endl;
		  val=results.value;
		  return true;
	   }
	  yield();
	}
	return false;
}

void CIRSignal::begin(){
	irrecv.enableIRIn();  // Start up the IR receiver.
}

void CIRSignal::loop(){
	  decode_results results;
	  if (irrecv.decode(&results)) {  // We have captured something.
		  irrecv.resume();
		  cout<<"IR =" << std::hex<<results.value <<endl;
		  if(!results.repeat){
			  this->notify(results.value);
		  }
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
				cout<<"switch "<< preVal_<<endl;
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
CLedCmdSignal::CLedCmdSignal():ledValue(LED_VAL_OFF){
	cmd_list={
			{"set", bind(&CLedCmdSignal::set,this,placeholders::_1)},
			{"toggle", bind(&CLedCmdSignal::toggle,this,placeholders::_1)}
	};
}

void CLedCmdSignal::begin(){
	m_enabled=true;
	this->notify(ledValue);
}

void CLedCmdSignal::set(const int32_t val){
	ledValue=val;
	if(m_enabled){
		this->notify(ledValue);
	}
}

void CLedCmdSignal::toggle(const int32_t val){
	set(ledValue?LED_VAL_OFF:LED_VAL_MAX);
}


bool CLedCmdSignal::onCmd(const std::string &cmd,const int32_t val){
	cout<< "cmd="<<cmd<<", val="<<std::hex<<val<<endl;
	const auto it=cmd_list.find(cmd);
	if(it!=cmd_list.end()){
		it->second(val);
		return true;
	}
	return false;
}

void CLedCmdSignal::onIRcmd(const uint64_t &cmd){

}
void CLedCmdSignal::onWallcmd(const bool &state){
	onCmd("toggle",0);
}

/***
 *
 */

void CDimableLed::setup(){
	pinMode(GPIO_POUT_LED, OUTPUT);
	config.begin();

	ledCmdSignal.onSignal([](const uint16_t val){
		cout << "DIMABLE_LED_VAL=" << val  << endl;
		if(0==val){
			digitalWrite(GPIO_POUT_LED,0);
		}else if(LED_VAL_MAX<val){
			digitalWrite(GPIO_POUT_LED,1);
		}else{
			analogWrite(GPIO_POUT_LED, val);
		}
	});
	IRSignal.onSignal([](const uint64_t &cmd){
		ledCmdSignal.onIRcmd(cmd);
	});
	WallSwitchSignal.onSignal([](const bool &state){
		ledCmdSignal.onWallcmd(state);
	});
	IRSignal.begin();
	WallSwitchSignal.begin();
	ledCmdSignal.begin();
}

void CDimableLed::loop() {
	IRSignal.loop();
	WallSwitchSignal.loop();
}

