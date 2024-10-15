#include <Arduino.h>
#if defined(ESP32)
  #include <WiFi.h>
#elif defined(ESP8266)
  #include <ESP8266WiFi.h>
#endif
#include <Firebase_ESP_Client.h>

// Provide the token generation process info.
#include "addons/TokenHelper.h"
// Provide the RTDB payload printing info and other helper functions.
#include "addons/RTDBHelper.h"

// Insert your network credentials
#define WIFI_SSID "P502"
#define WIFI_PASSWORD "20032002@"

// Insert RTDB URL
#define DATABASE_URL "https://esp32-controlled-default-rtdb.asia-southeast1.firebasedatabase.app/" //Firebase URL
// Insert Firebase project API Key
#define API_KEY "AIzaSyDO2JqnnBglE8r53fQeKdjlgzj-mACSf7g" // API FireBase

// Define GPIO pin for the LED
#define LED_PIN 2

// Define Firebase Data object
FirebaseData fbdo;
FirebaseAuth auth;
FirebaseConfig config;
bool ledStatus = false;
bool signupOK = false;

void setup() {
  Serial.begin(115200);
  InitWiFi();
  
  // Initialize LED pin
  pinMode(LED_PIN, OUTPUT);
  digitalWrite(LED_PIN, LOW); // Initial LED state off

  InitFireBase();

}

void loop() {
  // Keep the loop function empty since the LED status will be updated via the stream callback
}

void InitWiFi(){
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  Serial.print("Connecting to Wi-Fi");

  while (WiFi.status() != WL_CONNECTED) {
    Serial.print(".");
    delay(300);
  }

  Serial.println();
  Serial.print("Connected with IP: ");
  Serial.println(WiFi.localIP());
}

void InitFireBase(){
  // Assign the api key and RTDB URL
  config.api_key = API_KEY;
  config.database_url = DATABASE_URL;

  // Sign up
  if (Firebase.signUp(&config, &auth, "", "")) {
    Serial.println("Sign up successful");
    signupOK = true;
  } else {
    Serial.printf("Sign up failed: %s\n", config.signer.signupError.message.c_str());
  }

  // Assign the callback function for token generation task
  config.token_status_callback = tokenStatusCallback;

  Firebase.begin(&config, &auth);
  Firebase.reconnectWiFi(true);

  // Set up a stream to listen for changes in LED_STATUS
  if (!Firebase.RTDB.beginStream(&fbdo, "LED_STATUS")) {
    Serial.printf("Stream begin failed: %s\n", fbdo.errorReason().c_str());
  }

  // Set the stream callback function
  Firebase.RTDB.setStreamCallback(&fbdo, streamCallback, streamTimeoutCallback);
}

// Stream callback function to handle updates
void streamCallback(FirebaseStream data) {
  if (data.dataType() == "boolean") {
    ledStatus = data.boolData();
    digitalWrite(LED_PIN, ledStatus ? HIGH : LOW);
    Serial.println("LED_STATUS updated: " + String(ledStatus));
  }
}

// Stream timeout callback
void streamTimeoutCallback(bool timeout) {
  if (timeout) {
    Serial.println("Stream timed out, resuming...");
    Firebase.RTDB.readStream(&fbdo);
  }
}
