/*
 * Cconfig.cpp
 *
 *  Created on: Jan 6, 2021
 *      Author: ominenko
 */

#include "Cconfig.h"
#include <ArduinoJson.h>

Cconfig config;

void Cconfig::begin() {
	// TODO Auto-generated constructor stub
	  StaticJsonDocument<200> doc;
	  char json[] =
	      "{\"sensor\":\"gps\",\"time\":1351824120,\"data\":[48.756080,2.302038]}";

	  // Deserialize the JSON document
	  DeserializationError error = deserializeJson(doc, json);

	  // Test if parsing succeeds.
	  if (error) {
	    Serial.print(F("deserializeJson() failed: "));
	    Serial.println(error.f_str());
	    return;
	  }
}


