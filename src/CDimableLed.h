/*
 * CDimableLed.h
 *
 * on: 3 Jan , 2020
 *      Author: ominenko
 */

#pragma once
#include <Arduino.h>
#include <IRrecv.h>
#include <IRremoteESP8266.h>
#include <IRutils.h>
#include <IRtext.h>
#include "./libs/CSignal.h"

constexpr uint16_t kCaptureBufferSize = 1024;
constexpr uint8_t kTimeout = 15;  // Milli-Seconds
constexpr auto GPIO_POUT_LED = D1;
constexpr auto GPIO_PIN_WALL_SWITCH = D2;
constexpr auto GPIO_PIN_IRsensor = D3;
constexpr auto TIMEOUT_WALL_SWITCH = 100;

typedef enum {
	CMD_LED_OFF=0,
	CMD_LED_NIGHT,
	CMD_LED_NIGHT_HIGHT,
	CMD_LED_HIGHT,
  CMD_LED_TOGGLE_STATE
} led_cmd_t;


constexpr auto LED_VAL_OFF = 0;
constexpr auto LED_VAL_NIGHT = 2;
constexpr auto LED_VAL_NIGHT_HIGHT = (PWMRANGE / 4);
constexpr auto LED_VAL_HIGHT = PWMRANGE;

class CIRSignal:public SignalLoop<uint64_t>{
	IRrecv irrecv;
public:
	CIRSignal();
	void begin() override;
	void loop() override;
};


class CWallSwitchSignal:public SignalChange<bool>{
private:
	bool preVal_;
	uint32_t event_timeout;
	bool getValue();
	bool readRaw()const;
public:
	void begin() override;
};

class CLedCmdSignal:public Signal<int >{
	int ledValue;
public:
	void onCmd(const led_cmd_t &cmd);
	void onIRcmd(const uint64_t &cmd);
	void onWallcmd(const bool &state);
};

class CDimableLed {
public:
	void setup();
    void loop();
};

extern CLedCmdSignal ledCmdSignal;
extern CIRSignal IRSignal;
extern CWallSwitchSignal WallSwitchSignal;
extern CDimableLed dimableLed;

