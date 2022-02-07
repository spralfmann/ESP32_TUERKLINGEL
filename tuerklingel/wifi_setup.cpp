#include <WiFi.h>
#include "wifi_setup.h"

// Methode für Feedback über Netzwerkverbindung in Serial Monitor
void setupWifiDisplay(){
    Serial.print("Connecting to Wi-Fi");
    while (WiFi.status() != WL_CONNECTED){
    Serial.print(".");
    delay(300);
    }
    Serial.println();
    Serial.print("Connected with IP: ");
    Serial.println(WiFi.localIP());
    Serial.println();
}
