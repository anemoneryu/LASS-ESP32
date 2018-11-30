
//=====================	SPIFFS	==============
void listDir(fs::FS &fs, const char * dirname, uint8_t levels) {
	Serial.printf("Listing directory: %s\r\n", dirname);

	File root = fs.open(dirname);
	if (!root) {
		Serial.println("- failed to open directory");
		return;
	}
	if (!root.isDirectory()) {
		Serial.println(" - not a directory");
		return;
	}

	File file = root.openNextFile();
	while (file) {
		if (file.isDirectory()) {
			Serial.print("  DIR : ");
			Serial.println(file.name());
			if (levels) {
				listDir(fs, file.name(), levels - 1);
			}
		}
		else {
			Serial.print("  FILE: ");
			String fileName = file.name();
			int fileSize = file.size();
			Serial.print(fileName);
			Serial.print("\tSIZE: ");
			Serial.println(fileSize);
			if (fileName == LOG_FILENAME)
				if (fileSize > 10)
					logChecked = 1;
		}
		file = root.openNextFile();
	}
}

void readFile(fs::FS &fs, const char * path) {
	Serial.printf("Reading file: %s\r\n", path);

	File file = fs.open(path);
	if (!file || file.isDirectory()) {
		Serial.println("- failed to open file for reading");
		return;
	}

	Serial.println("- read from file:");
	while (file.available()) {
		Serial.write(file.read());
	}
}

void writeFile(fs::FS &fs, const char * path, const char * message) {
	Serial.printf("Writing file: %s\r\n", path);

	File file = fs.open(path, FILE_WRITE);
	if (!file) {
		Serial.println("- failed to open file for writing");
		return;
	}
	if (file.print(message)) {
		Serial.println("- file written");
	}
	else {
		Serial.println("- frite failed");
	}
}

void appendFile(fs::FS &fs, const char * path, const char * message) {
	Serial.printf("Appending to file: %s\r\n", path);

	File file = fs.open(path, FILE_APPEND);
	if (!file) {
		Serial.println("- failed to open file for appending");
		return;
	}
	if (file.print(message)) {
		Serial.println("- message appended");
	}
	else {
		Serial.println("- append failed");
	}
}

void renameFile(fs::FS &fs, const char * path1, const char * path2) {
	Serial.printf("Renaming file %s to %s\r\n", path1, path2);
	if (fs.rename(path1, path2)) {
		Serial.println("- file renamed");
	}
	else {
		Serial.println("- rename failed");
	}
}

void deleteFile(fs::FS &fs, const char * path) {
	Serial.printf("Deleting file: %s\r\n", path);
	if (fs.remove(path)) {
		Serial.println("- file deleted");
	}
	else {
		Serial.println("- delete failed");
	}
}

//=====================	SPIFFS END	==============


// if data logged, send it out and delete it.
void logSend() {
	int dotCnt = 0;
	boolean bConnected;
	int upload_fail = 0;

	//Drv.remove((char*)LOG_FILENAME); // for debug
	// upload log only when wifi ready
	/*
	if (Drv.exists((char*)LOG_FILENAME)) {
	Serial.println("Log exist! Send logging sensor records!");
	if (!mqttClient.connected()) {
	Serial.println("Reconnecting to MQTT Proxy");
	bConnected = mqttClient.connect(clientID);
	if (bConnected == false) {
	Serial.println("Reconnecting to MQTT Proxy: Fail!");
	}
	//mqttSubscribeRoutine();
	}*/

	// if log exist, send

	// re-open the file for reading:


	File myFile = SPIFFS.open(LOG_FILENAME);
	if (myFile) {
		Serial.println("Log exist! Send logging sensor records!");
		Serial.print("Open file : ");
		Serial.println(LOG_FILENAME);
		myFile.seek(0);
		Record = "@";
		// read from the file until there's nothing else in it:
		while (myFile.available()) {
			char c = myFile.read();
			if (c == '@') {
				if (Record != "@") {
					Record.toCharArray(msg, Record.length() + 1);
					Serial.println(msg);
					if (checkWifiConnected() == 1) {
						mqttPublishRoutine(0);

						LastPostTime = currentTime;

						//mqttClient.publish(mqttTopic, msg);
						//sent the same msg to partner which may monitor this topic, current work around
						//these delay msg may cause problem to partner, not send it now.
						//mqttClient.publish(mqttTopicSelf, msg);
						Serial.print(".");
						dotCnt++;
						if ((dotCnt % 1) == 0) {
							Serial.println(".");
						}
					}
					else {
						upload_fail = 1;
						break;
					}

				}
				Record = "";
			}
			else Record.concat(c);
			//Serial.write(c);
			//Serial.write("!");
		}
		//mqttClient.disconnect();
		// close the file:
		myFile.close();
		if (upload_fail == 0) {
			SPIFFS.remove(LOG_FILENAME);
			//Drv.remove((char*)LOG_FILENAME);
			Serial.println("\nUpload complete, log file removed");
		}
		else {
			Serial.println("\nUpload fail, log file not removed");

		}
		logRecordCnt = 0;
	} // if (myFile) 
	  // if(Drv.exists((char*)LOG_FILENAME))

}


