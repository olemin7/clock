/*
 * CMQTT.cpp
 *
 *  Created on: 29 ????. 2017 ?.
 *      Author: User
 */

#include "CMQTT.h"

void mqtt_callback(char* topic, byte* payload, unsigned int length) {
  Serial.print("Message arrived [");
  Serial.print(topic);
  Serial.print("] ");
    for (unsigned int i = 0; i < length; i++) {
    Serial.print((char)payload[i]);
  }
  Serial.println();

  // Switch on the LED if an 1 was received as first character
//  if ((char)payload[0] == '1') {
//    digitalWrite(BUILTIN_LED, LOW);   // Turn the LED on (Note that LOW is the voltage level
//    // but actually the LED is on; this is because
//    // it is acive low on the ESP-01)
//  } else {
//    digitalWrite(BUILTIN_LED, HIGH);  // Turn the LED off by making the voltage HIGH
//  }

}

CMQTT::CMQTT():client(espClient) {
    setClientID("ESP8266Client" __DATE__ __TIME__); //must be uniq for server
}


void CMQTT::setup(const char * domain, uint16_t port) {
    Serial.print("MQTT Server:");
    Serial.print(domain);
    Serial.print(" port:");
    Serial.println(port);
    client.setServer(domain, port);
    client.setCallback(mqtt_callback);
}

void CMQTT::loop(){
    if (!client.connected()) {
        reconnect();
        return;
    }
    client.loop();
}
void CMQTT::reconnect() {
   if(client.connected())
	   return;
   const long now = millis();
   if(now<reconnectTimeOut)// time out is not passed yet
	   return;
    Serial.print("Attempting MQTT connection...");
    // Attempt to connect
    if (client.connect(mClientID)) {
      Serial.println("connected");
      // Once connected, publish an announcement...
      //client.publish("outTopic", "hello world");
      // ... and resubscribe
      //client.subscribe("inTopic");
    } else {
      Serial.print("failed, rc=");
      Serial.println(client.state());
    }
    reconnectTimeOut=now+recconectTimeOut;
}
bool CMQTT::publish(const String &topic, const String &message){
	return client.publish(topic.c_str(), message.c_str());
}


