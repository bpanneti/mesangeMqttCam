boolean load_wifi() {
  File config_file = SPIFFS.open("/wifi.json", "r");
  if (!config_file) {
    Serial.println("in load Wifi : Failed to open file for reading");
    return false;
  }
String  config_string = config_file.readString();

  DeserializationError error = deserializeJson(WIFI, config_string);
  if (error) {
    Serial.print(F("deserializeJson() failed: "));
    Serial.println(error.c_str());
    return false;
  }
  config_file.close();
 
  Serial.println((const char*)WIFI["SSID"]);
  ssid =  (const char*)WIFI["SSID"];
  password =  (const char*)WIFI["Pass"];
  mqtt_server = (const char*)WIFI["MQTT_Server"];
  mqtt_user = (const char*)WIFI["MQTT_User"];
  mqtt_pass = (const char*)WIFI["MQTT_Pass"];
  topic_PHOTO =(const char*)WIFI["Topic_PHOTO"];
  topic_CONFIG = (const char*)WIFI["Topic_CONFIG"];
  topic_UP = (const char*)WIFI["Topic_UP"];
  HostName =(const char*)WIFI["HostName"];
  //IP_Address =(const char*)WIFI["IPAddress"];
  
  Serial.println("WiFi JSON loaded");

  return true;
}

void save_wifi() {
  String config_string;
  //Open the config.json file (Write Mode)
  File config_file = SPIFFS.open("/wifi.json", "w");
  if (!config_file) {
    Serial.println("Failed to open file (Writing mode)");
    return;
  }
  serializeJson(WIFI, config_string);
  //Save and close the JSON file
  if (config_file.println(config_string)) {
    Serial.println("New Config");
    Serial.println(config_string);
  } else {
    Serial.println("File write failed");
  }
  config_file.close();
}

boolean setup_wifi() {
  Serial.print("Connecting to ");
  Serial.print(ssid);
  //WiFi.mode(WIFI_STA);
  WiFi.setHostname(HostName);
  WiFi.begin(ssid, password);
  int count = 0;
  while ( count < 300 ) {
//    uint8_t status = WiFi.waitForConnectResult();
//
//   String m = connectionStatusMessage(status);
//   Serial.print("Connection attempt %d: status is%s", count, m.c_str());

    if (WiFi.status() == WL_CONNECTED) {
      Serial.println("WiFi connected");
      Serial.println("IP address: ");
      Serial.println(WiFi.localIP());
      return (true);
    }
    delay(500);
    Serial.print(".");
    count++;
  }
  Serial.println("Timed out.");
  return false;
}


 
void reconnect() {
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    cmnd_lwt="tele/"+String(HostName)+"/LWT";
    send_photo="video/"+String(HostName)+"/RESULT";
    
    if (client.connect(HostName,cmnd_lwt.c_str(),0,true,"Offline")) {
      Serial.println("connected");
      client.publish(cmnd_lwt.c_str(), "Online",true);
      
      char str[80];
      strcpy (str,"esp/");
      strcat (str,HostName);
      client.publish(str, "hello world");
  
      cmnd_power="cmnd/"+String(HostName)+"/"+topic_PHOTO;
      client.subscribe(cmnd_power.c_str());


      cmnd_takePicture     ="cmnd/"+String(HostName)+"/TAKEPICTURE"; 
      client.subscribe(cmnd_takePicture.c_str());


      cmnd_alarm           ="cmnd/"+String(HostName)+"/ALARM"; 
      client.subscribe(cmnd_alarm.c_str());

      cmnd_alarmStatus     ="cmnd/"+String(HostName)+"/ALARMSTATUS"; 
      client.subscribe(cmnd_alarmStatus.c_str());

      cmmd_erasePhoto="cmnd/"+String(HostName)+"/ERASEPHOTO"; 
      client.subscribe(cmmd_erasePhoto.c_str());
      
      
      char str2[80];
      strcpy (str2,"cmnd/");
      strcat (str2,HostName);
      strcat (str2,"/");
      strcat (str2,topic_CONFIG);
      client.subscribe(str2);
      char str3[80];
      strcpy (str3,"/cmnd/");
      strcat (str3,HostName);
      strcat (str3,"/");
      strcat (str3,topic_UP);
      client.subscribe(str3);
      cmnd_state="cmnd/"+String(HostName)+"/state";

      client.subscribe(cmnd_state.c_str());
      client.subscribe(send_photo.c_str());
      //client.subscribe(cmnd_alarmStatus.c_str());  
      Serial.println("topics subscribed");


    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      delay(5000);
    }
  }
}
