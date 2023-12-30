#include <base64.h>
#include "driver/rtc_io.h"


void createDir(fs::FS &fs, const char * path){
    Serial.printf("Creating Dir: %s\n", path);
    if(fs.mkdir(path)){
        Serial.println("Dir created");
    } else {
        Serial.println("mkdir failed");
    }
}

void removeDir(fs::FS &fs, const char * path){
    Serial.printf("Removing Dir: %s\n", path);
    if(fs.rmdir(path)){
        Serial.println("Dir removed");
    } else {
        Serial.println("rmdir failed");
    }
}
void camera_init() {
  camera_config_t config;
  config.ledc_channel = LEDC_CHANNEL_0;
  config.ledc_timer = LEDC_TIMER_0;
  config.pin_d0 = Y2_GPIO_NUM;
  config.pin_d1 = Y3_GPIO_NUM;
  config.pin_d2 = Y4_GPIO_NUM;
  config.pin_d3 = Y5_GPIO_NUM;
  config.pin_d4 = Y6_GPIO_NUM;
  config.pin_d5 = Y7_GPIO_NUM;
  config.pin_d6 = Y8_GPIO_NUM;
  config.pin_d7 = Y9_GPIO_NUM;
  config.pin_xclk = XCLK_GPIO_NUM;
  config.pin_pclk = PCLK_GPIO_NUM;
  config.pin_vsync = VSYNC_GPIO_NUM;
  config.pin_href = HREF_GPIO_NUM;
  config.pin_sscb_sda = SIOD_GPIO_NUM;
  config.pin_sscb_scl = SIOC_GPIO_NUM;
  config.pin_pwdn = PWDN_GPIO_NUM;
  config.pin_reset = RESET_GPIO_NUM;
  config.xclk_freq_hz = 20000000;
  config.pixel_format = PIXFORMAT_JPEG;

  config.fb_count = 1;
  config.jpeg_quality   = 50;
  config.frame_size     = FRAMESIZE_VGA;
  //config.fb_location    = CAMERA_FB_IN_PSRAM;
//  config.grab_mode      = CAMERA_GRAB_LATEST;

  #define FILE_PHOTO "/photos"

  
  Serial.println("Starting SD Card");

  if(!SD_MMC.begin()){
    Serial.println("SD Card Mount Failed");

  }
  else{
    Serial.println("create photos repo");
    createDir(SD_MMC,"/photos");
    Serial.println("  photos repo created");
  }

  uint8_t cardType = SD_MMC.cardType();
  if(cardType == CARD_NONE){
    Serial.println("No SD Card attached");
  }
  
  esp_err_t err = esp_camera_init(&config);
  if (err != ESP_OK) {
    Serial.printf("Camera init failed with error 0x%x", err);
    return;
  }
Serial.println("SD Card Mounted");

  EEPROM.begin(EEPROM_SIZE);
  pictureNumber = EEPROM.read(MEMORY_PHOTONUMBER) + 1; 
  Serial.println("camera initialization done!");
  Serial.printf("picture number %d",pictureNumber);

  int val  = EEPROM.read(MEMORY_ALARMSTATUS);
  ALARMSTATUS = false; 
  if(val==1)
    ALARMSTATUS = true;
  Serial.printf("ALARMSTATUS %d",ALARMSTATUS);
  val        = EEPROM.read(MEMORY_ALARM);
  ALARM = false; 
  if(val==1){
    ALARM = true;
    TAKE_PICTURE = true;
  }
  Serial.printf("ALARM %d",ALARM);

  Serial.printf("Init done");

}
// Check if photo capture was successful
bool checkPhoto(  fs::FS &fs ) {
  File f_pic = fs.open( FILE_PHOTO );
  unsigned int pic_sz = f_pic.size();
  return ( pic_sz > 100 );
}


void take_picture() {
  
//   if (ALARM==true){
//     pinMode(FLASH_GPIO_NUM, OUTPUT);
//     digitalWrite(FLASH_GPIO_NUM, HIGH);
//     Serial.println("ALARM ACTIVATED MOTION DETECTED!!!");
//   }
//  
  

 
 

 
  camera_fb_t * fb = NULL;
  fb = esp_camera_fb_get();
  if (!fb) {
    Serial.println("Camera capture failed 1");
    return;
  }
  

    //if(POWER || TAKE_PICTURE){
     // fb = esp_camera_fb_get();
    /*    if (!fb) {
            Serial.println("Camera capture failed 2");
            return;
        }*/
 
     if(POWER & TAKE_PICTURE){
      digitalWrite(FLASH_GPIO_NUM, HIGH);
     }
     if(POWER  ){
     
     client.beginPublish(send_photo.c_str(),fb->len, false);
     client.write(fb->buf, fb->len);
     client.endPublish();
    // Serial.println("image published");
     }
 
 // EEPROM.begin(EEPROM_SIZE);
  //pictureNumber = EEPROM.read(0) + 1;
 

 
    if (TAKE_PICTURE & pictureNumber <=240){
    
        pictureNumber = pictureNumber +1 ;//EEPROM.read(0) + 1;
        String path = "/photos/picture_" + String(pictureNumber) +".jpg"; ///----< /picture
        
        fs::FS &fs = SD_MMC; ///----< /picture
        Serial.printf("Picture file name: %s\n", path.c_str());
      
        File file =fs.open(path.c_str(), FILE_WRITE); ///----< /picture fs.open(path.c_str(), FILE_WRITE);
        if(!file){
            Serial.println("Failed to open file in writing mode");
        } 
  
        else {
             Serial.println("File opened  in writing mode");
            file.write(fb->buf, fb->len); // payload (image), payload length
            Serial.printf("Saved file to path: %s\n", path.c_str());

          }
        file.close(); 
        EEPROM.write(MEMORY_PHOTONUMBER, pictureNumber);
        EEPROM.commit();
                
        Serial.println("End take picture");
          
        /*
 
   
       if (pictureNumber==1){ 
          File file = SPIFFS.open(FILE_PHOTO, FILE_WRITE);
  
          // Insert the data in the photo file
            if (!file) {
            Serial.println("Failed to open file in writing mode");
            }
            else {
              file.write(fb->buf, fb->len); // payload (image), payload length
              Serial.print("The picture has been saved in ");
              Serial.print(FILE_PHOTO);
              Serial.print(" - Size: ");
              Serial.print(file.size());
              Serial.println(" bytes");
            }
            // Close the file
          file.close();
          
  
         if (!checkPhoto(SPIFFS))
           Serial.println("bad checked");
           sendPhoto();
         } */
    }
   if   (TAKE_PICTURE & pictureNumber >240 & POWER==false){
    //=====================
    //Affichage de la LED
    //====================
           pinMode(FLASH_GPIO_NUM, OUTPUT);
           digitalWrite(FLASH_GPIO_NUM, HIGH);
           delay(500);
           digitalWrite(FLASH_GPIO_NUM, LOW);
           delay(500);
         
           
    }

    
    esp_camera_fb_return(fb);
    fb = NULL;

 
 

 
}

void load_config() {
  File config_file = SPIFFS.open("/config.json", "r");
  if (!config_file) {
    Serial.println("Failed to open file for reading");
    return;
  }
  //String  config_string = config_file.readString();
  deserializeJson(CONFIG, config_file.readString());
  config_file.close();

  sensor_t * s = esp_camera_sensor_get();
  s->set_vflip(s, CONFIG["vflip"]); //0 - 1
  s->set_hmirror(s, CONFIG["hmirror"]); //0 - 1

  s->set_colorbar(s, CONFIG["colorbar"]); //0 - 1
  s->set_special_effect(s, CONFIG["special_effect"]); //0 -

  s->set_quality(s, CONFIG["quality"]); // 0 - 63

  s->set_gainceiling(s, (gainceiling_t)(int)CONFIG["gain"]); // 0 - 6
  s->set_brightness(s, CONFIG["brightness"]); // -2 - 2
  s->set_contrast(s, CONFIG["contrast"]); // -2 - 2
  s->set_saturation(s, CONFIG["saturation"]); // -2 - 2

  s->set_awb_gain(s, CONFIG["awb_gain"]); // 0 - 1
  s->set_wb_mode(s, CONFIG["wb_mode"]); // 0 - 4

  s->set_framesize(s, (framesize_t)(int)CONFIG["resolution"]);// QVGA|CIF|VGA|SVGA|XGA|SXGA|UXGA
}

void edit_config() {
  sensor_t * s = esp_camera_sensor_get();
  if (CONFIGTEMP.containsKey("vflip")) {
    CONFIG["vflip"] = CONFIGTEMP["vflip"];
  }
  if (CONFIGTEMP.containsKey("hmirror")) {
    CONFIG["hmirror"] = CONFIGTEMP["hmirror"];
  }
  if (CONFIGTEMP.containsKey("colorbar")) {
    CONFIG["colorbar"] = CONFIGTEMP["colorbar"];
  }
  if (CONFIGTEMP.containsKey("special_effect")) {
    CONFIG["special_effect"] = CONFIGTEMP["special_effect"];
  }
  if (CONFIGTEMP.containsKey("quality")) {
    CONFIG["quality"] = CONFIGTEMP["quality"];
  }
  if (CONFIGTEMP.containsKey("gain")) {
    CONFIG["gain"] = CONFIGTEMP["gain"];
  }
  if (CONFIGTEMP.containsKey("brightness")) {
    CONFIG["brightness"] = CONFIGTEMP["brightness"];
  }
  if (CONFIGTEMP.containsKey("contrast")) {
    CONFIG["contrast"] = CONFIGTEMP["contrast"];
  }
  if (CONFIGTEMP.containsKey("saturation")) {
    CONFIG["saturation"] = CONFIGTEMP["saturation"];
  }
  if (CONFIGTEMP.containsKey("awb_gain")) {
    CONFIG["awb_gain"] = CONFIGTEMP["awb_gain"];
  }
  if (CONFIGTEMP.containsKey("wb_mode")) {
    CONFIG["wb_mode"] = CONFIGTEMP["wb_mode"];
  }
  if (CONFIGTEMP.containsKey("resolution")) {
    CONFIG["resolution"] = CONFIGTEMP["resolution"];
  }
  if (CONFIGTEMP.containsKey("flash")) {
    CONFIG["flash"] = CONFIGTEMP["flash"];
  }
  save_config();
}
void save_config() {
  //Open the config.json file (Write Mode)
  String config_string;
  File config_file = SPIFFS.open("/config.json", "w");
  if (!config_file) {
    Serial.println("Failed to open file (Writing mode)");
    return;
  }
  //Save and close the JSON file
  serializeJson(CONFIG, config_string);
  if (config_file.println(config_string)) {
    Serial.println("New Config");
    Serial.println(config_string);
  } else {
    Serial.println("File write failed");
  }
  config_file.close();
  load_config();
}
/*
void sendPhoto(   ) {
  // Preparing email
  Serial.println("Sending email...");
  // Set the SMTP Server Email host, port, account and password
  smtpData.setLogin(smtpServer, smtpServerPort, emailSenderAccount, emailSenderPassword);
  
  // Set the sender name and Email
  smtpData.setSender("ESP32-CAM", emailSenderAccount);
  
  // Set Email priority or importance High, Normal, Low or 1 to 5 (1 is highest)
  smtpData.setPriority("High");

  // Set the subject
  smtpData.setSubject(emailSubject);
    
  // Set the email message in HTML format
  smtpData.setMessage("<h2>Photo captured with ESP32-CAM and attached in this email.</h2>", true);
  // Set the email message in text format
  //smtpData.setMessage("Photo captured with ESP32-CAM and attached in this email.", false);

  // Add recipients, can add more than one recipient
  smtpData.addRecipient(emailRecipient);
  //smtpData.addRecipient(emailRecipient2);

  // Add attach files from SPIFFS
  Serial.println(FILE_PHOTO);
  smtpData.addAttachFile(FILE_PHOTO,"image/jpg");//repertoire mais là c'est la racine
  smtpData.addAttachFile("wifi.json");//repertoire mais là c'est la racine
  // Set the storage type to attach files in your email (SPIFFS)
  smtpData.setFileStorageType(MailClientStorageType::SPIFFS);

  smtpData.setSendCallback(sendCallback);
  
  // Start sending Email, can be set callback function to track the status
  if (!MailClient.sendMail(smtpData))
    Serial.println("Error sending Email, " + MailClient.smtpErrorReason());
  else
     Serial.println("Email sending...");
  // Clear all data from Email object to free memory
  smtpData.empty();
}
// Callback function to get the Email sending status
void sendCallback(SendStatus msg) {
  //Print the current status
  Serial.println(msg.info());
}
*/
