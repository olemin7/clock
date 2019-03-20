#include "clock.h"
#include "./libs/TimeLib.h"
#include "./libs/Timezone.h"

constexpr auto pinCS = D6;
constexpr auto numberOfHorizontalDisplays = 4;
constexpr auto numberOfVerticalDisplays = 1;

constexpr auto DHTPin = D4;

#ifndef DEBUG
const auto MQTT_REFRESH_PERIOD = 15 * 60 * 1000;
#else
const auto MQTT_REFRESH_PERIOD=5*1000;
#endif

const char* update_path = "/firmware";

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
#ifdef _USE_DIMABLE_LED_
CDimableLed dimableLed;
#endif
CLightDetectResistor ldr;
static uint8_t preLevel = 0;
DHTesp dht;

ESP8266WebServer server(80);
#ifdef MQTT_ENABLE
CMQTT mqtt;
#endif
ESP8266HTTPUpdateServer otaUpdater;

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
void handleRoot() {
  char temp[400];
  const auto local = get_local_time();
  snprintf(temp, sizeof(temp),
  "<html>\
  <head>\
    <meta http-equiv='refresh' content='5'/>\
    <title>" DEVICE_NAME "</title>\
  </head>\
  <body>\
    "DEVICE_NAME "<br>\
    timeStatus= %d <br>\
    time: %02d:%02d <br>\
        Temperature= %f <br>\
        Humidity= %f <br>\
        LDR=%d (%d)<br>\
  </body>\
</html>",
      timeStatus(), hour(local), minute(local),
      dht.getTemperature(),dht.getHumidity(),
      preLevel, ldr.get()
      );
  //Serial.println(temp);
  server.send(200, "text/html", temp);

}
void setup() {

  Serial.begin(115200);
  //pinMode(DHTPin, INPUT); setted in driver
  delay(500);
#ifdef _USE_DIMABLE_LED_
  dimableLed.setup();
#endif

  Serial.println(DEVICE_NAME);
  Serial.println("Compiled " __DATE__ " " __TIME__);
  Serial.println();

  hw_info(Serial);
  setup_matrix();
  
  setup_wifi(wifi_ssid, wifi_password, DEVICE_NAME);
  MDNS.begin(DEVICE_NAME);
#ifdef MQTT_ENABLE
  mqtt.setup(mqtt_server, mqtt_port);
#endif
	//--------------

	ntpTime.init();
  ntpTime.setCallback(setTime_);
	//------------------
  dht.setup(DHTPin, DHTesp::DHT22);
	//-----------------
  otaUpdater.setup(&server, update_path, ota_username, ota_password);
  server.on("/", handleRoot);
  server.onNotFound([] {
    Serial.println("Error no handler");
    Serial.println(server.uri());
  });
#ifdef MQTT_ENABLE
  mqtt.setClientID(DEVICE_NAME);
#endif
  sw_info(DEVICE_NAME, Serial);
  server.begin();
  MDNS.addService("http", "tcp", 80);

	Serial.println("Setup done");
  matrix.fillScreen(LOW);
  matrix.setCursor(0, 7);
  matrix.print("--:--");
  matrix.write();
}

void mqtt_loop() {
#ifdef MQTT_ENABLE
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
#endif
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

const auto itransforms = std::array<int16_t, 4> { 250, 500, 750, 1000 };
void intensity_loop() {
  const auto now = millis();
  static long next = 0;
  if (now < next) {
    return;
  }
  next = now + 1000;
  const auto val = ldr.get();
  uint8_t level = 0;
  for (const auto it : itransforms) {
    if (val < it) {
      break;
    }
    level++;
  }
  if (preLevel != level) {
    preLevel = level;
    matrix.setIntensity(level);
    Serial.print("matrix.setIntensity=");
    Serial.print(level);
    Serial.print(",");
    Serial.println(val);
  }
}

void loop() {
  wifi_loop();
  mqtt_loop();
  ntpTime.loop();
  time_loop();	
  intensity_loop();
#ifdef _USE_DIMABLE_LED_
  dimableLed.loop();
#endif
  server.handleClient();
}
