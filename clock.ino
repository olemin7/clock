#include "clock.h"

const int pinCS = 12; // Attach CS to this pin, DIN to MOSI and CLK to SCK (cf http://arduino.cc/en/Reference/SPI )
const int numberOfHorizontalDisplays = 4;
const int numberOfVerticalDisplays = 1;

const int DHTPIN 	= D4;

const int32 SYNK_RTC_PERIOD = 30 * 60 * 1000; //one per hour
const unsigned long SYNK_NTP_PERIOD = 24 * 60 * 60 * 1000; // one per day
#ifndef DEBUG
    const long MQTT_REFRESH_PERIOD=15*60*1000;
#else
    const long MQTT_REFRESH_PERIOD=5*1000;
#endif
//channels/<channelID>/publish/fields/field<fieldnumber>/<apikey>
const char* mqtt_post_field_single="channels/%d/publish/fields/field%d/%s";

//channels/<channelID>/publish/<apikey>
//field1=100&field2=50&lat=30.61&long=40.3
const char* mqtt_post_fields="channels/%d/publish/%s";


Max72xxPanel matrix = Max72xxPanel(pinCS, numberOfHorizontalDisplays, numberOfVerticalDisplays);

class CSetClock:public  ObserverWrite<time_t>{
public:
    virtual void writeValue(time_t value){
        setTime(value);
    }
};
/***
 *
 */
class CClock:public Observer<time_t>,CSubjectPeriodic<time_t>{
	virtual uint32_t getTimeInMs(){return now();}
	void update(const Subject<time_t> &time){
		setValue(time.getValue());
	}
	bool readValue(time_t &time){
		if(timeNotSet==timeStatus())return false;
		time=now();
		return true;
	}
public:
	CClock(uint32_t aperiod):CSubjectPeriodic<time_t>(aperiod){};
};


NTPtime ntpTime(SYNK_NTP_PERIOD);
CSetClock setClock;
CClock Clock1sec(1000);

CDimableLed dimableLed;

CDisplayClock displayClock;

DHT dht(DHTPIN, DHT22);
//US Eastern Time Zone (New York, Detroit)

CMQTT mqtt;
COTA ota;

class CLDR: public CSubjectPeriodic<int16_t> {
    CLightDetectResistor ldr;
    virtual uint32_t getTimeInMs() {
        return millis();
    }
public:
    CLDR() : CSubjectPeriodic<int16_t>(100) {};
    bool readValue(int16_t &value) {
        value=ldr.get();
        return true;
    }
};
const int16_t itransforms[] = { 0, 200, 600, 1000 };
class CIntensitySet: public ObserverWrite<int16_t> {
	CFilter_ValueToPos<int16_t> intensityTransform;
	CFilter_OnChange<int16_t> intensityOnChange;
public:
	CLDR ldr;
    virtual void writeValue(int16_t value) {
        matrix.setIntensity(value);
        Serial.print("matrix.setIntensity=");
        Serial.println(value);
    }
    CIntensitySet():intensityTransform(itransforms, 5){
    	ldr.addListener(intensityTransform);
        intensityTransform.addListener(intensityOnChange);
        intensityOnChange.addListener(*this);
    }
};
CIntensitySet intensitySet;



void setup() {

    Serial.begin(115200);
    pinMode(GPIO_PIN_WALL_SWITCH, INPUT_PULLUP);
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
	ntpTime.addListener(setClock);
	ntpTime.addListener(Clock1sec);

    //----------------------
	//------------------
	dht.begin();
	//-----------------
    ota.begin(DEVICE_NAME, ota_password);
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

    float dht_readTemperature = dht.readTemperature();
    float dht_readHumidity = dht.readHumidity();
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

long nextSec = 0;

void loop() {
	const long now = millis();
	ota.loop();
    wifi_loop();
 //   mqtt_loop();
    CFilterLoop::loops();

	//update info
    if (now >= nextSec)
	{
        matrix.fillScreen(LOW);
        matrix.setCursor(0, 7);
        if(timeNotSet== timeStatus()){
            matrix.print("synk.");
        }else{
            matrix.print(displayClock.getStrMin());
        }
        matrix.write();
        nextSec = now + 5000;
	}
}
