#include "clock.h"
using namespace std;

constexpr auto pinCS = D6;
constexpr auto numberOfHorizontalDisplays = 4;
constexpr auto numberOfVerticalDisplays = 1;

constexpr auto DHTPin = D4;

const char *update_path = "/firmware";
bool is_safe_mode = false;
constexpr auto DEF_AP_PWD = "12345678";

Timezone myTZ((TimeChangeRule ) { "DST", Last, Sun, Mar, 3, +3 * 60 },
        (TimeChangeRule ) { "STD", Last, Sun, Oct, 4, +2 * 60 });

auto matrix = Max72xxPanel(pinCS, numberOfHorizontalDisplays, numberOfVerticalDisplays);

NTPtime ntpTime;
void mqtt_send();

CLDRSignal LDRSignal;
const char *pDeviceName = nullptr;

class CTimeKeeper: public SignalChange<time_t> {
    time_t getValue() {
        return myTZ.toLocal(now());
    }
} timeKeeper;

DHTesp dht;

ESP8266WebServer serverWeb(SERVER_PORT_WEB);
CMQTT mqtt;
ESP8266HTTPUpdateServer otaUpdater;
CWifiStateSignal wifiStateSignal;
auto config = CConfig<512>();

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
    matrix.fillScreen(LOW);
    matrix.setTextColor(HIGH, LOW);
    matrix.setTextSize(1);
    matrix.setCursor(0, 7);
    matrix.print("init");
    matrix.write();
}

te_ret get_about(ostream &out) {
    out << "{";
    out << "\"firmware\":\"clock " << __DATE__ << " " << __TIME__ << "\"";
    out << ",\"deviceName\":\"" << pDeviceName << "\"";
    out << ",\"resetInfo\":" << system_get_rst_info()->reason;
    out << "}";
    return er_ok;
}

te_ret get_status(ostream &out) {
    const auto local = timeKeeper.getSavedValue();
    out << "{\"timeStatus\":" << timeStatus();
    out << ",\"time\":\"";
    toTime(out, local);
    out << " ";
    toDate(out, local);
    out << "\"";
    out << ",\"temperature\":";
    toJson(out, dht.getTemperature());
    out << ",\"humidity\":";
    toJson(out, dht.getHumidity());
    out << ",\"ldr_raw\":" << LDRSignal.get();
    out << ",\"ldr\":" << LDRSignal.getSavedValue();
    out << ",\"led\":" << static_cast<unsigned>(ledCmdSignal.getVal());
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

    serverWeb.on("/about", [] {
        wifiHandle_send_content_json(serverWeb, get_about);
    });

    serverWeb.on("/status", [] {
        wifiHandle_send_content_json(serverWeb, get_status);
    });

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

    serverWeb.on("/scanwifi", HTTP_ANY,
            [&]() {
                wifiHandle_sendlist(serverWeb);
            });
    serverWeb.on("/connectwifi", HTTP_ANY,
            [&]() {
                wifiHandle_connect(serverWeb);
            });
    serverWeb.on("/command", [&]() {
        if (!serverWeb.hasArg("handler")) {
            webRetResult(serverWeb, er_no_parameters);
            return;
        }
        const auto handler = serverWeb.arg("handler").c_str();
        int val = 0;
        if (serverWeb.hasArg("val")) {
            val = serverWeb.arg("val").toInt();
        }
        webRetResult(serverWeb, ledCmdSignal.onCmd(handler, val) ? er_ok : er_errorResult);
    });
    if (config.getBool("HAS_IR")) {
        serverWeb.on("/get_rc_val", [&]() {
            DBG_FUNK();
            uint64_t val;
            const auto ledpre = ledCmdSignal.getVal();
            if (false == IRSignal.getExclusive(val, 5000, []() {
                ledCmdSignal.toggle(0);
            })
            ) {
                ledCmdSignal.set(ledpre);
                webRetResult(serverWeb, er_timeout);
                return;
            }
            ledCmdSignal.set(ledpre);

            wifiHandle_send_content_json(serverWeb, [=](ostream &out) {
                out << "{\"rc_val\":";
                out << val;
                out << "}";
                return er_ok;
            });
        }
        );
    }

    serverWeb.on("/getlogs", HTTP_ANY,
            [&]() {
                serverWeb.send(200, "text/plain", log_buffer.c_str());
                log_buffer = "";
            });

    serverWeb.on("/set_time", [&]() {
        DBG_FUNK();
        //todo
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
    WiFi.begin();
    wifiStateSignal.onSignal([](const wl_status_t &status) {
        wifi_status(cout);
    }
    );
    wifiStateSignal.begin();
    if (is_safe_mode) {
        WiFi.persistent(false);
        WiFi.mode(WIFI_AP);
        WiFi.softAP(pDeviceName, DEF_AP_PWD);
        DBG_OUT << "safemode AP " << pDeviceName << ",pwd: " << DEF_AP_PWD << ",ip:" << WiFi.softAPIP().toString() << std::endl;
    } else if (WIFI_STA == WiFi.getMode()) {
        DBG_OUT << "connecting <" << WiFi.SSID() << "> " << endl;
    }
}

void setup_signals() {
    DBG_FUNK();
    LDRSignal.onSignal([](const uint8_t &level) {
        matrix.setIntensity(level);
        cout << "matrix.setIntensity=" << level << endl;
    }
    );
    ledCmdSignal.onSignal([&](const uint16_t val) {
        mqtt_send();
    });

    timeKeeper.onSignal([](const time_t &time) {
        if (timeNotSet == timeStatus()) {
            return;
        }
        static auto preMinute = static_cast<uint8_t>(0xff);
        const auto curMinute = minute(time);
        if (curMinute == preMinute) {
            return;
        }
        preMinute = curMinute;
        matrix.fillScreen(LOW);
        matrix.setCursor(0, 7);
        char buffMin[6];
        sprintf_P(buffMin, "%2u:%02u", hour(time), curMinute);
        matrix.print(buffMin);
        matrix.write();
    });
    LDRSignal.begin();
    timeKeeper.begin();
}

void setup_mqtt() {
    DBG_FUNK();
    mqtt.setup(config.getCSTR("MQTT_SERVER"), config.getInt("MQTT_PORT"), pDeviceName);
    string topic = "cmd/";
    topic += pDeviceName;

    mqtt.callback(topic, [](char *topic, byte *payload, unsigned int length) {
        DBG_OUT << "MQTT>>[" << topic << "]:";
        auto tt = reinterpret_cast<const char*>(payload);
        auto i = length;
        while (i--) {
            DBG_OUT << *tt;
            tt++;
        };
        DBG_OUT << endl;
        StaticJsonDocument<512> json_cmd;
        DeserializationError error = deserializeJson(json_cmd, payload, length);
        if (error) {
            DBG_OUT << "Failed to read file, using default configuration" << endl;
        } else {
            if (json_cmd.containsKey("led")) {
                ledCmdSignal.set(json_cmd["led"]);
            }
            if (json_cmd.containsKey("time")) {
                ledCmdSignal.set(json_cmd["led"]);
            }
        }
    });
}

void setup_config() {
    config.getConfig().clear();
    config.getConfig()["DEVICE_NAME"] = "CLOCK";
    config.getConfig()["MQTT_SERVER"] = "";
    config.getConfig()["MQTT_PORT"] = 0;
    config.getConfig()["MQTT_PERIOD"] = 60 * 1000;
    config.getConfig()["OTA_USERNAME"] = "";
    config.getConfig()["OTA_PASSWORD"] = "";
    config.getConfig()["LED_MATRIX_ROTATION"] = 0;
    config.getConfig()["HAS_IR"] = 0;
    config.getConfig()["HAS_WALLSWITCH"] = 0;
    config.getConfig()["LDR_MIN"] = 0;
    config.getConfig()["LDR_MAX"] = 1000;
    if (!config.load("/www/config/config.json")) {
        //write file
        config.write("/www/config/config.json");
    }
    pDeviceName = config.getCSTR("DEVICE_NAME");
}

void setup() {
    is_safe_mode = isSafeMode(GPIO_PIN_WALL_SWITCH, 3000);

    Serial.begin(SERIAL_BAUND);
    logs_begin();
    DBG_FUNK();
    DBG_OUT << "is_safe_mode=" << is_safe_mode << endl;
    hw_info(DBG_OUT);
    LittleFS.begin();
    setup_config();

    LDRSignal.setRange(config.getInt("LDR_MIN"), config.getInt("LDR_MAX"));
    dimableLed.setup(config.getBool("HAS_IR"), config.getBool("HAS_WALLSWITCH"));
    MDNS.addService("http", "tcp", SERVER_PORT_WEB);
    MDNS.begin(pDeviceName);
    setup_WebPages();
    setup_signals();

    LittleFS_info(DBG_OUT);
    setup_matrix();
    setup_mqtt();

    ntpTime.init();
//------------------
    dht.setup(DHTPin, DHTesp::DHT22);
//-----------------
    matrix.fillScreen(LOW);
    matrix.setCursor(0, 7);
    matrix.print("--:--");
    matrix.write();
    setup_WIFIConnect();
    DBG_OUT << "Setup done" << endl;
}

static unsigned long nextMsgMQTT = 0;

void mqtt_send() {
    if (mqtt.isConnected()) {
        nextMsgMQTT = millis() + config.getULong("MQTT_PERIOD");
        string topic = "stat/";
        topic += pDeviceName;
        ostringstream payload;
        get_status(payload);
        DBG_OUT << "MQTT<<[" << topic << "]:" << payload.str() << endl;
        mqtt.publish(topic, payload.str());
    } else {
        nextMsgMQTT = 0; //force to send after connection
    }
}

void mqtt_loop() {
    if (WL_CONNECTED != WiFi.status()) {
        return;
    }
    mqtt.loop();

    if (millis() >= nextMsgMQTT) { //send
        mqtt_send();
    }

}

void loop() {
    wifiStateSignal.loop();
    mqtt_loop();
    ntpTime.loop();
    dimableLed.loop();
    LDRSignal.loop();
    timeKeeper.loop();
    serverWeb.handleClient();
}
