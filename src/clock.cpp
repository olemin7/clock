// clang-format off
#include <NTPClient.h>
#include <Arduino.h>
#include <SPI.h>
#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <ESP8266mDNS.h>
#include <ESP8266HTTPUpdateServer.h>
#include <LittleFS.h>

#include <Wire.h> // must be included here so that Arduino library object file references work
#include <pgmspace.h>
#include <stdio.h>
#include <iostream>
#include <sstream>
#include <string>
#include <ctime>
#include <Max72xxPanel.h> // https://github.com/markruys/arduino-Max72xxPanel.git
#include <FreeMono9pt7b.h>

#include "DHTesp.h"

#include <CMQTT.h>

// clang-format on
#include <misk.h>
#include <wifiHandle.h>
#include <logs.h>
#include <CConfig.h>
#include "CLDRSignal.h"

#include <eventloop.h>

#include <WiFiUdp.h>
#include <Timezone.h>
#include <ESP8266TrueRandom.h>

#define DEBUG

using namespace std;
using namespace std::chrono_literals;

constexpr auto SERIAL_BAUND    = 115200;
constexpr auto SERVER_PORT_WEB = 80;

constexpr auto AP_MODE_TIMEOUT           = 30s; // switch to ap if no wifi
constexpr auto AUTO_REBOOT_AFTER_AP_MODE = 5min; // switch to ap if no wifi

constexpr auto pinCS                      = D6;
constexpr auto numberOfHorizontalDisplays = 4;
constexpr auto numberOfVerticalDisplays   = 1;

constexpr auto DHTPin = D4;

const char*    update_path = "/firmware";
constexpr auto DEF_AP_PWD  = "12345678";

auto matrix = Max72xxPanel(pinCS, numberOfHorizontalDisplays, numberOfVerticalDisplays);

CLDRSignal  LDRSignal;
const char* pDeviceName = nullptr;

DHTesp dht;

ESP8266WebServer        serverWeb(SERVER_PORT_WEB);
CMQTT                   mqtt;
ESP8266HTTPUpdateServer otaUpdater;
WiFiUDP                 ntpUDP;
NTPClient               ntp_client(ntpUDP);
auto                    config = CConfig<512>();
Timezone                myTZ((TimeChangeRule){ "DST", Last, Sun, Mar, 3, +3 * 60 },
                   (TimeChangeRule){ "STD", Last, Sun, Oct, 4, +2 * 60 });

void display_print(std::string str) {
    matrix.fillScreen(LOW);
    matrix.setCursor(0, 7);
    matrix.print(str.c_str());
    matrix.write();
    DBG_OUT << "display[" << str << "]" << endl;
}

void setup_matrix() {
    matrix.setIntensity(0); // Use a value between 0 and 15 for brightness
    const auto rotation = config.getInt("LED_MATRIX_ROTATION");
    if (rotation) {
        matrix.setRotation(0, rotation);
        matrix.setRotation(1, rotation);
        matrix.setRotation(2, rotation);
        matrix.setRotation(3, rotation);
    }
    matrix.setFont(&FreeMono9pt7b);
    matrix.setTextWrap(false);
    matrix.setTextColor(HIGH, LOW);
    matrix.setTextSize(1);
    display_print("init");
}

te_ret get_about(ostream& out) {
    out << "{";
    out << "\"firmware\":\"clock " << __DATE__ << " " << __TIME__ << "\"";
    out << ",\"deviceName\":\"" << pDeviceName << "\"";
    out << ",\"resetInfo\":" << system_get_rst_info()->reason;
    out << "}";
    return er_ok;
}

te_ret get_status(ostream& out) {
    out << "{";
    out << "\"temperature\":";
    toJson(out, dht.getTemperature());
    out << ",\"humidity\":";
    toJson(out, dht.getHumidity());
    out << ",\"ldr_raw\":" << LDRSignal.getLDR();
    out << ",\"ldr\":" << static_cast<unsigned>(LDRSignal.getLastValue());
    out << ",\"mqtt\":" << mqtt.isConnected();
    out << "}";
    return er_ok;
}

void setup_WebPages() {
    DBG_FUNK();
    otaUpdater.setup(&serverWeb, update_path, config.getCSTR("OTA_USERNAME"), config.getCSTR("OTA_PASSWORD"));

    serverWeb.on("/restart", []() {
        webRetResult(serverWeb, er_ok);
        delay(1000);
        ESP.restart();
    });

    serverWeb.on("/about", [] { wifiHandle_send_content_json(serverWeb, get_about); });

    serverWeb.on("/status", [] { wifiHandle_send_content_json(serverWeb, get_status); });

    serverWeb.on("/filesave", []() {
        DBG_FUNK();
        if (!serverWeb.hasArg("path") || !serverWeb.hasArg("payload")) {
            webRetResult(serverWeb, er_no_parameters);
            return;
        }
        const auto path = string("/www/") + serverWeb.arg("path").c_str();
        cout << path << endl;
        auto file = LittleFS.open(path.c_str(), "w");
        if (!file) {
            webRetResult(serverWeb, er_createFile);
            return;
        }
        if (!file.print(serverWeb.arg("payload"))) {
            webRetResult(serverWeb, er_FileIO);
            return;
        }
        file.close();
        webRetResult(serverWeb, er_ok);
    });

    serverWeb.on("/scanwifi", HTTP_ANY, [&]() { wifiHandle_sendlist(serverWeb); });
    serverWeb.on("/connectwifi", HTTP_ANY, [&]() { wifiHandle_connect(pDeviceName, serverWeb, true); });

    serverWeb.on("/getlogs", HTTP_ANY, [&]() {
        serverWeb.send(200, "text/plain", log_buffer.c_str());
        log_buffer = "";
    });

    serverWeb.serveStatic("/", LittleFS, "/www/");

    serverWeb.onNotFound([] {
        Serial.println("Error no handler");
        Serial.println(serverWeb.uri());
        webRetResult(serverWeb, er_fileNotFound);
    });
    serverWeb.begin();
}

void setup_WIFIConnect() {
    DBG_FUNK();
    WiFi.hostname(pDeviceName);
    WiFi.begin();

    auto to_ap_mode_thread = event_loop::set_timeout(
        []() {
            display_print("AP");
            setup_wifi("", DEF_AP_PWD, pDeviceName, WIFI_AP, false);

            event_loop::set_timeout(
                []() {
                    DBG_OUT << "Rebooting" << std::endl;
                    ESP.restart();
                },
                AUTO_REBOOT_AFTER_AP_MODE);
        },
        AP_MODE_TIMEOUT);
    static WiFiEventHandler onStationModeConnected = WiFi.onStationModeConnected([](auto event) {
        DBG_OUT << "WiFi conected, ssid =" << event.ssid << ", channel=" << static_cast<unsigned>(event.channel)
                << endl;
    });

    static WiFiEventHandler onStationModeGotIP = WiFi.onStationModeGotIP([to_ap_mode_thread](auto event) {
        to_ap_mode_thread->cancel();
        if (!ntp_client.isTimeSet()) { // do not show ip for network distraction
            char buffMin[6];
            sprintf_P(buffMin, ".%03u", event.ip[3]);
            display_print(buffMin);
        }
        DBG_OUT << "WiFi IP=" << event.ip << ", mask=" << event.mask << ", gw=" << event.gw << endl;
    });

    static WiFiEventHandler onStationModeDisconnected = WiFi.onStationModeDisconnected([](const auto event) {
        DBG_OUT << "WiFi Disconected, reason =" << static_cast<unsigned>(event.reason) << endl;
    });

    if (WIFI_STA == WiFi.getMode()) {
        DBG_OUT << "connecting <" << WiFi.SSID() << "> " << endl;
    }
}

void setup_signals() {
    DBG_FUNK();
    LDRSignal.onChange([](const uint8_t& level) { matrix.setIntensity(level); });
}

void setup_mqtt() {
    DBG_FUNK();
    mqtt.setup(config.getCSTR("MQTT_SERVER"), config.getInt("MQTT_PORT"), pDeviceName);
    const auto mqtt_period = config.getInt("MQTT_PERIOD");

    static auto mqtt_el = event_loop::set_interval(
        [mqtt_period]() {
            if (mqtt.isConnected()) {
                string topic = "stat/";
                topic += pDeviceName;
                StaticJsonDocument<512> payload;
                if (!isnan(dht.getTemperature())) {
                    payload["temperature"] = dht.getTemperature();
                }
                if (!isnan(dht.getHumidity())) {
                    payload["humidity"] = dht.getHumidity();
                }

                payload["ldr"]          = static_cast<unsigned>(LDRSignal.getLastValue());
                payload["wifi"]["rssi"] = WiFi.RSSI();
                payload["wifi"]["ip"]   = WiFi.localIP();
                payload["upd_period"]   = mqtt_period;
                String json_string;
                serializeJson(payload, json_string);
                mqtt.publish(topic, json_string.c_str());
            }
        },
        std::chrono::seconds(mqtt_period), 0s);

    const auto power_period = config.getInt("POWER_PERIOD");
    const auto power_jitter = ESP8266TrueRandom.random(power_period);
    event_loop::set_interval(
        [power_period]() {
            StaticJsonDocument<256> payload;
            String                  json_string;
            payload["name"]         = pDeviceName;
            payload["upd_period"]   = power_period;
            payload["wifi"]["rssi"] = WiFi.RSSI();
            payload["wifi"]["ip"]   = WiFi.localIP();
            serializeJson(payload, json_string);
            mqtt.publish("stat/power", json_string.c_str());
        },
        std::chrono::seconds(power_period), std::chrono::seconds(power_jitter));

    mqtt.on_connection_change([](const auto is_connected) {
        DBG_OUT << "setup_mqtt=" << is_connected << endl;

        if (is_connected) {
            mqtt_el->trigger();
        } else {
            event_loop::set_timeout([]() { mqtt.connect(); }, 30s);
        }
    });

    static WiFiEventHandler onStationModeGotIP = WiFi.onStationModeGotIP(
        [](auto event) { event_loop::set_timeout([]() { mqtt.connect(); }, 100ms); }); // need to lanch froom loop
}

void setup_config() {
    config.getConfig().clear();
    config.getConfig()["DEVICE_NAME"]         = "CLOCK";
    config.getConfig()["MQTT_SERVER"]         = "";
    config.getConfig()["MQTT_PORT"]           = 0;
    config.getConfig()["MQTT_PERIOD"]         = 60 * 5;
    config.getConfig()["POWER_PERIOD"]        = 15;
    config.getConfig()["OTA_USERNAME"]        = "";
    config.getConfig()["OTA_PASSWORD"]        = "";
    config.getConfig()["LED_MATRIX_ROTATION"] = 0;
    config.getConfig()["LED_MATRIX_I_MAX"]    = 15;
    config.getConfig()["LDR_MIN"]             = 0;
    config.getConfig()["LDR_MAX"]             = 1000;
    config.getConfig()["ntpServerName"]       = "time.nist.gov";
    if (!config.load("/www/config/config.json")) {
        // write file
        config.write("/www/config/config.json");
    }
    pDeviceName = config.getCSTR("DEVICE_NAME");
}

void show_time() {
    DBG_FUNK();
    const auto local = myTZ.toLocal(ntp_client.getEpochTime());
    char       buffMin[6];
    sprintf_P(buffMin, "%2u:%02u", hour(local), minute(local));
    display_print(buffMin);
    DBG_OUT << "utc time:" << ntp_client.getFormattedTime() << endl;
    event_loop::set_timeout(show_time, 60s - std::chrono::seconds(ntp_client.getSeconds()));
}

void setup() {
    Serial.begin(SERIAL_BAUND);
    logs_begin();
    DBG_FUNK();

    hw_info(DBG_OUT);
    LittleFS.begin();
    event_loop::init();
    setup_config();
    LDRSignal.setup();
    LDRSignal.setRange(config.getInt("LDR_MIN"), config.getInt("LDR_MAX"), 0, config.getInt("LED_MATRIX_I_MAX"));
    MDNS.addService("http", "tcp", SERVER_PORT_WEB);
    MDNS.begin(pDeviceName);
    setup_WebPages();
    setup_signals();

    LittleFS_info(DBG_OUT);
    setup_matrix();
    setup_mqtt();
    ntp_client.setPoolServerName(config.getCSTR("ntpServerName"));
    ntp_client.begin();
    //------------------
    dht.setup(DHTPin, DHTesp::DHT22);
    //-----------------
    display_print(pDeviceName);
    setup_WIFIConnect();
    DBG_OUT << "Setup done" << endl;
}

void loop() {
    LDRSignal.loop();
    serverWeb.handleClient();
    event_loop::loop();
    mqtt.loop();
    ntp_client.update();
    static bool wait_for_first_ntp = true;
    if (wait_for_first_ntp && ntp_client.isTimeSet()) {
        wait_for_first_ntp = false;
        show_time();
    }
}
