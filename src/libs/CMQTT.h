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
    const char *mClientID;
    void reconnect();
    public:
    CMQTT();
    void setup(const char *domain, uint16_t port, const char *aClientID = nullptr);
    void loop();
    bool publish(const String &topic, const String &message);
};

#endif /* CLOCK_CMQTT_H_ */
