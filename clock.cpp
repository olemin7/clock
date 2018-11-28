#include "clock.h"
#include "./libs/TimeLib.h"
#include "./libs/Timezone.h"

const int pinCS = 12; // Attach CS to this pin, DIN to MOSI and CLK to SCK (cf http://arduino.cc/en/Reference/SPI )
const int numberOfHorizontalDisplays = 4;
const int numberOfVerticalDisplays = 1;

const int DHTPin = D4;

#ifndef DEBUG
    const long MQTT_REFRESH_PERIOD=15*60*1000;
#else
    const long MQTT_REFRESH_PERIOD=5*1000;
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

Max72xxPanel matrix = Max72xxPanel(pinCS, numberOfHorizontalDisplays, numberOfVerticalDisplays);

NTPtime ntpTime;
CDimableLed dimableLed;
CLightDetectResistor ldr;
DHTesp dht;
//US Eastern Time Zone (New York, Detroit)

ESP8266WebServer server(80);
CMQTT mqtt;
ESP8266HTTPUpdateServer otaUpdater;

void setup() {

  Serial.begin(115200);
  pinMode(GPIO_PIN_WALL_SWITCH, INPUT_PULLUP);
  pinMode(DHTPin, INPUT);
    delay(500);
    dimableLed.setup();
    Serial.println(DEVICE_NAME);
    Serial.println("Compiled " __DATE__ " " __TIME__);
    Serial.println();

    hw_info(Serial);

    matrix.setIntensity(0); // Use a value between 0 and 15 for brightness

    matrix.setRotation(0, 1);
    matrix.setRotation(1, 1);
    matrix.setRotation(2, 1);
    matrix.setRotation(3, 1);

    matrix.setFont(&FreeMono9pt7b);
    matrix.setTextWrap(false);
    matrix.fillScreen(LOW);
    matrix.setTextColor(HIGH, LOW);
    matrix.setTextSize(1);
    matrix.setCursor(0, 7);
    matrix.print("init");
    matrix.write();

    setup_wifi(wifi_ssid, wifi_password, DEVICE_NAME);
    mqtt.setup(mqtt_server, mqtt_port);
	//--------------

	ntpTime.init();
  ntpTime.setCallback(setTime_);
	//------------------
  dht.setup(DHTPin, DHTesp::DHT22);
	//-----------------
  otaUpdater.setup(&server, update_path, ota_username, ota_password);
  mqtt.setClientID(DEVICE_NAME);
  sw_info(DEVICE_NAME, Serial);
	Serial.println("Setup done");
}

void mqtt_loop() {
    if (WL_CONNECTED != WiFi.status()) {
        return;
    }

    const long now = millis();
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
  const auto now = millis();
  static long nextSec = 0;
  //update info
  if (now < nextSec) {
    return;
  }
  matrix.fillScreen(LOW);
  matrix.setCursor(0, 7);
  if (timeNotSet == timeStatus()) {
    matrix.print("synk.");
  } else {
    const auto local = get_local_time();
    char buffMin[6];
    sprintf_P(buffMin, "%2u:%02u", hour(local), minute(local));
    matrix.print(buffMin);
  }
  matrix.write();
  nextSec = now + 5000;
}

const std::array<int16_t, 4> itransforms = { 0, 200, 600, 1000 };
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
  static uint8_t preLevel = 0;
  if (preLevel != level) {
    preLevel = level;
    matrix.setIntensity(level);
    Serial.print("matrix.setIntensity=");
    Serial.println(level);
  }
}
void loop() {
  wifi_loop();
  mqtt_loop();
  ntpTime.loop();
  time_loop();	
  intensity_loop();
  dimableLed.loop();
}
