/*
 * wifiHandle.cpp
 *
 *  30 dec-2020
 *      Author: ominenko
 */

#include "wifiHandle.h"
#include "logs.h"
#include "misk.h"
#include <sstream>
using namespace std;
void wifiHandle_sendlist(ESP8266WebServer &server){
    DBG_PRINTLN("wifiHandle_sendlist ");
    server.setContentLength(CONTENT_LENGTH_UNKNOWN);
    server.sendHeader("Content-Type", "application/json", true);
    server.sendHeader("Cache-Control", "no-cache");
    server.sendContent("{\"AP_LIST\":[");
	const auto count= WiFi.scanNetworks(false, true);
	DBG_PRINT("networks ");
	DBG_PRINTLN(count);

	for (int i = 0; i < count; i++)
	{
		  ostringstream line;
		  if(i){
			  line<<",";
		  }
		  line<<"{";
		  line<<"\"SSID\":\""<<WiFi.SSID(i).c_str()<<"\",";
		  line<<"\"SIGNAL\":\""<<WiFi.RSSI(i)<<"\",";
		  line<<"\"IS_PROTECTED\":\""<<((ENC_TYPE_NONE==WiFi.encryptionType(i))?0:1)<<"\",";
		  line<<"\"IS_HIDDEN\":\""<<WiFi.isHidden(i)<<"\"";
		  line<<"}";
		  DBG_PRINTLN(line.str().c_str());
		  server.sendContent(line.str().c_str());
	}
	server.sendContent("]}");
	server.sendContent("");
}
static bool showed=false;
void wifiHandle_loop(){
    if (showed) {
        return;
    }
    if (WIFI_STA == WiFi.getMode()) {
        if (WiFi.status() == WL_CONNECTED) {
            showed = true;
            Serial.print("M117 ");
            Serial.println(WiFi.localIP());
            DBG_PRINT(F( "IP address:"));
            DBG_PRINT(WiFi.localIP().toString());
            DBG_PRINT(F(" RSSI: "));
            DBG_PRINTLN(WiFi.RSSI());
        }
    }
}

void wifiHandle_connect(ESP8266WebServer &server){

	auto retVal = er_last;
	String ssid;
	String pwd="";
	do
	{
		if (!server.hasArg("ssid"))
		{
			retVal = er_no_parameters;
			break;
		}
		ssid = server.arg("ssid");
		if (server.hasArg("pwd"))
		{
			pwd = server.arg("pwd");
		}
		retVal=er_ok;
	}while(0);

	webRetResult(server,retVal);
	if(er_ok==retVal){
		delay(500);
	    DBG_PRINT(F("ssid:"));
	    DBG_PRINT(ssid);
	    DBG_PRINT(F(" pwd: "));
	    DBG_PRINTLN(pwd);
		WiFi.mode(WIFI_STA);
		WiFi.begin(ssid.c_str(), pwd.c_str());
	}
}

void setup_wifi(const String &ssid, const String &pwd, const String &host_name, const WiFiMode_t &wifi_mode)
{
    // Set hostname first
    DBG_PRINT(F("hostname:"));
    DBG_PRINTLN(host_name);
    DBG_PRINT(F("pwd: "));
    DBG_PRINTLN(pwd);
    DBG_PRINT(F("Mode: "));
    DBG_PRINTLN(WiFi.getPhyMode())
    WiFi.hostname(host_name);
    // Reduce startup surge current
    WiFi.setAutoConnect(true);
    WiFi.setAutoReconnect(true);
    WiFi.mode(wifi_mode);
    WiFi.setPhyMode(WIFI_PHY_MODE_11N);
    if (WIFI_STA == wifi_mode)
    {
        WiFi.begin(ssid.c_str(), pwd.c_str());
    } else
    {
        WiFi.mode(WIFI_AP);
        WiFi.softAP(host_name.c_str(), pwd.c_str());
    }
}

