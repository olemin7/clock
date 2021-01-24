/*
 * CMQTT.cpp
 *
 *  Created on: 29 ????. 2017 ?.
 *      Author: User
 */

#include "CMQTT.h"
#include "logs.h"

void mqtt_callback(char *topic, byte *payload, unsigned int length) {
    Serial.print("Message arrived [");
    Serial.print(topic);
    Serial.print("] ");
    for (unsigned int i = 0; i < length; i++) {
        Serial.print((char) payload[i]);
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

CMQTT::CMQTT() :
        client(espClient), mClientID("ESP8266Client" __DATE__ __TIME__) {
}

void CMQTT::setup(const char *domain, uint16_t port, const char *aClientID) {
    DBG_OUT << "MQTT Server:" << domain << ", port:" << std::dec << port << ", ClientID:" << aClientID << std::endl;
    mClientID = aClientID;
    client.setServer(domain, port);
    client.setCallback(mqtt_callback);
}

void CMQTT::loop() {
    if (!client.connected()) {
        reconnect();
        return;
    }
    client.loop();
}
void CMQTT::reconnect() {
    if (client.connected())
        return;
    const long now = millis();
    if (now < reconnectTimeOut) // time out is not passed yet
        return;
    DBG_OUT << "Attempting MQTT connection..." << std::endl;
    // Attempt to connect
    if (client.connect(mClientID)) {
        DBG_OUT << "connected" << std::endl;
        // Once connected, publish an announcement...
        client.publish("general", "connected");
        // ... and resubscribe
        //client.subscribe("inTopic");
    } else {
        DBG_OUT << "failed, rc=" << client.state() << std::endl;
    }
    reconnectTimeOut = now + recconectTimeOut;
}
bool CMQTT::publish(const String &topic, const String &message) {
    return client.publish(topic.c_str(), message.c_str());
}

