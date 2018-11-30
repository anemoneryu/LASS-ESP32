
void wifi_setup() {
	Serial.println("\n\n===== Setup wifi : =====");
	Serial.print("Connecting to ");
	Serial.println(WIFI_SSID);

	WiFi.begin(WIFI_SSID, WIFI_PASS);
	while (WiFi.status() != WL_CONNECTED) {
		delay(500);
		Serial.print(".");
	}
}

void wifiConnecting() {
	Serial.println("wifi Connecting");
	Serial.print("Connecting to ");
	Serial.println(WIFI_SSID);

	WiFi.begin(WIFI_SSID, WIFI_PASS);
	if (WiFi.status() != WL_CONNECTED) {
		delay(50);
		Serial.print(".");
		if (millis() > 1000) {
			Serial.println("time out.");
		}
	}
}

int checkWifiConnected() {

	if (wifi_event == SYSTEM_EVENT_STA_GOT_IP) {
		return 1;
	}
	else {
		return 0;
	}
}

void WiFiEvent(WiFiEvent_t event)
{
	//Serial.printf("[WiFi-event] event: %d\n", event);
	wifi_event = event;
	switch (event) {
	case SYSTEM_EVENT_WIFI_READY:
		//Serial.println("WiFi interface ready");
		break;
	case SYSTEM_EVENT_SCAN_DONE:
		//Serial.println("Completed scan for access points");
		break;
	case SYSTEM_EVENT_STA_START:
		//Serial.println("WiFi client started");
		break;
	case SYSTEM_EVENT_STA_STOP:
		//Serial.println("WiFi clients stopped");
		break;
	case SYSTEM_EVENT_STA_CONNECTED:
		//Serial.println("Connected to access point");
		break;
	case SYSTEM_EVENT_STA_DISCONNECTED:
		//Serial.println("Disconnected from WiFi access point");
		break;
	case SYSTEM_EVENT_STA_AUTHMODE_CHANGE:
		//Serial.println("Authentication mode of access point has changed");
		break;
	case SYSTEM_EVENT_STA_GOT_IP:
		//Serial.print("Obtained IP address: ");
		//Serial.println(WiFi.localIP());
		break;
	case SYSTEM_EVENT_STA_LOST_IP:
		//Serial.println("Lost IP address and IP address is reset to 0");
		break;
	case SYSTEM_EVENT_STA_WPS_ER_SUCCESS:
		//Serial.println("WiFi Protected Setup (WPS): succeeded in enrollee mode");
		break;
	case SYSTEM_EVENT_STA_WPS_ER_FAILED:
		//Serial.println("WiFi Protected Setup (WPS): failed in enrollee mode");
		break;
	case SYSTEM_EVENT_STA_WPS_ER_TIMEOUT:
		//Serial.println("WiFi Protected Setup (WPS): timeout in enrollee mode");
		break;
	case SYSTEM_EVENT_STA_WPS_ER_PIN:
		//Serial.println("WiFi Protected Setup (WPS): pin code in enrollee mode");
		break;
	case SYSTEM_EVENT_AP_START:
		//Serial.println("WiFi access point started");
		break;
	case SYSTEM_EVENT_AP_STOP:
		//Serial.println("WiFi access point  stopped");
		break;
	case SYSTEM_EVENT_AP_STACONNECTED:
		//Serial.println("Client connected");
		break;
	case SYSTEM_EVENT_AP_STADISCONNECTED:
		//Serial.println("Client disconnected");
		break;
	case SYSTEM_EVENT_AP_STAIPASSIGNED:
		//Serial.println("Assigned IP address to client");
		break;
	case SYSTEM_EVENT_AP_PROBEREQRECVED:
		//Serial.println("Received probe request");
		break;
	case SYSTEM_EVENT_GOT_IP6:
		//Serial.println("IPv6 is preferred");
		break;
	case SYSTEM_EVENT_ETH_START:
		//Serial.println("Ethernet started");
		break;
	case SYSTEM_EVENT_ETH_STOP:
		//Serial.println("Ethernet stopped");
		break;
	case SYSTEM_EVENT_ETH_CONNECTED:
		//Serial.println("Ethernet connected");
		break;
	case SYSTEM_EVENT_ETH_DISCONNECTED:
		//Serial.println("Ethernet disconnected");
		break;
	case SYSTEM_EVENT_ETH_GOT_IP:
		//Serial.println("Obtained IP address");
		break;
	}
}

void getCurrentTime(unsigned long epoch, int *year, int *month, int *day, int *hour, int *minute, int *second) {
	int tempDay = 0;

	*hour = (epoch % 86400L) / 3600;
	*minute = (epoch % 3600) / 60;
	*second = epoch % 60;

	*year = 1970; // epoch starts from 1970
	*month = 0;
	*day = epoch / 86400;

	for (*year = 1970; ; (*year)++) {
		tempDay += (LEAP_YEAR(*year) ? 366 : 365);
		if (tempDay > *day) {
			tempDay -= (LEAP_YEAR(*year) ? 366 : 365);
			break;
		}
	}
	tempDay = *day - tempDay; // the days left in a year
	for ((*month) = 0; (*month) < 12; (*month)++) {
		if ((*month) == 1) {
			tempDay -= (LEAP_YEAR(*year) ? 29 : 28);
			if (tempDay < 0) {
				tempDay += (LEAP_YEAR(*year) ? 29 : 28);
				break;
			}
		}
		else {
			tempDay -= monthDays[(*month)];
			if (tempDay < 0) {
				tempDay += monthDays[(*month)];
				break;
			}
		}
	}
	*day = tempDay + 1; // one for base 1, one for current day
	(*month)++;
}

// display current setting information to the console
void display_current_setting() {

	topicTmp = "";
	topicTmp.concat(MQTT_TOPIC_PREFIX);
	topicTmp.concat("/");
	topicTmp.concat(APP_NAME);
	topicTmp.toCharArray(mqttTopic, topicTmp.length() + 1);

	topicTmp = "";
	topicTmp.concat(mqttTopic);
	topicTmp.concat("/");
	topicTmp.concat(DEVICE_ID);
	topicTmp.toCharArray(mqttTopicSelf, topicTmp.length() + 1);

	topicTmp = "";
	topicTmp.concat(mqttTopic);
	topicTmp.concat("/");
	topicTmp.concat(PARTNER_ID);
	topicTmp.toCharArray(mqttTopicPartner, topicTmp.length() + 1);

	// General

	Serial.print("-------------------- LASS V");
	Serial.print(VER_APP);
	Serial.println(" -------------------------");
	Serial.println("User configuration");
	Serial.print("SSID=");
	Serial.print(WIFI_SSID);
	Serial.print(", MQTT_IP=");
	Serial.print(MQTT_PROXY_IP);
	Serial.print(", DeviceID=");
	Serial.print(DEVICE_ID);
	Serial.print(", PartnerID=");
	Serial.print(PARTNER_ID);
	Serial.print(", TOPIC=");
	Serial.print(mqttTopic);
	Serial.print(", TOPIC_SELF=");
	Serial.println(mqttTopicSelf);

}

// pack different type's information, print it for debug
void packInfo(int infoType) {


	switch (infoType) {
	case INFO_GPS:
		if (FAKE_GPS == 0) { // there is a GPS
							 /*		TODO
							 Serial.print("GPS raw data(GPRMC):");
							 Serial.print((char*)info.GPRMC);
							 Serial.print("GPS raw data(GPGGA):");
							 Serial.print((char*)info.GPGGA);
							 Serial.print("GPS raw data(GPVTG):");
							 Serial.print((char*)info.GPVTG);
							 parseGPGGA((const char*)info.GPGGA);
							 parseGPRMC((const char*)info.GPRMC);
							 parseGPVTG((const char*)info.GPVTG);
							 */
		}
		else {          // there are no GPS
						//parseGPGGA((const char*)GPS_FIX_INFOR);    //Dont Use AnyMore
			gps_ready = 1;
			strcpy(str_GPS_lat, gps_lat);
			strcpy(str_GPS_lon, gps_lon);
			strcpy(str_GPS_altitude, gps_alt);
			strcpy(str_GPS_quality, "1");
			strcpy(str_GPS_satellite, "9");

			unsigned long epoch = epochSystem + (millis() - lastWifiReadyTime) / 1000;
			int year, month, day, hour, minute, second;
			getCurrentTime(epoch, &year, &month, &day, &hour, &minute, &second);
			sprintf(datestr, "%02d-%02d-%02d", year, month, day);  // use the UTC format for datestr
			sprintf(utcstr, "%02d:%02d:%02d", hour, minute, second); // force each number to have two digits
																	 //Serial.println(datestr);
																	 //Serial.println(utcstr);
		}
		// parseGPGGA((const char*)info.GPGGA);
		break;

	case INFO_MQTT:
		// mqtt library limit the packet size = 200
		msg_tmp = "|ver_format=";
		msg_tmp.concat(VER_FORMAT);
		msg_tmp.concat("|FAKE_GPS=");
		msg_tmp.concat(FAKE_GPS);
		msg_tmp.concat("|app=");
		msg_tmp.concat(APP_NAME);
		msg_tmp.concat("|ver_app=");
		msg_tmp.concat(VER_APP);
		msg_tmp.concat("|device_id=");
		msg_tmp.concat(clientID);
		msg_tmp.concat("|tick=");
		msg_tmp.concat(currentTime);
		msg_tmp.concat("|date=");
		msg_tmp.concat(datestr);
		msg_tmp.concat("|time=");
		msg_tmp.concat(utcstr);
		msg_tmp.concat("|device=");
		msg_tmp.concat(DEVICE_TYPE);
		msg_tmp.concat(sensorUploadString);
		// v0.7.0, added for future integration with backend DB       
		msg_tmp.concat("|gps_lat=");
		msg_tmp.concat(str_GPS_lat);
		msg_tmp.concat("|gps_lon=");
		msg_tmp.concat(str_GPS_lon);
		msg_tmp.concat("|gps_fix=");
		msg_tmp.concat(str_GPS_quality);
		msg_tmp.concat("|gps_num=");
		msg_tmp.concat(str_GPS_satellite);
		msg_tmp.concat("|gps_alt=");
		msg_tmp.concat(str_GPS_altitude);
		msg_tmp.concat(" ");
		//msg_tmp.concat("|gpgga=");
		//msg_tmp.concat((char*)info.GPGGA);
		//msg_tmp.concat("|gpgsa=");
		//msg_tmp.concat((char*)info.GPGSA);
		//msg_tmp.concat("|gprmc=");
		//msg_tmp.concat((char*)info.GPRMC);
		//msg_tmp.concat("|gpvtg=");
		//msg_tmp.concat((char*)info.GPVTG);
		//msg_tmp.concat("|gpgsv=");
		//msg_tmp.concat((char*)info.GPGSV);
		//msg_tmp.concat("|glgsv=");
		//msg_tmp.concat((char*)info.GLGSV);
		//msg_tmp.concat("|glgsa=");
		//msg_tmp.concat((char*)info.GLGSA);
		//msg_tmp.concat("|bdgsv=");
		//msg_tmp.concat((char*)info.BDGSV);
		//msg_tmp.concat("|bdgsa=");
		//msg_tmp.concat((char*)info.BDGSA);
		//Serial.print("Msg length=");
		//Serial.println(msg_tmp.length());


		if (msg_tmp.length() < MSG_BUFFER_MAX - 1) {
			msg_tmp.toCharArray(msg, msg_tmp.length()); // the last char will be NULL, design to replace \n
		}
		else {
			msg[0] = 0;
			Serial.println("MSG buffer overflow! Length = " + msg_tmp.length());
		}

		break;
	case INFO_LOGFILE:
		// DATA format: @msg
		// Currently, only support one topic in the log
		dataString = "";
		dataString.concat("@");
		dataString.concat(msg);
		// print to the serial port too:
		//Serial.println("Pack Offline log:");
		//Serial.println(dataString);

		break;
	}
}

// callback to handle incomming MQTT messages
void msgCallback(char* topic, byte* payload, unsigned int len) {
#if ALARM_ENABLE == 1
	msgDisplay(topic, payload, len);

	// Central alarm
	stringTopicCmp = (char*)mqttTopicSelf;
	if (stringTopicCmp.compareTo(topic) == 0) {
		alarmHandlerCentral(payload, len);
	}

	// Partner alarm
	stringTopicCmp = (char*)mqttTopicPartner;
	if (stringTopicCmp.compareTo(topic) == 0) {
		alarmHandlerPartner(payload, len);
	}
#endif  

}

void mqttPrintCurrentMsg() {
	Serial.print("Pack MQTT Topic:");
	Serial.println(mqttTopic);
	Serial.println(msg);
	Serial.print("Msg size:");
	Serial.println(sizeof(msg));
}

void mqttPublishRoutine(int bPartner) {
	// debug the payload limit code
	//-- v0.7.10

	if (checkWifiConnected() != 1) {
		wifi_ready = 0;
		failedCounter++;
		Serial.println("Did not try MQTT PUBLISH because it's not connected....");
		Serial.println();
		return;
	}

	if (mqttClient.connected()) {
		mqttClient.publish((char*)mqttTopic, msg);
		if (bPartner) {
			mqttClient.publish((char*)mqttTopicSelf, msg);
			Serial.println("MQTT Companion channel published...");
		}
		delay(100);
		if (mqttClient.connected()) {
			failedCounter = 0;
		}
		else {
			failedCounter++;
			Serial.println("Connection to MQTT server failed (" + String(failedCounter, DEC) + ")");
		}
	}
	else {
		Serial.println("Connection to MQTT server failed (" + String(failedCounter, DEC) + ")");
		failedCounter++;
	}

#if ALARM_ENABLE == 1

#endif
	mqttClient.loop();
	Serial.println("MQTT sending");

}

