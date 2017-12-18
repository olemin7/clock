/*
 * CDimableLed.cpp
 *
 *  Created on: Dec 6, 2017
 *      Author: ominenko
 */

#include "CDimableLed.h"
#include "TimeLib.h"

/***
 *
 */
class CLed_Handler:public ObserverWrite<uint8_t>{
	int ledValue=-1;
	void setLedValue(int Value) {
		ledValue = Value;
	    analogWrite(GPIO_POUT_LED, Value);
	    Serial.printf("DIMABLE_LED_VAL %d\n", Value);
	}
	public:
	  void writeValue(uint8_t cmd){
		  Serial.printf("LED CMD %d\n", cmd);
		  switch(cmd){
		  	  case CMD_LED_OFF: 		setLedValue(LED_OFF);break;
		  	  case CMD_LED_NIGHT:		setLedValue(LED_NIGHT);break;
		  	  case CMD_LED_NIGHT_HIGHT:	setLedValue(LED_NIGHT_HIGHT);break;
		  	  case CMD_LED_HIGHT:		setLedValue(LED_HIGHT);break;
		  	  case CMD_LED_SWICH_STATE: setLedValue((LED_OFF == ledValue) ? LED_HIGHT : LED_OFF);  break;
		  	  default:
		  		Serial.printf("LED no handler\n");
		  }
	  }
};

/***
 * IR control
 */
IRrecv irrecv(GPIO_PIN_IRsensor);
class CIR_Control_handler:public CSubjectPeriodic<uint8_t>{
	uint32_t getTimeInMs(){return 0;}
	bool readValue(uint8_t &key){
		decode_results results;
		if (false == irrecv.decode(&results))
			return false;
	    if (results.decode_type != NEC)
	        return false;
	    Serial.printf("IR %lX\n", results.value);
		switch (results.value) {
		case IR_NEC_0:	key=CMD_LED_OFF;		  break;
		case IR_NEC_1:	key=CMD_LED_NIGHT;		  break;
		case IR_NEC_2:	key=CMD_LED_NIGHT_HIGHT;  break;
		case IR_NEC_3:	key=CMD_LED_HIGHT;		  break;
		default:
		  Serial.println("No handler");
		  return false;
		}
		return true;
	}
public:
    CIR_Control_handler(){
        irrecv.enableIRIn();
    }
};

/***
 *
 */


class CSwitch_Control_handler:public Subject<uint8_t>,public ObserverWrite<bool>{
	class CSource:public CSubjectPeriodic<bool> {
		uint32_t getTimeInMs(){return 0;}
		bool readValue(bool &value){
			value=digitalRead(GPIO_PIN_WALL_SWITCH);
			return true;
		}
	};
	class CDebounce:public CFilter_Debounce<bool>{
		uint32_t getTimeInMs(){return millis();}
	public:
		CDebounce():CFilter_Debounce(50){};
	};
	CSource source;
	CDebounce debounce;
	CFilter_OnChange<bool> onChange;
	void writeValue(bool value){
		setValue(CMD_LED_SWICH_STATE);
		Serial.print("CMD_LED_SWICH_STATE val=");
		Serial.println(value);
	}
	public:
		void setup(){
			source.addListener(debounce);
			debounce.addListener(onChange);
			debounce.waitSetting();
			onChange.addListener(*this);
	}
};

CLed_Handler ledHandler;
CIR_Control_handler irControl_handler;
CSwitch_Control_handler switchControl_handler;

void CDimableLed::setup(){
    switchControl_handler.setup();
    irControl_handler.addListener(ledHandler);
    switchControl_handler.addListener(ledHandler);
}



