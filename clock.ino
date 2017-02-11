#include <SPI.h>
#include <Adafruit_GFX.h>
#include <Max72xxPanel.h>

#include <ESP8266WiFi.h>
#include <WiFiUdp.h>
#include <pgmspace.h>
#include "NTPtime.h"
#include "CLightDetectResistor.h"

#include <Wire.h> // must be included here so that Arduino library object file references work
#include <RtcDS3231.h>
RtcDS3231<TwoWire> Rtc(Wire);


char ssid[] = "Guest1";  //  your network SSID (name)
char pass[] = "MH-6346PQMS";       // your network password


int pinCS = 2; // Attach CS to this pin, DIN to MOSI and CLK to SCK (cf http://arduino.cc/en/Reference/SPI )
int numberOfHorizontalDisplays = 4;
int numberOfVerticalDisplays = 1;

Max72xxPanel matrix = Max72xxPanel(pinCS, numberOfHorizontalDisplays, numberOfVerticalDisplays);

String tape = "Arduino";
int wait = 20; // In milliseconds

int spacer = 1;
int width = 5 + spacer; // The font width is 5 pixels

NTPtime ntpTime;
CLightDetectResistor ldr;
void rtc_setup(){
//--------RTC SETUP ------------
Rtc.Begin();

// if you are using ESP-01 then uncomment the line below to reset the pins to
// the available pins for SDA, SCL
// Wire.begin(0, 2); // due to limited pins, use pin 0 and 2 for SDA, SCL

RtcDateTime compiled = RtcDateTime(__DATE__, __TIME__);
printDateTime(compiled);
Serial.println();

if (!Rtc.IsDateTimeValid())
{
    // Common Cuases:
    //    1) first time you ran and the device wasn't running yet
    //    2) the battery on the device is low or even missing

    Serial.println("RTC lost confidence in the DateTime!");

    // following line sets the RTC to the date & time this sketch was compiled
    // it will also reset the valid flag internally unless the Rtc device is
    // having an issue

    Rtc.SetDateTime(compiled);
}

if (!Rtc.GetIsRunning())
{
    Serial.println("RTC was not actively running, starting now");
    Rtc.SetIsRunning(true);
}

RtcDateTime now = Rtc.GetDateTime();
if (now < compiled)
{
    Serial.println("RTC is older than compile time!  (Updating DateTime)");
    Rtc.SetDateTime(compiled);
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
Rtc.Enable32kHzPin(false);
Rtc.SetSquareWavePin(DS3231SquareWavePin_ModeNone);
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
  RtcDateTime compiled = RtcDateTime(__DATE__, __TIME__);
	printDateTime(compiled);
	Serial.println();
	rtc_setup();

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

void loop() {
	Serial.printf("LDR sensor %d \n",	ldr.get());

    if (!Rtc.IsDateTimeValid())
    {
        // Common Cuases:
        //    1) the battery on the device is low or even missing and the power line was disconnected
        Serial.println("RTC lost confidence in the DateTime!");
    }

    RtcDateTime now = Rtc.GetDateTime();
    printDateTime(now);
    Serial.println();

    RtcTemperature temp = Rtc.GetTemperature();
    Serial.print(temp.AsFloat());
    Serial.println("C");

	unsigned long time =ntpTime.getTime();
	if(time){
		matrix.fillScreen(LOW);

		matrix.setCursor(0,0);
		unsigned char sec=time%60;
		Serial.println(sec);
		matrix.print(sec);
		matrix.write();
	}
	delay(500);
}
#define countof(a) (sizeof(a) / sizeof(a[0]))
void printDateTime(const RtcDateTime& dt)
{
    char datestring[20];

    snprintf_P(datestring,
            countof(datestring),
            PSTR("%02u/%02u/%04u %02u:%02u:%02u"),
            dt.Month(),
            dt.Day(),
            dt.Year(),
            dt.Hour(),
            dt.Minute(),
            dt.Second() );
    Serial.print(datestring);
}
