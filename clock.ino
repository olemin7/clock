#include "clock.h"

const int pinCS = 12; // Attach CS to this pin, DIN to MOSI and CLK to SCK (cf http://arduino.cc/en/Reference/SPI )
const int numberOfHorizontalDisplays = 4;
const int numberOfVerticalDisplays = 1;

const int DHTPIN= 4;//gpio16    // what digital pin we're connected to
const int DHTTYPE= DHT22;   // DHT 22  (AM2302), AM2321

const int pinButton=18;//|GPIO0

const int32 SYNK_RTC=30*1000;
const int32 SYNK_NTP=24*60*60*1000;// one per day
int32 synk_ntp_count=0;


Max72xxPanel matrix = Max72xxPanel(pinCS, numberOfHorizontalDisplays, numberOfVerticalDisplays);

RtcDS3231<TwoWire> rtc(Wire);
NTPtime ntpTime;
CLightDetectResistor ldr;
time_t ntp_next_synk;
CDisplayClock displayClock;
int aIntensityRation[][2] ={{10,0},{300,1},{1000,3}};
CIntensity intensity(aIntensityRation,3);
DHT dht(DHTPIN, DHTTYPE);
//US Eastern Time Zone (New York, Detroit)

time_t getRTCTime(){
#ifdef DEBUG
	Serial.println("getRTCTime");
#endif
	if(synk_ntp_count)
		synk_ntp_count--;
	if((WL_CONNECTED==WiFi.status()) &&(
			(0==synk_ntp_count)
			||(!rtc.IsDateTimeValid())
			)){
		#ifdef DEBUG
			Serial.print("SYNK_NTP...");
		#endif
		time_t ntp;
		ntp=ntpTime.getTime();
		if(ntp){
			synk_ntp_count=SYNK_NTP/SYNK_RTC;
			Serial.printf("mtp GMT %02u:%02u:%02u done\n", hour(ntp),minute(ntp),second(ntp));

			RtcDateTime dt;
			dt.InitWithEpoch32Time(ntp);
			rtc.SetDateTime(dt);
			return ntp;
		}
		Serial.println("Error");
	}
	if(!rtc.IsDateTimeValid()){
			return 0;
	}
	time_t rtctime=rtc.GetDateTime().Epoch32Time();
#ifdef DEBUG
	Serial.printf("rtc GMT %02u:%02u:%02u done\n", hour(rtctime),minute(rtctime),second(rtctime));
#endif
	return rtctime;
}

void rtc_init(){
	//--------RTC SETUP ------------
	Serial.println("RTC SETUP");
	rtc.Begin();

	// if you are using ESP-01 then uncomment the line below to reset the pins to
	// the available pins for SDA, SCL
	// Wire.begin(0, 2); // due to limited pins, use pin 0 and 2 for SDA, SCL

	RtcDateTime compiled = displayClock.toUTC(RtcDateTime(__DATE__, __TIME__));
	if (!rtc.IsDateTimeValid())
	{
	    // Common Cuases:
	    //    1) first time you ran and the device wasn't running yet
	    //    2) the battery on the device is low or even missing

	    Serial.println("RTC lost confidence in the DateTime!");

	    // following line sets the RTC to the date & time this sketch was compiled
	    // it will also reset the valid flag internally unless the Rtc device is
	    // having an issue

	    rtc.SetDateTime(compiled);
	}

	if (!rtc.GetIsRunning())
	{
	    Serial.println("RTC was not actively running, starting now");
	    rtc.SetIsRunning(true);
	}

	RtcDateTime now = rtc.GetDateTime();
	if (now < compiled)
	{
	    Serial.println("RTC is older than compile time!  (Updating DateTime)");
	    rtc.SetDateTime(compiled);
	}
	else if (now > compiled)
	{
	    Serial.println("RTC is newer than compile time. (this is expected)");
	}
	else if (now == compiled)
	{
	    Serial.println("RTC is the same as compile time! (not expected but all is fine)");
	}

	// never assume the Rtc was last configured by you, so
	// just clear them to your needed state
	rtc.Enable32kHzPin(false);
	rtc.SetSquareWavePin(DS3231SquareWavePin_ModeNone);

}
int ldr_get(){
	return ldr.get();
}
void setIntensity(int level){
	matrix.setIntensity(level);
}

void setup() {

  Serial.begin(115200);
  Serial.println();
  Serial.println("Compiled " __DATE__ " " __TIME__);
  Serial.println();

  delay(500);
  matrix.setIntensity(0); // Use a value between 0 and 15 for brightness

  matrix.setRotation(0,1);
  matrix.setRotation(1,1);
  matrix.setRotation(2,1);
  matrix.setRotation(3,1);

  matrix.setFont(&FreeMono9pt7b);
  matrix.setTextWrap(false);
  matrix.fillScreen(LOW);
  matrix.setTextColor(HIGH, LOW);
  matrix.setTextSize(1);
  matrix.print("Start");
  matrix.write();

  // We start by connecting to a WiFi network
  Serial.print("WiFi Connecting to ");
  Serial.print(ssid);
  WiFi.begin(ssid, password);

  if(WiFi.waitForConnectResult() != WL_CONNECTED){
   Serial.println(". Failed");
	}else{
		Serial.print(". Connected, IP address: ");
		Serial.println(WiFi.localIP());
	}
	//--------------
	rtc_init();
	ntpTime.init();
	setSyncProvider(getRTCTime);
	setSyncInterval(SYNK_RTC/1000);
	synk_ntp_count=0;
	//----------------------
	intensity.setGetEnviropment(ldr_get);
	intensity.setSetIntensity(setIntensity);

	ota_begin("clock" __DATE__,ota_password);
	//------------------
	dht.begin();
	//-----------------
	Serial.println("Setup done");
}


unsigned long period=0;
wl_status_t wl_status= WL_IDLE_STATUS;
void loop() {
	ota_loop();
	period++;
	delay(10);

	if(wl_status!=WiFi.status()){
		wl_status=WiFi.status();
		Serial.printf("WiFi.status %d\n",wl_status);
		if(WL_CONNECTED==wl_status){
			  Serial.print("WiFi connected, IP address: ");
			  Serial.println(WiFi.localIP());
		}
	}

	//update info
	if(displayClock.isChangedMin())
	{
			matrix.fillScreen(LOW);
			matrix.setCursor(0,7);
			matrix.print(displayClock.getStrMin());
			matrix.write();
	}

	if((period%500)==0){//5 sec
		char tt[20];
		displayClock.getFullTime(tt);
		Serial.println(tt);
		Serial.printf("LDR sensor %d , temperature ",	ldr.get());
		Serial.print(rtc.GetTemperature().AsFloat());
		Serial.println(" C");
		intensity.handle();
	}

}
