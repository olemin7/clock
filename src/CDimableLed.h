/*
 * CDimableLed.h
 *
 *  Created on: Dec 6, 2017
 *      Author: ominenko
 */

#ifndef CDIMABLELED_H_
#define CDIMABLELED_H_
#include <Arduino.h>
typedef enum {
	CMD_LED_OFF=0,
	CMD_LED_NIGHT,
	CMD_LED_NIGHT_HIGHT,
	CMD_LED_HIGHT,
  CMD_LED_TOGGLE_STATE
} led_cmd_t;

constexpr auto GPIO_POUT_LED = D1;
constexpr auto GPIO_PIN_WALL_SWITCH = D2;
constexpr auto GPIO_PIN_IRsensor = D3;

constexpr auto LED_OFF = 0;
constexpr auto LED_NIGHT = 2;
constexpr auto LED_NIGHT_HIGHT = (PWMRANGE / 4);
constexpr auto LED_HIGHT = PWMRANGE;

class CDimableLed {
public:
	void setup();
  void loop();
	virtual ~CDimableLed(){};
};

#endif /* CDIMABLELED_H_ */
