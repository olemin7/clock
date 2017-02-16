#include <SPI.h>
#include <Adafruit_GFX.h>
#include <Max72xxPanel.h>

#include <ESP8266WiFi.h>
#include <WiFiUdp.h>
#include <TimeLib.h>
#include <Timezone.h>

#include <Wire.h> // must be included here so that Arduino library object file references work
#include <RtcDS3231.h>
#include <pgmspace.h>
#include <time.h>

#include "NTPtime.h"
#include "CLightDetectResistor.h"

#if 0
char ssid[] = "Guest1";  //  your network SSID (name)
char pass[] = "MH-6346PQMS";       // your network password
#else
char ssid[] = "ITPwifi";  //  your network SSID (name)
char pass[] = "_RESTRICTED3db@ea";       // your network password
#endif
#define DEBUG


int pinCS = 2; // Attach CS to this pin, DIN to MOSI and CLK to SCK (cf http://arduino.cc/en/Reference/SPI )
int numberOfHorizontalDisplays = 4;
int numberOfVerticalDisplays = 1;

const int32 SYNK_RTC=30*1000;
const int32 SYNK_NTP=60*1000;
int32 synk_ntp_count=0;

Max72xxPanel matrix = Max72xxPanel(pinCS, numberOfHorizontalDisplays, numberOfVerticalDisplays);

int wait = 20; // In milliseconds

int spacer = 1;
int width = 5 + spacer; // The font width is 5 pixels

RtcDS3231<TwoWire> rtc(Wire);
NTPtime ntpTime;
CLightDetectResistor ldr;
time_t ntp_next_synk;

//US Eastern Time Zone (New York, Detroit)
TimeChangeRule myDST = {"DST", Last, Sun, Mar, 3, +3*60};    //Daylight time = UTC - 4 hours
TimeChangeRule mySTD = {"STD", Last, Sun, Oct, 4, +2*60};     //Standard time = UTC - 5 hours
Timezone myTZ(myDST, mySTD);

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
			Serial.printf("GMT %02u:%02u:%02u done\n", hour(ntp),minute(ntp),second(ntp));

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
	rtc.Begin();

	// if you are using ESP-01 then uncomment the line below to reset the pins to
	// the available pins for SDA, SCL
	// Wire.begin(0, 2); // due to limited pins, use pin 0 and 2 for SDA, SCL

	RtcDateTime compiled = RtcDateTime(__DATE__, __TIME__);
	Serial.println();

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

void setup() {

  matrix.setIntensity(1); // Use a value between 0 and 15 for brightness
  matrix.setRotation(0,1);
  matrix.setRotation(1,1);
  matrix.setRotation(2,1);
  matrix.setRotation(3,1);

  Serial.begin(115200);
  Serial.println();
  Serial.println();

  // We start by connecting to a WiFi network
  Serial.print("Connecting to ");
  Serial.println(ssid);
  WiFi.begin(ssid, pass);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");

  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());


  //--------------
  ntpTime.init();
  setSyncProvider(getRTCTime);
  setSyncInterval(SYNK_RTC/1000);

  matrix.setTextColor(HIGH, LOW);
  matrix.setTextSize(1);
  matrix.print("test");
}
unsigned long period=0;
void loop() {

	period++;
	delay(10);

	if(0==(period%100)){
		time_t local=myTZ.toLocal(now());
		Serial.printf("%02u:%02u:%02u\n", hour(local),minute(local),second(local));
	}


//	if(0==(period%50)){//fast 0.5 sec
//		int32 time;
//		if(rtc.getTime(time))
//	    {
//	        // Common Cuases:
//	        //    1) the battery on the device is low or even missing and the power line was disconnected
//	        Serial.println("RTC lost confidence in the DateTime!");
//	    }
//	    RtcDateTime now;
//	    now.InitWithEpoch32Time(time);
//	    Serial.printf("%02u:%02u:%02u\n", now.Hour(),now.Minute(),now.Second());
//	    matrix.fillScreen(LOW);
//
//		matrix.setCursor(0,0);
//		matrix.printf("%02u:%02u", now.Hour(),now.Minute());
//		matrix.write();
//	}

//	if(0==(period%(30*100))){
//		Serial.printf("LDR sensor %d , temperature %lf C \n",	ldr.get(),rtc.GetTemperature());
//	}


}
