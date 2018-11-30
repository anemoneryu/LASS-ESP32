
#include "config.h"
#include "ESPinternet.h"
#include "sensor.h"
#include <pthread.h>
#include "log_function.h"


#define VER_FORMAT "3"  // version number has been increased to 2 since v0.7.0
#define VER_APP "0.2"
#define CLEAR_SETTING 0  //modify this to 1 if you can't change Setting in Flash
#define APP_NAME "PM25"


// time counter
unsigned long currentTime = 0;  // current loop time
unsigned long LastPostTime = 0; // last upload time
unsigned long lastWifiReadyTime = 0; // last wifi ready time | the time be checked

//----- GPS -----
//gpsSentenceInfoStruct info;
char buff_tmp[128]; //buffer
char utcstr[32]; //buffer
char datestr[32]; //buffer
double ground_speed;

/// new variables starting from v0.7.0
//char str_GPS_location[60];
char str_GPS_lat[15];
char str_GPS_lon[15];
char str_GPS_quality[5];
char str_GPS_satellite[5];
char str_GPS_altitude[10];

enum info_type {
  INFO_GPS = 1,
  INFO_MQTT = 2,
  INFO_LOGFILE = 3
};


// The setup() function runs once each time the micro-controller starts
void setup()
{
  
  Serial.begin(115200);
  Serial.println("LASS ESP32 PM2.5 Detector System starting....");

  // delete old config
  WiFi.disconnect(true);

  delay(1000);

  // Examples of different ways to register wifi events
  WiFiEventId_t eventID = WiFi.onEvent(WiFiEvent);

  if (!SPIFFS.begin(FORMAT_SPIFFS_IF_FAILED)) {
    Serial.println("SPIFFS Mount Failed");
    return;
  }
  listDir(SPIFFS, "/", 0);

  if (digitalRead(2) == 1)
  {
    deleteFile(SPIFFS, LOG_FILENAME);
  }

  sensor_setup();
  wifi_setup();
  setting_show();

  clientIDStr = DEVICE_ID;
  clientIDStr.toCharArray(clientID, clientIDStr.length() + 1);

  display_current_setting();
  
  init_sensor_data();

  if (FAKE_GPS == 1) {
    Serial.println("Using Fake GPS....Connect to Wifi now....");

    String wifi_pass = WIFI_PASS;
    wifi_pass.trim();
    while (checkWifiConnected() == 0) {
      Serial.println("Connect WIFI");
      WiFi.begin(WIFI_SSID, WIFI_PASS);
      delay(1000);
    }
    timeClient.begin();
    timeClient.setTimeOffset(0);
    while (!timeClient.update()) {
      timeClient.forceUpdate();
    }

    epochSystem = timeClient.getEpochTime();
    Serial.print("Seconds: ");
    Serial.println(epochSystem);
    lastWifiReadyTime = millis();
  }

  delay(3000);
  Serial.println("Setup complete! Looping main program");

}

void loop()
{
  currentTime = millis();
  Serial.print("\n-----Loop ID: ");
  Serial.print(record_id);
  Serial.print(", current tick= ");
  Serial.print(currentTime);

  Serial.println(" -----");

  packInfo(INFO_GPS);
  // Sensor
  get_sensor_data();

  // MQTT
  packInfo(INFO_MQTT);

  unsigned int need_save = 0;


  if (checkWifiConnected() == 0) {
    Serial.println("checkWifiConnected Fail!");
    wifiConnecting();
    if (WiFi.status() != WL_CONNECTED)
      need_save = 1;
    //wifi_ready = 0;
  }
  else {

    if (!mqttClient.connected()) {
      Serial.println("Reconnecting to MQTT Proxy");

      bConnected = mqttClient.connect(clientID);
      if (bConnected == false) {
        Serial.println("Reconnecting to MQTT Proxy: Fail!");
        need_save = 1;
      }
      //mqttSubscribeRoutine();
    }

    if (!mqttClient.connected()) {
      wifi_ready = 0;
      need_save = 1;
      Serial.println("MQTT send fail!");
    }
    else {
      mqttPrintCurrentMsg();
      mqttPublishRoutine(1);
      //mqttClient.disconnect();
      LastPostTime = currentTime;
      //Serial.print("logRecordCnt:");
      //Serial.println(logRecordCnt);
      // example:
      // Sensors/DustSensor |device_id=LASD-wuulong|time=20645|device=LinkItONE|values=0|gps=$GPGGA,235959.000,2448.0338,N,12059.5733,E,0,0,,160.1,M,15.0,M,,*4F

      if (logRecordCnt > 0 || logChecked ==1) {
        logSend();
        logChecked = 0;
      }
    }

  }


  if  (need_save == 1) {
    // Offline log
    packInfo(INFO_LOGFILE);

    Serial.println("Saving to file");
    // open the file. note that only one file can be open at a time, so you have to close this one before opening another.
    //LFile dataFile = Drv.open(LOG_FILENAME, FILE_WRITE);
    //Serial.println(dataString);
    appendFile(SPIFFS, LOG_FILENAME, dataString.c_str());
    logRecordCnt++;
    File dataLog = SPIFFS.open(LOG_FILENAME);
    Serial.print("File size: ");
    Serial.println(dataLog.size());
    dataLog.close();
    //readFile(SPIFFS, LOG_FILENAME);
    // if the file is available, write to it:
    /*if (dataFile) {
      Serial.println(dataString);
      dataFile.print(dataString); // record not include \n
      dataFile.close();
      logRecordCnt++;
    }
    // if the file isn't open, pop up an error:
    else {
      Serial.println("error opening log file");
    }*/
  }

  int usedTime = millis() - currentTime;
  signed long delayTime = (period_target[current_power_policy][PERIOD_SENSING_IDX] * 1000) - usedTime + DELAY_SYS_EARLY_WAKEUP_MS;

  record_id++;

  if (delayTime > 0) {
    delay(delayTime);
  }
}


// show current setting from setting memory
void setting_show() {

  Serial.println("-------------------- User Setting Show --------------------");
  Serial.print("SET_VERSION=");
  Serial.println(SETTING_VERSION);

  Serial.print("SSID=");
  Serial.print(WIFI_SSID);
  Serial.print(",WIFI_PASS=");
  Serial.println(WIFI_PASS);
  //Serial.print(",WIFI_AUTH=");
  //Serial.println(setting.wifi_auth);

  Serial.print("DeviceType=");
  Serial.print(DEVICE_TYPE);
  Serial.print(", DeviceID=");
  Serial.print(DEVICE_ID);
  Serial.print(", MqttTopicPrefix=");
  Serial.println(MQTT_TOPIC_PREFIX);

}


