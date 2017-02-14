#include <SPI.h>
#include <Adafruit_GFX.h>
#include <Max72xxPanel.h>

#include <ESP8266WiFi.h>
#include <WiFiUdp.h>
#include "NTPtime.h"
#include "CLightDetectResistor.h"

#include "CRTCWraper.h"
#if 0
char ssid[] = "Guest1";  //  your network SSID (name)
char pass[] = "MH-6346PQMS";       // your network password
#else
char ssid[] = "ITPwifi";  //  your network SSID (name)
char pass[] = "_RESTRICTED3db@ea";       // your network password
#endif


int pinCS = 2; // Attach CS to this pin, DIN to MOSI and CLK to SCK (cf http://arduino.cc/en/Reference/SPI )
int numberOfHorizontalDisplays = 4;
int numberOfVerticalDisplays = 1;

Max72xxPanel matrix = Max72xxPanel(pinCS, numberOfHorizontalDisplays, numberOfVerticalDisplays);

String tape = "Arduino";
int wait = 20; // In milliseconds

int spacer = 1;
int width = 5 + spacer; // The font width is 5 pixels

CRTCWraper rtc;
NTPtime ntpTime;
CLightDetectResistor ldr;


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

  ntpTime.init();
  matrix.setTextColor(HIGH, LOW);
  matrix.setTextSize(1);
  matrix.print("test");
}
long period=0;
void loop() {
	period++;
	delay(10);
	if(0==(period%50)){//fast 0.5 sec
		int32 time;
		if(rtc.getTime(time))
	    {
	        // Common Cuases:
	        //    1) the battery on the device is low or even missing and the power line was disconnected
	        Serial.println("RTC lost confidence in the DateTime!");
	    }
	    RtcDateTime now;
	    now.InitWithEpoch32Time(time);
	    Serial.printf("%02u:%02u:%02u\n", now.Hour(),now.Minute(),now.Second());
	    matrix.fillScreen(LOW);

		matrix.setCursor(0,0);
		matrix.printf("%02u:%02u", now.Hour(),now.Minute());
		matrix.write();
	}

	if(1==(period%(30*100))){//10 sec
			int32 timeNtp ;
			 Serial.print("sending NTP packet...");
			if(0==ntpTime.getTime(timeNtp)){
				Serial.print("have answer");
				int32 rtcTime;
				rtc.getTime(rtcTime);
				if(abs(rtcTime-timeNtp)>5){//time delta is more 30 sec
					rtc.setTime(timeNtp);
					Serial.print("ntp time synk");
				}

			}
			Serial.println();
	}
	if(0==(period%(30*100))){
		Serial.printf("LDR sensor %d , temperature %lf C \n",	ldr.get(),rtc.GetTemperature());
	}


}
