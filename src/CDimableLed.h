/*
 * CDimableLed.h
 *
 * on: 3 Jan , 2020
 *      Author: ominenko
 */

#pragma once
#include <Arduino.h>
#include <stdint.h>
#include <IRrecv.h>
#include <IRremoteESP8266.h>
#include <IRutils.h>
#include <IRtext.h>
#include <string>
#include <functional>
#include <map>
#include <tuple>
#include "./libs/CSignal.h"

constexpr uint16_t kCaptureBufferSize = 1024;
constexpr uint8_t kTimeout = 15;  // Milli-Seconds
constexpr auto GPIO_POUT_LED = D1;
constexpr auto GPIO_PIN_WALL_SWITCH = D2;
constexpr auto GPIO_PIN_IRsensor = D3;
constexpr auto TIMEOUT_WALL_SWITCH = 100;

constexpr auto JSON_FILE_PRESET_CMD = "/www/config/preset_cmd.json";
constexpr auto JSON_FILE_PRESET_RC = "/www/config/preset_rc.json";

class CIRSignal: public SignalLoop<uint64_t> {
    IRrecv irrecv;
    public:
    CIRSignal();
    bool getExclusive(uint64_t &val, const uint32_t timeout, std::function<void(void)> blink);
    void begin() override;
    void loop() override;
};

class CWallSwitchSignal: public SignalChange<bool> {
private:
    bool preVal_;
    uint32_t event_timeout;
    bool getValue();
    bool readRaw() const;
    public:
    void begin() override;
};

/*
 *
 */

class CLedCmdSignal: public Signal<uint8_t> {
    uint8_t m_ledValue;
    std::map<std::string, std::function<void(const int32_t)> > m_cmd_list;
    std::map<uint64_t, pair<string, int32_t>> m_ir_cmd;
    bool m_enabled = false;
    public:
    CLedCmdSignal();
    uint8_t getVal() const {
        return m_ledValue;
    }
    void set(const int32_t val);
    void toggle(const int32_t val);
    bool onCmd(const std::string &cmd, const int32_t val);
    void onIRcmd(const uint64_t &cmd);
    void onWallcmd(const bool &state);
    void loop();
    void begin();
};

class CDimableLed {
    bool m_hasIR;
    bool m_hasWallSwitch;
    public:
    void setup(bool hasIR = true, bool hasWallSwitch = true);
    void loop();
};

extern CLedCmdSignal ledCmdSignal;
extern CIRSignal IRSignal;
extern CWallSwitchSignal WallSwitchSignal;
extern CDimableLed dimableLed;

