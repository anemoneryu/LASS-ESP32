
//----- SENSORS -----
char sensorType[SENSOR_CNT][3];
float sensorValue[SENSOR_CNT];
char sensorUploadString[SENSOR_STRING_MAX]; //buffer // Please extend this if you need

long record_id = 0;							
float h, t;		// The followings are used for DHT22 sensors
String msg_sensor;
unsigned long timecount;