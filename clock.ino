#include "clock.h"

const int pinCS = 12; // Attach CS to this pin, DIN to MOSI and CLK to SCK (cf http://arduino.cc/en/Reference/SPI )
const int numberOfHorizontalDisplays = 4;
const int numberOfVerticalDisplays = 1;

const int DHTPIN = D4;
const int pinButton = D3;

const int32 SYNK_RTC_PERIOD = 30 * 60 * 1000; //one per hour
const unsigned long SYNK_NTP_PERIOD = 24 * 60 * 60 * 1000; // one per day

const long MQTT_REFRESH_PERIOD=15*60*1000;

//channels/<channelID>/publish/fields/field<fieldnumber>/<apikey>
const char* mqtt_post_field_single="channels/%d/publish/fields/field%d/%s";

//channels/<channelID>/publish/<apikey>
//field1=100&field2=50&lat=30.61&long=40.3
const char* mqtt_post_fields="channels/%d/publish/%s";


Max72xxPanel matrix = Max72xxPanel(pinCS, numberOfHorizontalDisplays, numberOfVerticalDisplays);

class CIntClock:public  ObserverWrite<time_t>{
public:
    virtual void writeValue(time_t value){
 //       time_t cur= now();
        setTime(value);
    }
};
#ifdef USE_HW_RTC
class CHWClock:public  ObserverWrite<time_t>,public CSubjectPeriodic<time_t>{
    virtual uint32_t getTimeInMs(){     return millis();  }
    RtcDS3231<TwoWire> rtc;
public:
    CHWClock(uint32_t period):CSubjectPeriodic<time_t>(period,10*1000),rtc(Wire){};
    virtual void writeValue(time_t value){
        RtcDateTime dt;
        dt.InitWithEpoch32Time(value);
        rtc.SetDateTime(dt);
    }
    virtual bool readValue(time_t &value){
#ifdef DEBUG
        Serial.println("getRTCTime");
#endif
        if (!rtc.IsDateTimeValid()) {
            return false;
        }
        value = rtc.GetDateTime().Epoch32Time();
#ifdef DEBUG
        Serial.printf("rtc GMT %02u:%02u:%02u done\n", hour(value),
                minute(value), second(value));
#endif
        return true;
    }
    void init(){
        //--------RTC SETUP ------------
        Serial.println("RTC SETUP");
        rtc.Begin();

        if (!rtc.IsDateTimeValid())
        {
            // Common Cuases:
            //    1) first time you ran and the device wasn't running yet
            //    2) the battery on the device is low or even missing

            Serial.println("RTC lost confidence in the DateTime!");

            // following line sets the RTC to the date & time this sketch was compiled
            // it will also reset the valid flag internally unless the Rtc device is
            // having an issue

            rtc.SetDateTime(0);
        }

        if (!rtc.GetIsRunning())
        {
            Serial.println("RTC was not actively running, starting now");
            rtc.SetIsRunning(true);
        }

        // never assume the Rtc was last configured by you, so
        // just clear them to your needed state
        rtc.Enable32kHzPin(false);
        rtc.SetSquareWavePin(DS3231SquareWavePin_ModeNone);
    }
};
CHWClock hwClock(SYNK_RTC_PERIOD);
#endif

NTPtime ntpTime(SYNK_NTP_PERIOD);
CIntClock intClock;

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
class CIntensitySet: public ObserverWrite<int16_t> {
public:
    virtual void writeValue(int16_t value) {
        matrix.setIntensity(value);
    }
};

const int16_t itransforms[] = { 0, 200, 600, 1000 };

CLDR ldr;
CFilter_ValueToPos<int16_t> intensityTransform(itransforms, 5);
CFilter_OnChange<int16_t> intensityOnChange;
CIntensitySet intensitySet;

void setup() {
    pinMode(pinButton, INPUT);
    Serial.begin(115200);
    delay(500);
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
    matrix.print(" 0:00");
    matrix.write();

    setup_wifi(wifi_ssid, wifi_password, DEVICE_NAME);
    mqtt.setup(mqtt_server, mqtt_port);
	//--------------

	ntpTime.init();
	ntpTime.addListener(intClock);
#ifdef USE_HW_RTC
	hwClock.init();
	ntpTime.addListener(hwClock);
	hwClock.addListener(intClock);
#endif
    //----------------------
	ldr.addListener(intensityTransform);
    intensityTransform.addListener(intensityOnChange);
    intensityOnChange.addListener(intensitySet);
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
    mqtt.publish(topic, data);

    Serial.print("topic= ");
    Serial.print(topic);
    Serial.print(" [");
    Serial.print(data);
    Serial.println("]");
}

long nextStat=0;
long nextSec = 0;
void log_loop() {

}
void loop() {
	const long now = millis();
	ota.loop();
    wifi_loop();
    mqtt_loop();
    CFilterLoop::loops();

	//update info
    if (now >= nextSec)
	{
        matrix.fillScreen(LOW);
        matrix.setCursor(0, 7);
        matrix.print(displayClock.getStrMin());
        matrix.write();
        nextSec = now + 5000;
	}

	if(now>=nextStat){
		nextStat=now+5000;
		char tt[20];
		displayClock.getFullTime(tt);
		Serial.println(tt);
        Serial.print("LDR sensor =");
        Serial.print(ldr.getValue());

        Serial.print(", button =");
		Serial.println(	digitalRead(pinButton));
	}
 
}
