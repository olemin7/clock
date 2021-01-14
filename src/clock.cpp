#include "clock.h"
using namespace std;

constexpr auto pinCS = D6;
constexpr auto numberOfHorizontalDisplays = 4;
constexpr auto numberOfVerticalDisplays = 1;

constexpr auto DHTPin = D4;

#ifndef DEBUG
const auto MQTT_REFRESH_PERIOD = 15 * 60 * 1000;
#else
const auto MQTT_REFRESH_PERIOD = 5 * 1000;
#endif

const char *update_path = "/firmware";

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
#ifdef  LED_MATRIX_ROTATION
    matrix.setRotation(0, LED_MATRIX_ROTATION);
    matrix.setRotation(1, LED_MATRIX_ROTATION);
    matrix.setRotation(2, LED_MATRIX_ROTATION);
    matrix.setRotation(3, LED_MATRIX_ROTATION);
#endif //LED_MATRIX_ROTATION
    matrix.setFont(&FreeMono9pt7b);
    matrix.setTextWrap(false);
    matrix.fillScreen(LOW);
    matrix.setTextColor(HIGH, LOW);
    matrix.setTextSize(1);
    matrix.setCursor(0, 7);
    matrix.print("init");
    matrix.write();
}

void http_about()
{
    DBG_PRINTLN("http_about ");
    serverWeb.setContentLength(CONTENT_LENGTH_UNKNOWN);
    serverWeb.sendHeader("Content-Type", "text/plain", true);
    std::ostringstream sAbout;

    sAbout << "Compiled :" << __DATE__ << " " << __TIME__ << std::endl;
    sAbout << "Serial Speed " << SERIAL_BAUND << std::endl;
    sAbout << getResetInfo().c_str() << std::endl;
    sAbout << "RSSI " << WiFi.RSSI() << std::endl;

    sAbout << std::endl;

    const auto local = get_local_time();
    sAbout << "timeStatus= " << timeStatus();
    sAbout << "time: " << hour(local) << ":" << minute(local) << std::endl;
    sAbout << "Temperature= " << dht.getTemperature() << ", Humidity=" << dht.getHumidity() << std::endl;
    sAbout << "LDR=" << LDRSignal.get() << std::endl;

    serverWeb.sendContent(sAbout.str().c_str());
#ifdef DEBUG_STREAM
    serverWeb.sendContent("\nDEBUG_STREAM on");
#endif
    //---------------
    serverWeb.sendContent("");
}

void setup_WebPages() {
    otaUpdater.setup(&serverWeb, update_path, ota_username, ota_password);

    serverWeb.on("/restart", []() {
        webRetResult(serverWeb, er_ok);
        delay(1000);
        ESP.restart();
    });
    serverWeb.on("/about", http_about);
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
        if (false == IRSignal.getExclusive(val)) {
            webRetResult(serverWeb, er_timeout);
            return;
        }

        serverWeb.setContentLength(CONTENT_LENGTH_UNKNOWN);
        serverWeb.sendHeader("Content-Type", "application/json", true);
        serverWeb.sendHeader("Cache-Control", "no-cache");
        ostringstream line;
        line << "{\"rc_val\":";
        line << val;
        line << "}";
        serverWeb.sendContent(line.str().c_str());
        serverWeb.sendContent("");
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
    WiFi.begin();
    wifiStateSignal.onSignal([](const wl_status_t &status) {
        wifi_status(cout);
    }
    );
    wifiStateSignal.begin();
    if (WIFI_STA == WiFi.getMode())
            {
        const auto now = millis() + WIFI_CONNECT_TIMEOUT;
        cout << "connecting <" << WiFi.SSID() << "> " << endl;
        while (now > millis()) {
            if (WL_CONNECTED == WiFi.status()) {
                return;
            }
            blink();
        }
        //Temporary server
        //todo add key to swith
        cout << "fail" << endl;
        WiFi.persistent(false);
        WiFi.mode(WIFI_AP);
        WiFi.softAP(DEVICE_NAME, DEF_AP_PWD);
        cout << "AP " << DEVICE_NAME << ",pwd: " << DEF_AP_PWD << ",ip:" << WiFi.softAPIP().toString() << std::endl;

    }
}

void setup() {
    pinMode(LED_BUILTIN, OUTPUT);
    //pinMode(DHTPin, INPUT); setted in driver

    Serial.begin(SERIAL_BAUND);
    DBG_PRINTLN(DEVICE_NAME);
    hw_info(cout);
    LittleFS.begin();
    dimableLed.setup();
    MDNS.addService("http", "tcp", SERVER_PORT_WEB);
    MDNS.begin(DEVICE_NAME);
    setup_WebPages();
    LDRSignal.onSignal([](const uint8_t &level) {
        matrix.setIntensity(level);
        cout << "matrix.setIntensity=" << level << endl;
    }
    );
    LDRSignal.begin();

    LittleFS_info(cout);
    setup_matrix();

    mqtt.setup(mqtt_server, mqtt_port);
    //--------------

    ntpTime.init();
    ntpTime.setCallback(setTime_);
    //------------------
    dht.setup(DHTPin, DHTesp::DHT22);
    //-----------------

    mqtt.setClientID(DEVICE_NAME);

    matrix.fillScreen(LOW);
    matrix.setCursor(0, 7);
    matrix.print("--:--");
    matrix.write();
    setup_WIFIConnect();
    cout << "Setup done" << endl;
}

void mqtt_loop() {
    if (WL_CONNECTED != WiFi.status()) {
        return;
    }

    const auto now = millis();
    mqtt.loop();
    static long nextMsgMQTT = 0;
    if (now < nextMsgMQTT) {
        return;
    }

    const auto dht_readTemperature = dht.getTemperature();
    const auto dht_readHumidity = dht.getHumidity();
    Serial.print("t=");
    Serial.print(dht_readTemperature);
    Serial.print(" h=");
    Serial.println(dht_readTemperature);

    if (isnan(dht_readTemperature) || isnan(dht_readHumidity)) {
        Serial.println("Failed to read from sensor!");
        nextMsgMQTT = now + 5 * 1000;
        return;
    }

    nextMsgMQTT = now + MQTT_REFRESH_PERIOD;

    String topic;

    topic = "channels/" + String(House_channelID) + "/publish/"
            + House_Write_API_Key;
    String data;
    data = "field" + String(MQTT_TEMPERATURE) + "="
            + String(dht_readTemperature, 1);
    data += "&field" + String(MQTT_HUMIDITY) + "="
            + String(dht_readHumidity, 1);
#ifndef DEBUG
    mqtt.publish(topic, data);
#endif
    Serial.print("topic= ");
    Serial.print(topic);
    Serial.print(" [");
    Serial.print(data);
    Serial.println("]");
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
