#include "clock.h"
using namespace std;

constexpr auto pinCS = D6;
constexpr auto numberOfHorizontalDisplays = 4;
constexpr auto numberOfVerticalDisplays = 1;

constexpr auto DHTPin = D4;

const char *update_path = "/firmware";
bool is_safe_mobe = false;

Timezone myTZ((TimeChangeRule ) { "DST", Last, Sun, Mar, 3, +3 * 60 },
        (TimeChangeRule ) { "STD", Last, Sun, Oct, 4, +2 * 60 });

time_t get_local_time() {
    return myTZ.toLocal(now());
}
void setTime_(const time_t &par) {
    setTime(par);
    Serial.println("setTime_");
}

auto matrix = Max72xxPanel(pinCS, numberOfHorizontalDisplays, numberOfVerticalDisplays);

NTPtime ntpTime;
void mqtt_send();

class CLDRSignal: public SignalChange<uint8_t> {
    CLightDetectResistor ldr;
    uint8_t getValue() {
        static const auto itransforms = std::array<int16_t, 4> { 250, 500, 750, 1000 };
        const auto val = ldr.get();
        uint8_t level = 0;
        for (const auto it : itransforms) {
            if (val < it) {
                break;
            }
            level++;
        }
        return level;
    }
public:
    int16_t get() {
        return ldr.get();
    }
} LDRSignal;

DHTesp dht;

ESP8266WebServer serverWeb(SERVER_PORT_WEB);
CMQTT mqtt;
ESP8266HTTPUpdateServer otaUpdater;
CWifiStateSignal wifiStateSignal;

void setup_matrix() {
    matrix.setIntensity(0); // Use a value between 0 and 15 for brightness
    const auto rotation = config.getLedMattixRotation();
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
    out << ",\"deviceName\":\"" << config.getDeviceName() << "\"";
    out << ",\"resetInfo\":" << system_get_rst_info()->reason;
    out << "}";
    return er_ok;
}

te_ret get_status(ostream &out) {
    const auto local = get_local_time();
    out << "{\"timeStatus\":" << static_cast<unsigned>(timeStatus());
    out << ",\"time\":\"" << std::ctime(&local) << "\"";
    const auto temp = dht.getTemperature();
    out << ",\"temperature\":" << isnan(temp) ? "null" : temp;
    const auto humm = dht.getHumidity()
    out << ",\"humidity\":" << isnan(humm) ? "null" : humm;
    out << ",\"ldr\":" << LDRSignal.get();
    out << ",\"led\":" << static_cast<unsigned>(ledCmdSignal.getVal());
    out << "}";
    return er_ok;
}

void setup_WebPages() {
    otaUpdater.setup(&serverWeb, update_path, config.getOtaUsername(), config.getOtaPassword());

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
        DBG_PRINTLN("filesave");
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
    serverWeb.on("/get_rc_val", [&]() {
        DBG_PRINTLN("get_rc_val ");
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

        serverWeb.setContentLength(CONTENT_LENGTH_UNKNOWN);
        serverWeb.sendHeader("Content-Type", "application/json", true);
        serverWeb.sendHeader("Cache-Control", "no-cache");
        ostringstream line;
        line << "{\"rc_val\":";
        line << val;
        line << "}";
        serverWeb.sendContent(line.str().c_str());
        serverWeb.sendContent("");
    }
    );

    serverWeb.serveStatic("/", LittleFS, "/www/");
    serverWeb.onNotFound([] {
        Serial.println("Error no handler");
        Serial.println(serverWeb.uri());
        webRetResult(serverWeb, er_fileNotFound);
    });
    serverWeb.begin();
}

void setup_WIFIConnect() {
    WiFi.begin();
    wifiStateSignal.onSignal([](const wl_status_t &status) {
        wifi_status(cout);
    }
    );
    wifiStateSignal.begin();
    if (is_safe_mobe) {
        WiFi.persistent(false);
        WiFi.mode(WIFI_AP);
        WiFi.softAP(config.getDeviceName(), DEF_AP_PWD);
        DBG_OUT << "safemode AP " << config.getDeviceName() << ",pwd: " << DEF_AP_PWD << ",ip:" << WiFi.softAPIP().toString() << std::endl;
    } else if (WIFI_STA == WiFi.getMode()) {
        DBG_OUT << "connecting <" << WiFi.SSID() << "> " << endl;
    }
}

void setup() {
    is_safe_mobe = isSafeMode(GPIO_PIN_WALL_SWITCH, 3000);

    Serial.begin(SERIAL_BAUND);
    CDBG_FUNK();
    DBG_OUT << "is_safe_mobe=" << is_safe_mobe << endl;
    hw_info(cout);
    LittleFS.begin();
    if (!config.setup() || is_safe_mobe) {
        config.setDefault();
    }
    dimableLed.setup();
    MDNS.addService("http", "tcp", SERVER_PORT_WEB);
    MDNS.begin(config.getDeviceName());
    setup_WebPages();
    LDRSignal.onSignal([](const uint8_t &level) {
        matrix.setIntensity(level);
        cout << "matrix.setIntensity=" << level << endl;
    }
    );
    ledCmdSignal.onSignal([&](const uint16_t val) {
        mqtt_send();
    });
    LDRSignal.begin();

    LittleFS_info(cout);
    setup_matrix();

    mqtt.setup(config.getMqttServer(), config.getMqttPort(), config.getDeviceName());
    string topic = "cmd/";
    topic += config.getDeviceName();

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
        }
    });
//--------------

    ntpTime.init();
    ntpTime.setCallback(setTime_);
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

static long nextMsgMQTT = 0;
void mqtt_send() {
    nextMsgMQTT = millis() + config.getMqttPeriod();

    string topic = "stat/";
    topic += config.getDeviceName();
    ostringstream payload;
    get_status(payload);
    DBG_OUT << "MQTT<<[" << topic << "]:" << payload.str() << endl;
    mqtt.publish(topic, payload.str());
}

void mqtt_loop() {
    if (WL_CONNECTED != WiFi.status()) {
        return;
    }
    mqtt.loop();

    if (millis() >= nextMsgMQTT) {
        mqtt_send();
    }
}

void time_loop() {
    if (timeNotSet == timeStatus()) {
        return;
    }
    const auto local = get_local_time();
    static auto preMinute = static_cast<uint8_t>(0xff);
    const auto curMinute = minute(local);
    if (curMinute == preMinute) {
        return;
    }
    preMinute = curMinute;
    matrix.fillScreen(LOW);
    matrix.setCursor(0, 7);
    char buffMin[6];
    sprintf_P(buffMin, "%2u:%02u", hour(local), curMinute);
    matrix.print(buffMin);
    matrix.write();
}

void loop() {
    wifiStateSignal.loop();
    mqtt_loop();
    ntpTime.loop();
    time_loop();
    dimableLed.loop();
    LDRSignal.loop();
    serverWeb.handleClient();
}
