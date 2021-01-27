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
#include "./libs/misk.h"

#define ARDUINOJSON_USE_LONG_LONG 1
#include <ArduinoJson.h>
#include <LittleFS.h>
using namespace std;

constexpr auto LED_VAL_MIN = 0;
constexpr auto LED_VAL_MAX = 1023; //PWMRANGE;

CIRSignal IRSignal;
CDimableLed dimableLed;
CWallSwitchSignal WallSwitchSignal;
CLedCmdSignal ledCmdSignal;

CIRSignal::CIRSignal() :
        irrecv(GPIO_PIN_IRsensor, kCaptureBufferSize, kTimeout, false) {
}
;

bool CIRSignal::getExclusive(uint64_t &val, const uint32_t timeout, std::function<void(void)> blink) {
    CDBG_FUNK();
    const auto wait = millis() + timeout;
    decode_results results;
    unsigned long blink_timeout = 0;
    while (1) {
        if (irrecv.decode(&results)) {  // We have captured something.
            irrecv.resume();
            DBG_OUT << "excl IR =" << std::hex << results.value << std::dec << endl;
            val = results.value;
            return true;
        }
        const auto cur = millis();
        if (wait < cur)
            break;
        if (blink_timeout < cur) {
            blink_timeout = cur + 300;
            blink();
        }
        yield();
    }
    return false;
}

void CIRSignal::begin() {
    irrecv.enableIRIn();  // Start up the IR receiver.
}

void CIRSignal::loop() {
    decode_results results;
    if (irrecv.decode(&results)) {  // We have captured something.
        irrecv.resume();
        cout << "IR =" << std::hex << results.value << endl;
        if (!results.repeat) {
            this->notify(results.value);
        }
    }
}
bool CWallSwitchSignal::readRaw() const {
    return digitalRead(GPIO_PIN_WALL_SWITCH);
}

bool CWallSwitchSignal::getValue() {
    const auto cur = millis();
    const auto value = readRaw();
    if (value == preVal_) {
        event_timeout = 0;
    } else if (event_timeout) {
        if (cur > event_timeout) {
            preVal_ = value; //debounce ok
            cout << "switch " << preVal_ << endl;
        }
    } else {
        event_timeout = cur + TIMEOUT_WALL_SWITCH;
    }
    return preVal_;
}

void CWallSwitchSignal::begin() {
    CDBG_FUNK();
    pinMode(GPIO_PIN_WALL_SWITCH, INPUT_PULLUP);
    SignalChange<bool>::begin();
    preVal_ = false;
    event_timeout = 0;
}
/***
 *
 */
CLedCmdSignal::CLedCmdSignal() :
        m_ledValue(0) {
    m_cmd_list = {
            { "set", bind(&CLedCmdSignal::set, this, placeholders::_1) },
            { "toggle", bind(&CLedCmdSignal::toggle, this, placeholders::_1) }
    };
}

JsonArrayConst::iterator json_get_cmd(JsonArrayConst list, const string &cmd) {
    if (list)
        for (auto it = list.begin(); it != list.end(); ++it) {
            if (it->containsKey("cmd"))
                if (0 == cmd.compare((*it)["cmd"].as<const char*>())) {
                    return it;
                }
        }
    return list.end();
}

std::map<uint64_t, pair<string, int32_t>> json_get_ir_cmd_map() {
    std::map<uint64_t, pair<string, int32_t>> data;
    do {
        auto cmdFile = LittleFS.open(JSON_FILE_PRESET_CMD, "r");
        StaticJsonDocument<512> preset_cmd;
        DeserializationError error = deserializeJson(preset_cmd, cmdFile);
        DBG_OUT << "preset_cmd " << preset_cmd.capacity() << ":" << preset_cmd.memoryUsage() << endl;
        if (error) {
            Serial.println(F("Failed to read file, using default configuration"));
            break;
        }
        auto cmd_items = preset_cmd["items"].as<JsonArrayConst>();
        if (!cmd_items) {
            Serial.println("cmd_item");
            break;
        }

        auto rcFile = LittleFS.open(JSON_FILE_PRESET_RC, "r");
        StaticJsonDocument<1024> preset_rc;
        error = deserializeJson(preset_rc, rcFile);
        if (error) {
            Serial.println(F("Failed to read file, using default configuration"));
            break;
        }
        DBG_OUT << "preset_rc " << preset_rc.capacity() << ":" << preset_rc.memoryUsage() << endl;
        for (const auto &it : preset_rc["items"].as<JsonArray>()) {
            auto cmd_index = json_get_cmd(cmd_items, it["cmd"].as<const char*>());
            if (cmd_items.end() != cmd_index) {

                data[it["code"].as<uint64_t>()] = pair<string, int32_t>(
                        (*cmd_index)["handler"].as<char*>(),
                        (*cmd_index)["val"].as<int32_t>()
                        );
            }

        }

    } while (0);

    for (const auto &it : data) {
        DBG_OUT << it.first << " " << it.second.first << " " << it.second.second << endl;
    }
    return data;
}

void CLedCmdSignal::begin() {
    CDBG_FUNK();
    m_ir_cmd = std::move(json_get_ir_cmd_map());
    m_enabled = true;
    set(0);
}

void CLedCmdSignal::set(const int32_t val) {
    CDBG_FUNK();
    DBG_OUT << "val=" << val << endl;
    m_ledValue = (100 > val) ? val : 100;
    //todo move to led
    const auto duty = (10 > m_ledValue) ? val : (LED_VAL_MIN + m_ledValue * (LED_VAL_MAX - LED_VAL_MIN) / 100);
    if (m_enabled) {
        this->notify(duty);
    }
}

void CLedCmdSignal::toggle(const int32_t val) {
    set(m_ledValue ? 0 : 100);
}

bool CLedCmdSignal::onCmd(const std::string &cmd, const int32_t val) {
    DBG_OUT << "cmd=" << cmd << ", val=" << std::hex << val << endl;
    const auto it = m_cmd_list.find(cmd);
    if (it != m_cmd_list.end()) {
        it->second(val);
        return true;
    }
    return false;
}

void CLedCmdSignal::onIRcmd(const uint64_t &cmd) {
    DBG_OUT << "onIRcmd" << endl;
    const auto &it = m_ir_cmd.find(cmd);
    if (it != m_ir_cmd.end()) {
        onCmd(it->second.first, it->second.second);
    }

}
void CLedCmdSignal::onWallcmd(const bool &state) {
    DBG_OUT << "onWallcmd:" << state << endl;
    onCmd("toggle", 0);
}

/***
 *
 */

void CDimableLed::setup() {
    CDBG_FUNK();
    pinMode(GPIO_POUT_LED, OUTPUT);

    ledCmdSignal.onSignal([](const uint16_t val) {
        DBG_OUT << "DIMABLE_LED_VAL=" << val << endl;
        if (0 == val) {
            digitalWrite(GPIO_POUT_LED, 0);
        } else if (LED_VAL_MAX <= val) {
            digitalWrite(GPIO_POUT_LED, 1);
        } else {
            analogWrite(GPIO_POUT_LED, val);
        }
    });
    IRSignal.onSignal([](const uint64_t &cmd) {
        ledCmdSignal.onIRcmd(cmd);
    });
    WallSwitchSignal.onSignal([](const bool &state) {
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

