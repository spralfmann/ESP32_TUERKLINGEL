/*
  Rui Santos
  Complete project details at our blog.
    - ESP32: https://RandomNerdTutorials.com/esp32-firebase-realtime-database/
    - ESP8266: https://RandomNerdTutorials.com/esp8266-nodemcu-firebase-realtime-database/
  Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files.
  The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.
  Based in the RTDB Basic Example by Firebase-ESP-Client library by mobizt
  https://github.com/mobizt/Firebase-ESP-Client/blob/main/examples/RTDB/Basic/Basic.ino
*/

#include <Arduino.h>
#include <WiFi.h>
#include <Firebase_ESP_Client.h>

//Provide the token generation process info.
#include "addons/TokenHelper.h"
//Provide the RTDB payload printing info and other helper functions.
#include "addons/RTDBHelper.h"

// Variablen Definitionen
  // Insert your network credentials
  #define WIFI_SSID "Test"
  #define WIFI_PASSWORD "cxha3216"
  
  // FCM Deklaration: FIREBASE CLOUD MESSAGING Server Key
  #define FIREBASE_FCM_SERVER_KEY "AAAAAPpPfWI:APA91bFBJZFFdhyuPB7-voUwCWFfBTO7pWEdp3p2N2AFJNyCif0DAnOOVMzLGZqcsL0XCpEd3ieuSw9-MaKIwcE2oo2fCsi2HRCPsMc2AfojkImYv-z5_Mmqo_5c0sQ5aHbJYbsXsVvV"
  
  // Insert Firebase project API Key
  #define API_KEY "AIzaSyDIaa0nwEYfaesAgcph1rO8ahau0k5LR6w"
  
  // Insert RTDB URLefine the RTDB URL */
  #define DATABASE_URL "https://tuerklingel-a0ba8-default-rtdb.europe-west1.firebasedatabase.app/" 


//Define Firebase Data object
  FirebaseData fbdo;
  
  FirebaseAuth auth;
  FirebaseConfig config;

  unsigned long lastTime = 0;
  unsigned long sendDataPrevMillis = 0;
  int count = 0;
  bool signupOK = false;

// Funktionsvordeklarationen
  // FCM Funktion zum Senden einer Nachricht an ein Topic
  void sendMessage();

int klingeltaster = 12; // Klingetaster anlegen

// Initialisierung
  void setup(){
    pinMode(klingeltaster, INPUT); 
    // Wifi Netzwerk Verbindungsaufbau
    Serial.begin(115200);
    WiFi.begin(WIFI_SSID,WIFI_PASSWORD);
    Serial.print("Connecting to Wi-Fi");
    while (WiFi.status() != WL_CONNECTED){
      Serial.print(".");
      delay(300);
    }
    Serial.println();
    Serial.print("Connected with IP: ");
    Serial.println(WiFi.localIP());
    Serial.println();

    // EWAUSGABE Firebase Client Infos
    Serial.printf("Firebase Client v%s\n\n", FIREBASE_CLIENT_VERSION);

    // FCM Benötigt für die Legacy HTTP API
    Firebase.FCM.setServerKey(FIREBASE_FCM_SERVER_KEY);
  
    /* Assign the api key (required) */
    config.api_key = API_KEY;
  
    /* Assign the RTDB URL (required) */
    config.database_url = DATABASE_URL;
  
    /* Sign up */
    if (Firebase.signUp(&config, &auth, "", "")){
      Serial.println("ok");
      signupOK = true;
    }
    else{
      Serial.printf("%s\n", config.signer.signupError.message.c_str());
    }
  
    /* Assign the callback function for the long running token generation task */
    config.token_status_callback = tokenStatusCallback; //see addons/TokenHelper.h
    
    Firebase.begin(&config, &auth);
    Firebase.reconnectWiFi(true);
  }

void loop(){
   if(digitalRead(klingeltaster) == LOW)
  {
  // senden einer Nachricht an Firebase
        sendMessage();
  }
}

// Sendet eine Benachrichtigung an den Topic "/topics/ring"
void sendMessage()
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

    //all data key-values should be string
    payload.add("temp", "28");
    payload.add("unit", "celsius");
    payload.add("timestamp", "1609815454");
    msg.payloads.data = payload.raw();

    if (Firebase.FCM.send(&fbdo, &msg)) //send message to recipient
        Serial.printf("ok\n%s\n\n", Firebase.FCM.payload(&fbdo).c_str());
    else
        Serial.println(fbdo.errorReason());

    count++;
}
