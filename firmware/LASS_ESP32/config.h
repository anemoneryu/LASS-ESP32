//LASS CONFIGURATION FILE  
#include "Arduino.h"
#include "esp_wifi.h"

//Step 1: How you Connect WIFI....Basic things.
//WIFI
//System default wifi setting: SSID=LASS, PASS=LASS123456, WIFI_AUTH=LWIFI_WPA
// REPLACE: your network password (use for WPA, or use as key for WEP)

#define WIFI_SSID "LASS"			// REPLACE: your network SSID (name)
#define WIFI_PASS "LASS123456"
#define WIFI_AUTH LWIFI_WPA						//Default:LWIFI_WPA // choose from LWIFI_OPEN, LWIFI_WPA, or LWIFI_WEP.
//-----------------------------------------------------------

//Step 2:Do you use Blyak.If yes,fill info below.
//Blynk-IoT
#define BLYNK_ENABLE 0 // deafult(0) 0: If you don't need to support BLYNK, 1: support BLYNK 
char blynk_auth[] = "yourBlynkID"; // REPLACE: your Blynk auto id
								   //-----------------------------------------------------------

//Step 3:MQTT info
//MQTT-IoT
#define MQTT_PROXY_IP "gpssensor.ddns.net"		// Current LASD server , dont change!
#define DEVICE_TYPE  "ESP-WROOM-32"				// since there is only one device LASS supported now,dont change!
#define DEVICE_ID "FT1_ESP_999"					// REPLACE: The device ID you like, please start from LASD. Without this prefix, maybe it will be filter out.
#define MQTT_TOPIC_PREFIX "LASS/Test"			// CAN REPLACE if you like //Dont Replace IF YOU ARE FIELD-TRY USER
#define PARTNER_ID "Anemoneryu"				// CAN REPLACE if you like


//Step 4:GPS
//Do you want to use gps? 0:YES 1:FAKE GPS
#define FAKE_GPS 1						// FAKE_GPS : 0: default format with gps, 1: default format but gps is fix data, need to update GPS_FIX_INFOR 
										          //NOTICE:If you choose 1 modify "FAKE" GPS location. Fill info below
const char gps_lat[] = "23.000000";		// device's gps latitude
const char gps_lon[] = "120.000000";	// device's gps longitude

const char gps_alt[] = "30.0";          // device's gps altitude
#define GPS_SIGNAL_NOCHECK 1   // 0: log or send only when GPS have signal, 1: always log and send even when GPS have no signal 

										//NOTICE: for Field TRY-PM2.5 DONT CHANGE AFTER THIS LINE!   --2015/11/09
										//-----------------------------------------------------------
										//Step 5:About LASS 
#define APP_ID (APPTYPE_SYSTEM_BASE+1)               // REPLACE: this is your unique application 0-255: system reserved, 256-32767: user public use, 32768-65536: private purpose
#define APPTYPE_SYSTEM_BASE 0
#define APPTYPE_PUBLIC_BASE 256
#define APPTYPE_PRIVATE_BASE 32768
#define SERIAL_BAUDRATE 115200

//NOTICE: You are ready to ROCK, NO MORE TO SETUP!!!!
//-----------------------------------------------------------


//For ADVANCED user ONLY 
#define ALARM_ENABLE 0 // default(0) 0: disable alarm, 1: enable alarm
#if ALARM_ENABLE==1
#define BUZZER_ALARM_PIN 3
#endif


//----- USER SENSOR CONFIG -----

#define SENSOR_CNT 25          // REPLACE: the sensors count that publish to server.
#define SENSOR_STRING_MAX 300  

#define SENSOR_ID_RECORDID 0
#define SENSOR_ID_BATTERYLEVEL 1
#define SENSOR_ID_BATTERYCHARGING 2 //      battery is charging: (0) not charging, (1) charging
#define SENSOR_ID_GROUNDSPEED 3
#define SENSOR_ID_DEBUGWIFI 4

										//----- DEAFULT PIN DEFINE -----
enum pinConfig {
	ARDUINO_LED_PIN = 13,
	STORAGE_CHIP_SELECT_PIN = 10
};


//LASS's OPEN PM2.5 Field-TRY
#if APP_ID==(APPTYPE_SYSTEM_BASE+1)

	/*	define temperature and humidity sensor
	0 : undefine
	1 : DHT11
	2 : DHT22
	*/
	#define temp_humi_sensor 0

	/*	define PM2.5 sensor
	0 : PMS1003 G1
	1 : PMS3003 G3
	2 :
	*/
	#define PM25_sensor 1

	//#define USE_PM25_G3
	//#define USE_PM25_A4
	//#define USE_DHT22  // not recommend for DHT series sensors
	//#define USE_DHT11 
	//#define USE_SHT31
	//#define USE_LCD  // use 1602 i2c LCD...
	#define SENSOR_ID_DUST 10
	#define SENSOR_ID_TEMPERATURE 11
	#define SENSOR_ID_HUMIDITY 12  
	#define SENSOR_ID_DUST10 13
	#define SENSOR_ID_DUST_BLYNK 6
	#define SENSOR_ID_TEMPERATURE_BLYNK 7
	#define SENSOR_ID_HUMIDITY_BLYNK 8
	#define SENSOR_ID_DUST10_BLYNK 9

//LASS's start up project by wuloong

#endif 

//SYSTEM PARAMETERS

#define DELAY_SYS_EARLY_WAKEUP_MS 11

#define POLICY_ONLINE_ALWAYS 1
#define POLICY_ONLINE_LESS 2
#define POLICY_ONLINE_DEFAULT 0

#define POLICY_POWER_DONTCARE 0
#define POLICY_POWER_SAVE 1
#define POLICY_POWER_AUTO 2

#define LED_MODE_DEFAULT 0 // to show system status and behavior
#define LED_MODE_OFF 1 // To not disturbe the environment, never have LED on 

#define ADJ_MODE_NORMAL 0 // default system behavior, no special adjustment
#define ADJ_MODE_FLY 1    // for fly mode, speed up sensing, less wifi check 

#define SETTING_MODE_CODE // user setting from code
#define SETTING_MODE_FLASH // user setting from flash
#define SETTING_VERSION 2

#define PERIOD_SENSING_IDX 0
#define PERIOD_UPLOAD_IDX 1
#define PERIOD_WIFICHECK_IDX 2


#define POLICY_ONLINE POLICY_ONLINE_ALWAYS //1: POLICY_ONLINE_ALWAYS 2: POLICY_ONLINE_LESS                                            
#define POLICY_POWER  POLICY_POWER_DONTCARE //2: POLICY_POWER_AUTO(Auto power saving mode) 0: POLICY_POWER_DONTCARE 1: POLICY_POWER_SAVE
// policy auto check if not charging and battery lower than seting of battery level, switch to power saving mode.

#define POWER_POLICY_BATTERY_LEVEL 70 // When battery level lower than this, trigger power saving mode when power policy is AUTO
#define LED_MODE LED_MODE_DEFAULT  
#define ADJ_MODE ADJ_MODE_NORMAL // Some special adjustment for different scenario
#define SETTING_MODE SETTING_MODE_CODE

// The logic decide if we should do something
#define LOGIC_WIFI_NEED_CONNECT 1
#define LOGIC_MQTT_NEED_SEND 2
#define LOGIC_DATA_NEED_SAVETOFLASH 3
#define LOGIC_WHAT_LED_STATE 4
#define LOGIC_LOG_NEED_SEND 5

#define LED_STATE_OFF 0
#define LED_STATE_READY 1
#define LED_STATE_ERROR 2
