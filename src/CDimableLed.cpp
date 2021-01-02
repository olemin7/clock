/*
 * CDimableLed.cpp
 *
 *  Created on: Dec 6, 2017
 *      Author: ominenko
 */
#if 0
#include "CDimableLed.h"
#include "./libs/TimeLib.h"

/***
 *
 */
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
  void cmd(led_cmd_t cmd) {
    Serial.print("LED CMD ");
    Serial.println(cmd);
		  switch(cmd){
      case CMD_LED_OFF:
        setLedValue(LED_OFF);
        break;
      case CMD_LED_NIGHT:
        setLedValue(LED_NIGHT);
        break;
      case CMD_LED_NIGHT_HIGHT:
        setLedValue(LED_NIGHT_HIGHT);
        break;
      case CMD_LED_HIGHT:
        setLedValue(LED_HIGHT);
        break;
      case CMD_LED_TOGGLE_STATE:
        setLedValue((LED_OFF == ledValue) ? LED_HIGHT : LED_OFF);
        break;
      default:
        Serial.printf("LED no handler\n");
		  }
	  }
};

/***
 * IR control
 */
IRrecv irrecv(GPIO_PIN_IRsensor);
class CIR_Control_handler {
  CLed_Handler &led_;
  public:
  void loop() {
		decode_results results;
    if (false == irrecv.decode(&results)) {
      return;
    }
    if (results.decode_type != NEC) {
      return;
    }
    Serial.printf("IR %lX\n", results.value);
		switch (results.value) {
      case IR_NEC_0:
        led_.cmd(CMD_LED_OFF);
        break;
      case IR_NEC_1:
        led_.cmd(CMD_LED_NIGHT);
        break;
      case IR_NEC_2:
        led_.cmd(CMD_LED_NIGHT_HIGHT);
        break;
      case IR_NEC_3:
        led_.cmd(CMD_LED_HIGHT);
        break;
		default:
        Serial.println("No handler");
    }
	}
  CIR_Control_handler(CLed_Handler &led) :
      led_(led) {
        irrecv.enableIRIn();
    }
};

/***
 *
 */


class CSwitch_Control_handler {
  CLed_Handler &led_;
  bool preVal_;
  bool read_switch_raw() {
    return digitalRead(GPIO_PIN_WALL_SWITCH);
  }
  uint32_t event_timeout;
	public:
  CSwitch_Control_handler(CLed_Handler &led) :
      led_(led) {
  }
  ;
		void setup(){
    pinMode(GPIO_PIN_WALL_SWITCH, INPUT_PULLUP);
    preVal_ = read_switch_raw();
    event_timeout = 0;
	}
  void loop() {
    const auto cur = millis();
    const auto value = read_switch_raw();
    if (value != preVal_) {
      event_timeout = cur + 100;
      preVal_ = value;
    }
    if (event_timeout) {
      if (cur > event_timeout) {
        event_timeout = 0;
        led_.cmd(CMD_LED_TOGGLE_STATE);
        Serial.print("CMD_LED_TOGGLE_STATE");
      }
  }
  }
};

CLed_Handler ledHandler;
CIR_Control_handler irControl_handler(ledHandler);
CSwitch_Control_handler switchControl_handler(ledHandler);

void CDimableLed::setup(){
  ledHandler.setup();
  switchControl_handler.setup();
}
void CDimableLed::loop() {
  irControl_handler.loop();
  switchControl_handler.loop();
}

#endif

