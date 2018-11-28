
#include "unsorted.h"
#include <ESP8266WiFi.h>

void setup_wifi(const char *ssid, const char *password, const char *aHostname =
        NULL) {
  // We start by connecting to a WiFi network
  Serial.println();
    WiFi.persistent(false);
    WiFi.mode(WIFI_STA);
    if (NULL != aHostname) {
        WiFi.hostname(aHostname);
        Serial.print("Hostname=");
        Serial.print(aHostname);
        Serial.print(", ");
    }
  Serial.print("Connecting to ");
  Serial.println(ssid);


  WiFi.begin(ssid, password);
}

void wifi_loop(){
	static wl_status_t wl_status = WL_IDLE_STATUS;
	if(wl_status!=WiFi.status()){
		wl_status=WiFi.status();
		Serial.printf("WiFi.status %d\n",wl_status);
		if(WL_CONNECTED==wl_status){
			  Serial.print("WiFi connected, IP address: ");
			  Serial.println(WiFi.localIP());
		}
	}
}

/*******************************************************************************
 *
 */
void sw_info(char const *fwName,Stream &Out){
	Out.printf("FW: %s\nDate: %s %s\n",fwName,__DATE__,__TIME__);
	Out.println("sw_info");
    Out.print("WiFi, Hostname=");
    Out.print(WiFi.hostname());
	Out.print("IP address: ");
	Out.println(WiFi.localIP());
	WiFi.printDiag(Out);
	Out.printf("FreeHeap:   %d\n", ESP.getFreeHeap());
}

void hw_info(Stream &Out){
	Out.println("hw_info");
    uint32_t realSize = ESP.getFlashChipRealSize();
    uint32_t ideSize = ESP.getFlashChipSize();
    FlashMode_t ideMode = ESP.getFlashChipMode();
    Out.println("Flash info");
    Out.printf("id:   %08X\n", ESP.getFlashChipId());
    Out.printf("size: %u\n", realSize);
    if(ideSize != realSize) {
    	Out.printf("\n!!Different size\nFlash IDE size: %u\n\n", ideSize);
    }
    Out.printf("ide speed: %u\n", ESP.getFlashChipSpeed());
    Out.printf("ide mode:  %s\n", (ideMode == FM_QIO ? "QIO" : ideMode == FM_QOUT ? "QOUT" : ideMode == FM_DIO ? "DIO" : ideMode == FM_DOUT ? "DOUT" : "UNKNOWN"));
    Out.println("end Flash info");


}

/*******************************************************************************
 *
 */
void wifiList(Stream &Out){
	Out.println("Wifi scaning...");
	 // WiFi.scanNetworks will return the number of networks found
	int n = WiFi.scanNetworks();
	for (int i = 0; i < n; ++i)
		Out.printf("- %s (%d)%s\n", WiFi.SSID(i).c_str(),WiFi.RSSI(i),(WiFi.encryptionType(i) == ENC_TYPE_NONE)?" ":"*");
	Out.printf("Scan is done. Networks %d\n",n);
}

//eof
