/*
 * CMQTT.h
 *
 *  Created on: 29 ????. 2017 ?.
 *      Author: User
 */

#ifndef CLOCK_CMQTT_H_
#define CLOCK_CMQTT_H_
#include <WiFiClient.h>
#include <PubSubClient.h>
#include <string.h>

class CMQTT {
protected:
    WiFiClient espClient;
    PubSubClient client;
    const static long recconectTimeOut = 15000;
    long reconnectTimeOut = 0;
    const char *m_ClientID;
    std::string m_cb_topic;
    void reconnect();
    public:
    CMQTT();
    void setup(const char *domain, uint16_t port, const char *aClientID = nullptr);
    void callback(std::string topic, MQTT_CALLBACK_SIGNATURE) {
        client.setCallback(callback);
        m_cb_topic = std::move(topic);
    }
    void loop();
    bool isConnected() {
        return client.connected();
    }
    bool publish(const std::string &topic, const std::string &message);
};

#endif /* CLOCK_CMQTT_H_ */
