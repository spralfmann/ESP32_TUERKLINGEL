// Projekt Türklingel mit ESP32-CAM und Android Java App

#include <Arduino.h>
#include <WiFi.h>
#include <Firebase_ESP_Client.h>
#include "WiFi.h"
#include "esp_camera.h"
#include "soc/soc.h"           
#include "soc/rtc_cntl_reg.h"  
#include "driver/rtc_io.h"
#include <SPIFFS.h>
#include <FS.h>
#include <Firebase_ESP_Client.h>

// Einbinden der eigenen Librarys für Kamera und Wifi Setup
#include "config.h"
#include "wifi_setup.h"
#include "camera_setup.h"

//Provide the token generation process info.
#include "addons/TokenHelper.h"
//Provide the RTDB payload printing info and other helper functions.
#include "addons/RTDBHelper.h"

// Variablen Definitionen
  // Insert your network credentials
  
  // FCM Deklaration: FIREBASE CLOUD MESSAGING Server Key
  #define FIREBASE_FCM_SERVER_KEY "AAAAAPpPfWI:APA91bFBJZFFdhyuPB7-voUwCWFfBTO7pWEdp3p2N2AFJNyCif0DAnOOVMzLGZqcsL0XCpEd3ieuSw9-MaKIwcE2oo2fCsi2HRCPsMc2AfojkImYv-z5_Mmqo_5c0sQ5aHbJYbsXsVvV"
  
  // Insert Firebase project API Key
  #define API_KEY "AIzaSyDIaa0nwEYfaesAgcph1rO8ahau0k5LR6w"
  
  // Insert RTDB URLefine the RTDB URL */
  #define DATABASE_URL "https://tuerklingel-a0ba8-default-rtdb.europe-west1.firebasedatabase.app/" 

 // ENter Authorized Email and Password
  #define USER_EMAIL "erhof1@gmail.com"               //angepasst
  #define USER_PASSWORD "passwort1234"                //angepasst

  // Enter Firebase storage bucket ID
  #define STORAGE_BUCKET_ID "tuerklingel-a0ba8.appspot.com"      //angepasst

  #define IMAGE_PATH "/pictures/Testfoto.jpg"

  


//Define Firebase Data object
  FirebaseData fbdo;
  FirebaseAuth auth;
  FirebaseConfig config;
bool done = false;

bool check_photo( fs::FS &fs ) {
  File f_pic = fs.open( IMAGE_PATH );
  unsigned int pic_sz = f_pic.size();
  return ( pic_sz > 100 );
}

// Capture Photo and Save it to SPIFFS
void captureSave_photo( void ) {
  camera_fb_t * fb = NULL; 
  bool ok = 0; 
  do {
    Serial.println("ESP32-CAM capturing photo...");

    fb = esp_camera_fb_get();
    if (!fb) {
      Serial.println("Failed");
      return;
    }
    Serial.printf("Picture file name: %s\n", IMAGE_PATH);
    File file = SPIFFS.open(IMAGE_PATH, FILE_WRITE);
    if (!file) {
      Serial.println("Failed to open file in writing mode");
    }
    else {
      file.write(fb->buf, fb->len); 
      Serial.print("The picture has been saved in ");
      Serial.print(IMAGE_PATH);
      Serial.print(" - Size: ");
      Serial.print(file.size());
      Serial.println(" bytes");
    }
    file.close();
    esp_camera_fb_return(fb);
    ok = check_photo(SPIFFS);
  } while ( !ok );
}

  unsigned long lastTime = 0;
  unsigned long sendDataPrevMillis = 0;
  int count = 0;
  bool signupOK = false;

  int intValue;
  float floatValue;

// Funktionsvordeklarationen
  // FCM Funktion zum Senden einer Nachricht an ein Topic
  void sendMessageFcm();


int klingeltaster = 12; // Klingetaster anlegen
int tuerkontakt = 13;
int relais = 15;
String led_state = "";  

// Initialisierung
  void setup(){
    pinMode(klingeltaster, INPUT); 
    pinMode(relais, OUTPUT); 
    pinMode(tuerkontakt, INPUT); 
    
    // Wifi Netzwerk Verbindungsaufbau
    Serial.begin(115200);
    WiFi.begin(WIFI_SSID,WIFI_PASSWORD);
    

    // EWAUSGABE Firebase Client Infos
    Serial.printf("Firebase Client v%s\n\n", FIREBASE_CLIENT_VERSION);

    // FCM Benötigt für die Legacy HTTP API
    Firebase.FCM.setServerKey(FIREBASE_FCM_SERVER_KEY);
  
    /* Assign the api key (required) */
    config.api_key = API_KEY;
  
    /* Assign the RTDB URL (required) */
    config.database_url = DATABASE_URL;
    
    

  auth.user.email = USER_EMAIL;
  auth.user.password = USER_PASSWORD;
  config.token_status_callback = tokenStatusCallback; 


  Firebase.begin(&config, &auth);
  Firebase.reconnectWiFi(true);

  
    /* Sign up */
    if (Firebase.signUp(&config, &auth, "", "")){
      signupOK = true;
    }
    else{
      Serial.printf("%s\n", config.signer.signupError.message.c_str());
    }
  
    /* Assign the callback function for the long running token generation task */
    config.token_status_callback = tokenStatusCallback; //see addons/TokenHelper.h
    
    Firebase.begin(&config, &auth);
    Firebase.reconnectWiFi(true);
     if (!SPIFFS.begin(true)) {
    Serial.println("An Error has occurred while mounting SPIFFS");
    ESP.restart();
  }
  else {
    delay(500);
    Serial.println("SPIFFS mounted successfully");
  }

  WRITE_PERI_REG(RTC_CNTL_BROWN_OUT_REG, 0);

 // initialize OV2640 camera module
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

  if (psramFound()) {
    config.frame_size = FRAMESIZE_UXGA;
    config.jpeg_quality = 10;
    config.fb_count = 2;
  } else {
    config.frame_size = FRAMESIZE_SVGA;
    config.jpeg_quality = 12;
    config.fb_count = 1;
  }

  esp_err_t err = esp_camera_init(&config);
  if (err != ESP_OK) {
    Serial.printf("Camera init failed with error 0x%x", err);
    ESP.restart();
  } 
}

void loop(){
   if(digitalRead(klingeltaster) == LOW)
  {
    sendMessage_klingeltaster();
    captureSave_photo();
    delay(1);
    if (Firebase.ready())
    {
      Serial.print("Uploading Photo... ");
  
      if (Firebase.Storage.upload(&fbdo, STORAGE_BUCKET_ID, IMAGE_PATH, mem_storage_type_flash, IMAGE_PATH, "image/jpeg" ))
      {
        Serial.printf("\nDownload URL: %s\n", fbdo.downloadURL().c_str());
      }
      else{
        Serial.println(fbdo.errorReason());
      }
    }
  }
  Firebase.RTDB.getInt(&fbdo, "/status/triggeropen");
  intValue = fbdo.intData();
  Serial.print("first ");
  Serial.println(intValue);
  
  if (intValue == 1) {
  
        Serial.print("second ");
        Serial.println(intValue);
       //sendMessage_relais();
        digitalWrite(relais, HIGH);
        delay(3000);
        digitalWrite(relais, LOW);
        Firebase.RTDB.setInt(&fbdo, "/status/triggeropen", 0);
    }
  delay(500);
}

// Sendet eine Benachrichtigung an den Topic "/topics/ring"
void sendMessageFcm()
{

    Serial.print("Send Firebase Cloud Messaging... ");

    //Read more details about legacy HTTP API here https://firebase.google.com/docs/cloud-messaging/http-server-ref
    FCM_Legacy_HTTP_Message msg;

    msg.targets.to = "/topics/ring";

    msg.options.time_to_live = "1000";
    msg.options.priority = "high";

    msg.payloads.notification.title = "Test Ring";
    msg.payloads.notification.body = "der ESP hat geklingelt";
    msg.payloads.notification.icon = "myicon";
    msg.payloads.notification.click_action = "OPEN_ACTIVITY_1";

    FirebaseJson payload;

    msg.payloads.data = payload.raw();

    if (Firebase.FCM.send(&fbdo, &msg)) //send message to recipient
        {
        Serial.print("Sendung nach Klingeltasterdruck kam an ");
        Serial.printf("ok\n%s\n\n", Firebase.FCM.payload(&fbdo).c_str());
        }
    else
        Serial.println(fbdo.errorReason());

    count++;
}
