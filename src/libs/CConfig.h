/*
 * CConfig.h
 *
 *  Created on: Jan 16, 2021
 *      Author: ominenko
 */

#pragma once
#include <sstream>
#include <ArduinoJson.h>

constexpr auto JSON_FILE_CONFIG = "/www/config/config.json";

#define DEF_DEVICE_NAME "CLOCK"
#define DEF_AP_PWD "12345678"

class CConfig {
    StaticJsonDocument<512> json_config;
    public:
    bool setup();
    void setDefault();
    const char* getDeviceName() const;
    const char* getMqttServer() const;
    const int getMqttPort() const;
    const unsigned long getMqttPeriod() const {
        return json_config["MQTT_PERIOD"].as<unsigned long>(); //in ms
    }
    const char* getOtaUsername() const;
    const char* getOtaPassword() const;
    const int getLedMattixRotation() const;
};
extern CConfig config;
