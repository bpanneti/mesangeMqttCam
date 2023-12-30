  #include "esp_camera.h"
  #include <WiFi.h>
  #include <DNSServer.h>
  #include <WiFiClient.h>
  #include <WebServer.h>
  #include <PubSubClient.h>
  #include <ArduinoJson.h> //ArduinoJSON6
  #include "SPIFFS.h"
  #include "FS.h"                // SD Card ESP32
  #include "SD_MMC.h"            // SD Card ESP32
  #include "soc/soc.h"           // Disable brownour problems
  #include "soc/rtc_cntl_reg.h"  // Disable brownour problems
  #include "driver/rtc_io.h"
  #include <EEPROM.h>            // read and write from flash memory
  #include "time.h"
  
  //#include "ESP32_MailClient.h"
  // The Email Sending data object contains config and data to send
  //SMTPData smtpData;
  /*
  #define emailSenderAccount    "pannetierbenjamin@gmail.com"
  #define emailSenderPassword   "Gpann2003"
  #define smtpServer            "smtp.gmail.com"
  #define smtpServerPort        465
  #define emailSubject          "Alarm maison ! ESP32-CAM Photo Capture"
  #define emailRecipient        "pannetierbenjamin@gmail.com"
  */
  // define the number of bytes you want to access
  #define EEPROM_SIZE 3
  #define FLASH_GPIO_NUM 4
  #define ONBOARD_RED_LED  33               // main board red LED 
  #define MQTT_MAX_PACKET_SIZE 1024
  
  #define MEMORY_PHOTONUMBER 0
  #define MEMORY_ALARMSTATUS 1
  #define MEMORY_ALARM       2
   
  IPAddress apIP(192, 168, 1, 41);
  const char* apSSID = "ESPCAM_SETUP";
  
  volatile boolean POWER          = false;
  volatile boolean TAKE_PICTURE   = false;
  volatile boolean ALARM          = false;
  volatile boolean ALARMSTATUS    = false;
   
  String ssidList;
  int pictureNumber = 0;
  //DNSServer dnsServer;
  //WebServer webServer(80);
  
  DynamicJsonDocument CONFIG(2048);
  DynamicJsonDocument CONFIGTEMP(2048);
  DynamicJsonDocument WIFI(2048);
  
  // Select camera model
  //#define CAMERA_MODEL_WROVER_KIT
  //#define CAMERA_MODEL_ESP_EYE
  //#define CAMERA_MODEL_M5STACK_PSRAM
  //#define CAMERA_MODEL_M5STACK_WIDE
  #define CAMERA_MODEL_AI_THINKER
  #include "pins.h"
  #define timeSeconds 10
  
  
  
  const char  *ssid, *password, *mqtt_server, *mqtt_user, *mqtt_pass, *topic_PHOTO, *topic_CONFIG, *topic_UP, *HostName;
  String cmnd_state,   cmnd_power, cmnd_lwt, send_photo, cmnd_takePicture,cmnd_alarm,cmnd_alarmStatus,cmmd_erasePhoto;
  
  WiFiClient espClient;
  PubSubClient client(espClient);
  
  // Timer: Auxiliary variables
  unsigned long now = millis();
  unsigned long lastTrigger = 0;
  unsigned long startTime = millis();
  boolean startTimer = false;
  
  // Set GPIOs for PIR Motion Sensor
  const int motionSensor = 16;
  
  
   // Pour la récupération de l'heure
  const char* ntpServer = "pool.ntp.org";
  const long  gmtOffset_sec = 0;
  const int   daylightOffset_sec = 3600;
  
  const int delayTorestart = 24; //delay en heure de redémarrage 
  
  void(* resetFunc) (void) = 0; //declare reset function @ address 0
  void restart(){
  
    if ((now-startTime)>delayTorestart*3600*1000){
      Serial.println("----> in restart");
      Serial.println(startTime);
      Serial.println(now);  
      startTime = millis();
      resetFunc();
    }
  }
  void  detectsMovement() {
   
    if (digitalRead(motionSensor) &&  ALARMSTATUS && (now - lastTrigger > (timeSeconds*1000))){
    Serial.println("MOTION DETECTED!!!");
  
    startTimer = true;
    lastTrigger = millis();
  
  
  
        
    //Activation de l'alarme
  
    //POWER       = true;
    //statePOWER();
    //----> TAKE_PICTURE
    TAKE_PICTURE = true;
    statePICTURE();
    //----> SEND alarm
    ALARM = true;
    stateALARM();
    }
  
  
  }
   
  
  void statePICTURE(){
     char  json_state[100];
  
   
     
   if (TAKE_PICTURE) 
        sprintf(json_state,"{\"sensor\" : \"CAMERA\",\"TAKEPICTURE\" : \"ON\",\"NUMBERPICTURE\" : %d }", pictureNumber);
        //strcpy(json_state ,"{\"sensor\" : \"CAMERA\",\"TAKEPICTURE\" : \"ON\"  }");
        
   else
       //strcpy(json_state,"{\"sensor\" : \"CAMERA\",\"TAKEPICTURE\" : \"OFF\"  }");
       sprintf(json_state,"{\"sensor\" : \"CAMERA\",\"TAKEPICTURE\" : \"OFF\",\"NUMBERPICTURE\" : %d }", pictureNumber);
  
    Serial.print(json_state);
   char str2[80];
   strcpy (str2,"stat/");
   strcat (str2,HostName);
   strcat (str2,"/RESULT");
   client.publish(str2,json_state);
   EEPROM.write(MEMORY_PHOTONUMBER, pictureNumber);
   EEPROM.commit();  
  }
  void statePOWER(){
   char  json_state[50];
   if (POWER)
       strcpy(json_state ,"{\"sensor\" : \"CAMERA\",\"POWER\" : \"ON\"}");
   else
       strcpy(json_state,"{\"sensor\" : \"CAMERA\",\"POWER\" : \"OFF\"}");
      
   char str2[80];
   strcpy (str2,"stat/");
   strcat (str2,HostName);
   strcat (str2,"/RESULT");
   client.publish(str2,json_state);
    
  }
  
  void stateALARM(){
  
   char  json_state[50];
   if (ALARM)
       strcpy(json_state ,"ON");
   else
       strcpy(json_state,"OFF");
      
   char str2[80];
   strcpy (str2,"stat/");
   strcat (str2,HostName);
   strcat (str2,"/ALARM");
   client.publish(str2,json_state);
   int valBool = 0;
    if (ALARM)
      valBool = 1;
      
   EEPROM.write(MEMORY_ALARM, valBool);
   EEPROM.commit();  
  }
  
  void stateALARMSTATUS(){
  
   char  json_state[50];
   if (ALARMSTATUS)
       strcpy(json_state ,"{\"sensor\" : \"CAMERA\",\"ALARMSTATUS\" : \"ON\"}");
   else
       strcpy(json_state,"{\"sensor\" : \"CAMERA\",\"ALARMSTATUS\" : \"OFF\"}");
      
   char str2[80];
   strcpy (str2,"stat/");
   strcat (str2,HostName);
   strcat (str2,"/RESULT");
   client.publish(str2,json_state);
    int valBool = 0;
    if (ALARMSTATUS)
      valBool = 1;
      
  EEPROM.write(MEMORY_ALARMSTATUS, valBool);
  EEPROM.commit();
  
   valBool = 0;
    if (ALARM)
      valBool = 1;
      
   EEPROM.write(MEMORY_ALARM, valBool);
   EEPROM.commit();  
  
  }
  
  void state(){
   char  json_state[50];
  // if (POWER)
       strcpy(json_state ,"Online");
  // else
  //     strcpy(json_state,"Offline");
      
   char str2[80];
   strcpy (str2,"tele/");
   strcat (str2,HostName);
   strcat (str2,"/LWT");
   client.publish(str2,json_state);
    
  }
  void callback(char * topic, byte* message, unsigned int length) {
  
    Serial.print("Message arrived on topic: ");
    Serial.print(String(topic));
    Serial.print(". Message: ");
    String messageTemp;
   
     for (int i = 0; i < length; i++) {
      Serial.print((char)message[i]);
      messageTemp += (char)message[i];
    }
    Serial.println();
  
    //String msg = String(topic);  
    if ((String)(topic) == cmnd_state){
        statePOWER();
        statePICTURE();
        stateALARMSTATUS();
        Serial.println("state sended");
    }
    
  
    if ((String)(topic) == cmnd_takePicture && messageTemp=="ON"){
    TAKE_PICTURE = true;
    statePICTURE();
    }
    
    if ((String)(topic) == cmnd_takePicture && messageTemp=="OFF"){
      TAKE_PICTURE = false;
      statePICTURE();
    }
  
    if ((String)(topic) == cmmd_erasePhoto )
    {
  
   
  
      pictureNumber = 0;     
      statePICTURE();
      Serial.println("..restart");
      resetFunc();
      
    }
      if ((String)(topic) == cmnd_alarmStatus && messageTemp=="ON"){
        ALARMSTATUS = true;
  
        stateALARMSTATUS();
    }
      if ((String)(topic) == cmnd_alarmStatus && messageTemp=="OFF"){
        ALARMSTATUS = false;
        stateALARMSTATUS();
    }
      if ((String)(topic) == cmnd_alarm && messageTemp=="ON"){
        ALARMSTATUS = true;
        ALARM       = true;
        stateALARMSTATUS();
        TAKE_PICTURE = true;
        statePICTURE();
  //      POWER=true;
  //      statePOWER();
    }
        if ((String)(topic) == cmnd_alarm && messageTemp=="OFF"){
        ALARMSTATUS = false;
        ALARM       = false;
        stateALARMSTATUS();
        TAKE_PICTURE = false;
        statePICTURE();
        POWER=false;
        statePOWER();
    }
  
    
    if ((String)(topic) == cmnd_power && messageTemp=="OFF"){
      POWER=false;
      TAKE_PICTURE = false;
      statePICTURE();
      statePOWER();
    }
    if ((String)(topic) == cmnd_power && messageTemp=="ON") {
  
      POWER = true;
       statePOWER();
   
    }
    if (topic == topic_CONFIG) {
      deserializeJson(CONFIGTEMP, message, length);
      edit_config();
    }
  }
  
  void printLocalTime(){
    struct tm timeinfo;
    if(!getLocalTime(&timeinfo)){
      Serial.println("Failed to obtain time");
      return;
    }
    Serial.println(&timeinfo, "%A, %B %d %Y %H:%M:%S");
    Serial.print("Day of week: ");
    Serial.println(&timeinfo, "%A");
    Serial.print("Month: ");
    Serial.println(&timeinfo, "%B");
    Serial.print("Day of Month: ");
    Serial.println(&timeinfo, "%d");
    Serial.print("Year: ");
    Serial.println(&timeinfo, "%Y");
    Serial.print("Hour: ");
    Serial.println(&timeinfo, "%H");
    Serial.print("Hour (12 hour format): ");
    Serial.println(&timeinfo, "%I");
    Serial.print("Minute: ");
    Serial.println(&timeinfo, "%M");
    Serial.print("Second: ");
    Serial.println(&timeinfo, "%S");
  
    Serial.println("Time variables");
    char timeHour[3];
    strftime(timeHour,3, "%H", &timeinfo);
    Serial.println(timeHour);
    char timeWeekDay[10];
    strftime(timeWeekDay,10, "%A", &timeinfo);
    Serial.println(timeWeekDay);
    Serial.println();
  }
  
  void setup() {
  
  
    Serial.begin(115200);
  
    
    if (!SPIFFS.begin(true)) {
      Serial.println("An Error has occurred while mounting SPIFFS");
      return;
    }
    
 
    pinMode(ONBOARD_RED_LED, OUTPUT);
  
  
  
    // PIR Motion Sensor mode INPUT_PULLUP
  
    pinMode(motionSensor,INPUT);
  
 
  
   if (load_wifi()) {
      if (setup_wifi()) {
      
        //WiFi.mode(WIFI_STA);
         camera_init();
         
         client.setServer(mqtt_server, 1883);
         client.setCallback(callback);
    
         load_config();
  
  //      statePOWER();
  //      statePICTURE();
  //      stateALARMSTATUS();
  //      stateALARM();
  
      // Init and get the time
   
        configTime(gmtOffset_sec, daylightOffset_sec, ntpServer);
        printLocalTime();
   
        Serial.println("topics send configuration");
        return;
      }
    }
  
  }
  
  
  
  
  void loop() {
    // Current time
     now = millis();
    /*
    if (settingMode) {
      dnsServer.processNextRequest();
      webServer.handleClient();
    }
    else {*/
  
   while (   WiFi.status() != WL_CONNECTED){
  setup_wifi();
    
   }
   while (!client.connected() ) {
        reconnect();
      }

   
    take_picture();
   
    //Gestion du PIR
    
    detectsMovement();
  
    //restart
  
    restart();
    
    client.loop();
  }
