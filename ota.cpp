/*
 * ota.cpp
 *
 *  Created on: 22 ????. 2017 ?.
 *      Author: User
 */
#include "ota.h"
int ota_progress;
void ota_begin(const char *hostname,const char *pwd) {

  Serial.println("ArduinoOTA.begin()");
  // Port defaults to 8266
  // ArduinoOTA.setPort(8266);

  // Hostname defaults to esp8266-[ChipID]
  ArduinoOTA.setHostname(hostname);

  // No authentication by default
//  ArduinoOTA.setPassword(pwd);
  Serial.print("hostname=");
  Serial.print(hostname);
  Serial.print(" pwd=");
  Serial.println(pwd);

  ArduinoOTA.onStart([]() {
	ota_progress=0;
    Serial.println("Start\n");
  });
  ArduinoOTA.onEnd([]() {
    Serial.println(" Done");
  });
  ArduinoOTA.onProgress([](unsigned int progress, unsigned int total) {
	int cur=progress*20/total;
	if(ota_progress!=cur){
		ota_progress=cur;
		Serial.print(".");
	}
  });
  ArduinoOTA.onError([](ota_error_t error) {
    Serial.printf("Error[%u]: ", error);
    if (error == OTA_AUTH_ERROR) Serial.println("Auth Failed");
    else if (error == OTA_BEGIN_ERROR) Serial.println("Begin Failed");
    else if (error == OTA_CONNECT_ERROR) Serial.println("Connect Failed");
    else if (error == OTA_RECEIVE_ERROR) Serial.println("Receive Failed");
    else if (error == OTA_END_ERROR) Serial.println("End Failed");
  });
  ArduinoOTA.begin();
  Serial.println("OTA Ready");
}

void ota_loop() {
  ArduinoOTA.handle();
}



