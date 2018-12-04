

#if temp_humi_sensor == 1 || temp_humi_sensor == 2
	#include <DHTesp-master/DHTesp.h>	// Reference: https://github.com/beegee-tokyo/DHTesp
	DHTesp dht;
	hw_timer_t * timer;
	SemaphoreHandle_t syncSemaphore;
	void IRAM_ATTR onTimer() {
		xSemaphoreGiveFromISR(syncSemaphore, NULL);
	}
#elif temp_humi_sensor == 3 
	#include <Wire.h>
	#include "Adafruit_SHT31.h"
	Adafruit_SHT31 sht31 = Adafruit_SHT31();
	//SHT3X sht3x;
#endif // temp_humi_sensor == 1


//----- SENSOR CUSTOMIZATION start
int sensor_setup() {
#if (APP_ID == (APPTYPE_SYSTEM_BASE+1) || APP_ID==(APPTYPE_PUBLIC_BASE+12))
	#if temp_humi_sensor == 1 || temp_humi_sensor == 2
		syncSemaphore = xSemaphoreCreateBinary();
		#if 	temp_humi_sensor == 1
			dht.setup(15, DHTesp::DHT11);
		#elif temp_humi_sensor == 2
			dht.setup(15, DHTesp::DHT22);
		#endif // 	temp_humi_sensor ==1

		timer = timerBegin(0, 20, true);
		timerAttachInterrupt(timer, &onTimer, true);
		timerAlarmWrite(timer, 10000000, true);
		timerAlarmEnable(timer);  
	#elif temp_humi_sensor == 3
		if (!sht31.begin(0x44))
		{
			Serial.println("Couldn't find SHT31");
		}

	#endif

#endif
}

// init all sensor data to 0, maybe not necessary
void init_sensor_data() {

	int i;
	for (i = 0; i<SENSOR_CNT; i++)
	{
		sensorValue[i] = 0;
		strcpy(sensorType[i], "-");
	}
	/*
	strcpy(sensorType[0], "0");
	strcpy(sensorType[1], "1");
	strcpy(sensorType[2], "2");
	strcpy(sensorType[3], "3");
	strcpy(sensorType[4], "4");
	*/
	#if APP_ID == (APPTYPE_SYSTEM_BASE+1)
		strcpy(sensorType[SENSOR_ID_DUST], "d0");
		strcpy(sensorType[SENSOR_ID_DUST10], "d1");
		strcpy(sensorType[SENSOR_ID_TEMPERATURE], "t0");
		strcpy(sensorType[SENSOR_ID_HUMIDITY], "h0");

	#endif
}



// please customize the how to get the sensor data and store to sensorValue[]
int get_sensor_data() {
	// sensor 0-9: system sensor
	Serial.print("SensorValue(RecordID):");
	sensorValue[SENSOR_ID_RECORDID] = record_id;
	Serial.println(sensorValue[SENSOR_ID_RECORDID]);

	//sensor 10-19: user sensor
	#if (APP_ID == (APPTYPE_SYSTEM_BASE+1) || APP_ID == (APPTYPE_PUBLIC_BASE+12))
		//Debug Time Count
		Serial.print("[Performence TIME-COUNT]:");
		timecount = millis() - timecount;
		Serial.println(timecount);
		timecount = millis();
		//Debug Time Count
		#if (PM25_sensor == 1 || PM25_sensor == 3 || PM25_sensor == 4)
			sensorValue[SENSOR_ID_DUST] = (float)pm25sensorG1();
			Serial.print("[SENSOR-DUST-PM2.5]:");
			Serial.println(sensorValue[SENSOR_ID_DUST]);
		#endif
		/*#ifdef USE_PM25_A4	//TODO
			sensorValue[SENSOR_ID_DUST] = (float)pm25sensorA4();
			Serial.print("[SENSOR-DUST-PM2.5]:");
			Serial.println(sensorValue[SENSOR_ID_DUST]);
		#endif*/

		//in-code assign value
		Serial.print("[SENSOR-DUST-PM10]:");
		Serial.println(sensorValue[SENSOR_ID_DUST10]);

		#if temp_humi_sensor == 1  || temp_humi_sensor == 2

			xSemaphoreTake(syncSemaphore, portMAX_DELAY);

			dht.getTempAndHumidity();
			t = dht.values.temperature;
			h = dht.values.humidity;
		#elif temp_humi_sensor == 3
			t = sht31.readTemperature();
			h = sht31.readHumidity();

		#endif

		/*#ifdef USE_SHT31	//TODO
			sht3x.readSample();
			t = sht3x.getTemperature();
			h = sht3x.getHumidity();
		#endif*/
		

		sensorValue[SENSOR_ID_TEMPERATURE] = t;
		Serial.print("SensorValue(Temperature):");
		Serial.println(sensorValue[SENSOR_ID_TEMPERATURE]);
		sensorValue[SENSOR_ID_HUMIDITY] = h;
		Serial.print("SensorValue(Humidity):");
		Serial.println(sensorValue[SENSOR_ID_HUMIDITY]);

		#if APP_ID==(APPTYPE_PUBLIC_BASE+12)
			get_sensor_data_mgs();
		#endif      

	#endif

	msg_sensor = "";
	int i;
	for (i = 0; i<SENSOR_CNT; i++)
	{
		if (sensorType[i][0] != '-') {
			msg_sensor.concat("|s_");
			msg_sensor.concat(sensorType[i]);
			msg_sensor.concat("=");
			msg_sensor.concat(sensorValue[i]);
		}
	}

	if (msg_sensor.length()<SENSOR_STRING_MAX) {
		msg_sensor.toCharArray(sensorUploadString, msg_sensor.length() + 1);
	}
	else {
		sensorUploadString[0] = 0;
		Serial.println("Sensor string overflow!");
	}
}


#if (PM25_sensor == 1 || PM25_sensor == 3 || PM25_sensor == 4)
int pm25sensorG1() {
	unsigned long timeout = millis();
	int count = 0;
	byte incomeByte[32];
	boolean startcount = false;
	byte data;
	Serial2.begin(9600);
	while (1) {
		if ((millis() - timeout) > 3000) {
			Serial.println("[G1-ERROR-TIMEOUT]");
			//#TODO:make device fail alarm message here
			break;
		}
		if (Serial2.available()) {
			data = Serial2.read();
			if (data == 0x42 && !startcount) {
				startcount = true;
				count++;
				incomeByte[0] = data;
			}
			else if (startcount) {
				count++;
				incomeByte[count - 1] = data;
				if (count >= 32) { break; }
			}
		}
	}
	Serial2.end();
	Serial2.flush();
	unsigned int calcsum = 0; // BM
	unsigned int exptsum;
	for (int i = 0; i < 30; i++) {
		calcsum += (unsigned int)incomeByte[i];
	}

	exptsum = ((unsigned int)incomeByte[30] << 8) + (unsigned int)incomeByte[31];
	if (calcsum == exptsum) {
		count = ((unsigned int)incomeByte[12] << 8) + (unsigned int)incomeByte[13];

		//PM10
		sensorValue[SENSOR_ID_DUST10] = ((unsigned int)incomeByte[14] << 8) + (unsigned int)incomeByte[15];

		#if  PM25_sensor == 4
			//Temp&Humi 
			t = (((unsigned int)incomeByte[24] << 8) + (unsigned int)incomeByte[25]) * 0.1;
			h = (((unsigned int)incomeByte[26] << 8) + (unsigned int)incomeByte[27]) * 0.1;
		#endif

		return count;
	}
	else {
		Serial.println("#[exception] PM2.5 Sensor CHECKSUM ERROR!");
		sensorValue[SENSOR_ID_DUST10] = -1;
		return -1;
	}
}
#elif PM25_sensor == 2
	int pm25sensorG3() {
		unsigned long timeout = millis();
		int count = 0;
		byte incomeByte[24];
		boolean startcount = false;
		byte data;
		Serial2.begin(9600);
		while (1) {
			if ((millis() - timeout) > 3000) {
				Serial.println("[G3-ERROR-TIMEOUT]");
				//#TODO:make device fail alarm message here
				break;
			}
			if (Serial2.available()) {
				data = Serial2.read();
				if (data == 0x42 && !startcount) {
					startcount = true;
					count++;
					incomeByte[0] = data;
				}
				else if (startcount) {
					count++;
					incomeByte[count - 1] = data;
					if (count >= 24) { break; }
				}
			}
		}
		Serial2.end();
		Serial2.flush();
		unsigned int calcsum = 0; // BM
		unsigned int exptsum;
		for (int i = 0; i < 22; i++) {
			calcsum += (unsigned int)incomeByte[i];
		}

		exptsum = ((unsigned int)incomeByte[22] << 8) + (unsigned int)incomeByte[23];
		if (calcsum == exptsum) {
			count = ((unsigned int)incomeByte[12] << 8) + (unsigned int)incomeByte[13];

			//PM10
			sensorValue[SENSOR_ID_DUST10] = ((unsigned int)incomeByte[14] << 8) + (unsigned int)incomeByte[15];

			return count;
		}
		else {
			Serial.println("#[exception] PM2.5 Sensor CHECKSUM ERROR!");
			return -1;
		}
	}


#endif



