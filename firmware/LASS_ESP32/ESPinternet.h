
// include Wifi library
#include <WiFiUdp.h>
#include <WiFiType.h>
#include <WiFiServer.h>
#include <WiFiScan.h>
#include <WiFiMulti.h>
#include <WiFiGeneric.h>
#include <WiFiClient.h>
#include <WiFiAP.h>
#include <ETH.h>
#include <WiFi.h>
#include <PubSubClient.h>



// include NTP library
#include "NTPClient.h"

// Use WiFiClient class to create TCP connections
WiFiClient client;

// variable 
int wifi_event = 0;
String clientIDStr;
int failedCounter = 0;

// packInfo msg
String msg_tmp;

// current setting information
String topicTmp = "";

// MQTT msg
String stringTopicCmp = "";


// Define NTP Client to get time
WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP);

// Variables to save date and time
String formattedDate;
String dayStamp;
String timeStamp;
uint32_t epochSystem;
static const uint8_t monthDays[] = { 31,28,31,30,31,30,31,31,30,31,30,31 }; // API starts months from 1, this array starts from 0


int current_power_policy = 0;
// First index is POLICY, [Sensing period],[Upload period],[Wifi check period], unit is second
int period_target[2][3] = 
{
#if ADJ_MODE == ADJ_MODE_FLY    
	5,5,300, // don't care power, fly mode
#else
	30,30,10, // don't care power, normal mode
#endif
	60,600,300  // power saving
};

//----- STATE -----
int wifi_ready = 0;
int gps_ready = 0; // 1: when satellites number>1, 0: when satellites==0 


 //----- MQTT -----
char mqttTopic[64];
char mqttTopicSelf[64];			// The topic used for central alarm
char mqttTopicPartner[64];		// The topic used for partner alarm
void msgCallback(char* topic, byte* payload, unsigned int len);
PubSubClient mqttClient((char*)MQTT_PROXY_IP, 1883, msgCallback, client);
char clientID[50];				//buffer
#define MSG_BUFFER_MAX 512
char msg[MSG_BUFFER_MAX];		//buffer

boolean bConnected;


