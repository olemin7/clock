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
class CMQTT {
	WiFiClient espClient;
	PubSubClient client;
	const static long recconectTimeOut=5000;
	long reconnectTimeOut=0;
	void reconnect();
public:
	CMQTT(const char * domain, uint16_t port);
	void loop();
	bool publish(const String &topic, const String &message);
};

#endif /* CLOCK_CMQTT_H_ */
