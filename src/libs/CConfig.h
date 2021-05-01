/*
 * CConfig.h
 *
 *  Created on: Jan 16, 2021
 *      Author: ominenko
 */

#pragma once
#include <sstream>
#include <ArduinoJson.h>
#include <LittleFS.h>
#include "logs.h"

template<size_t json_sz>
class CConfig {
protected:
    StaticJsonDocument<json_sz> m_config;
    public:
    JsonDocument& getConfig() {
        return m_config;
    }
    bool load(const char *_fileName) {
        DBG_FUNK();
        StaticJsonDocument<json_sz> tmp_conf;
        auto cmdFile = LittleFS.open(_fileName, "r");
        if (!cmdFile) {
            DBG_OUT << "Failed to open file" << std::endl;
            return false;
        }
        DeserializationError error = deserializeJson(tmp_conf, cmdFile);
        if (error) {
            DBG_OUT << "deserializeJson error=" << error.c_str() << std::endl;
            return false;
        }

        bool isOk = true;
        //checking
        for (const auto &kv : m_config.template as<JsonObject>()) {
            if (!tmp_conf.containsKey(kv.key())) {
                if (isOk) {
                    DBG_OUT << "config error file=" << _fileName << std::endl;
                }
                isOk = false;
                DBG_OUT << "miss keys:" << kv.key().c_str() << std::endl;
            }
        }
        //update
        for (const auto &kv : tmp_conf.template as<JsonObject>()) {
            if (m_config.containsKey(kv.key())) {
                m_config[kv.key()] = kv.value();
            }
        }

        serializeJsonPretty(m_config, Serial);
        DBG_OUT << std::endl;
        DBG_OUT << "config " << m_config.capacity() << ":" << m_config.memoryUsage() << std::endl;
        return isOk;
    }
    bool write(const char *_fileName) {
        auto configFile = LittleFS.open(_fileName, "w");

        if (!configFile) {
            DBG_OUT << "Failed to create file" << std::endl;
            return false;
        }

        // Serialize JSON to file
        if (serializeJson(m_config, configFile) == 0) {
            DBG_OUT << "Failed to write file" << std::endl;
        }

        configFile.close();
        return true;
    }

    const bool getBool(const char *key) const {
        return m_config[key].template as<bool>();
    }
    const int getInt(const char *key) const {
        return m_config[key].template as<int>();
    }
    const unsigned long getULong(const char *key) const {
        return m_config[key].template as<unsigned long>(); //in ms
    }
    const char* getCSTR(const char *key) const {
        return m_config[key].template as<const char*>();
    }
    virtual ~CConfig() {
    }
};

