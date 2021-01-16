/*
 * CConfig.cpp
 *
 *  Created on: Jan 16, 2021
 *      Author: ominenko
 */

#include <libs/CConfig.h>
#include "logs.h"
#include <LittleFS.h>
#include <vector>
using namespace std;

CConfig config;

bool CConfig::setup() {
    CDBG_FUNK();
    auto cmdFile = LittleFS.open(JSON_FILE_CONFIG, "r");
    DeserializationError error = deserializeJson(json_config, cmdFile);
    if (error) {
        DBG_OUT << "Failed to read file, using default configuration" << endl;
        return false;
    }
    bool isOk = true;
    const auto keys = vector<string> { "DEVICE_NAME", "AP_PWD" };
    for (const auto &key : keys) {
        if (!json_config.containsKey(key.c_str())) {
            isOk = false;
            DBG_OUT << "miss keys:" << key << endl;
        }
    }
    if (!isOk) {
        return false;
    }

    DBG_OUT << "config " << json_config.capacity() << ":" << json_config.memoryUsage() << endl;
    return true;
}
void CConfig::setDefault() {
    json_config.clear();
    json_config["DEVICE_NAME"] = DEF_DEVICE_NAME;
    json_config["AP_PWD"] = DEF_AP_PWD;
    json_config["MQTT_SERVER"] = "";
    json_config["MQTT_PORT"] = 0;

    DBG_OUT << "default config " << json_config.capacity() << ":" << json_config.memoryUsage() << endl;
    serializeJsonPretty(json_config, Serial);
    DBG_OUT << endl;
}

const char* CConfig::getDeviceName() const {
    return json_config["DEVICE_NAME"].as<const char*>();
}
const char* CConfig::getAPPwd() const {
    return json_config["AP_PWD"].as<const char*>();
}
const char* CConfig::getMqttServer() const {
    return json_config["MQTT_SERVER"].as<const char*>();
}
const int CConfig::getMqttPort() const {
    return json_config["MQTT_PORT"].as<int>();
}
