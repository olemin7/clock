/*
 * CMQTT.cpp
 *
 *  Created on: 29 ????. 2017 ?.
 *      Author: User
 */

#include "CMQTT.h"
#include "logs.h"
#include "wifiHandle.h"
#include <sstream>
using namespace std;

CMQTT::CMQTT() :
        client(espClient), m_ClientID("ESP8266Client" __DATE__ __TIME__) {
}

void CMQTT::setup(const char *domain, uint16_t port, const char *aClientID) {
    DBG_OUT << "MQTT Server:" << domain << ", port:" << std::dec << port << ", ClientID:" << aClientID << std::endl;
    m_ClientID = aClientID;
    client.setServer(domain, port);
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
    if (client.connect(m_ClientID)) {
        DBG_OUT << "connected" << std::endl;
        ostringstream line;
        line << "{\"name\":" << m_ClientID << ",\"ip\":" << WiFi.localIP() << "}";
        publish("connected", line.str());
        // ... and resubscribe
        client.subscribe(m_cb_topic.c_str());
    } else {
        DBG_OUT << "failed, rc=" << client.state() << std::endl;
    }
    reconnectTimeOut = now + recconectTimeOut;
}
bool CMQTT::publish(const string &topic, const string &message) {
    return client.publish(topic.c_str(), message.c_str());
}

