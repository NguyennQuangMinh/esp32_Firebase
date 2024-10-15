#include <Arduino.h>
#include <ArduinoJson.h>
#include <HTTPClient.h>
#include <LiquidCrystal_I2C.h>
#include <WiFi.h>
#include "time.h"

// WiFi Credentials
#define WIFI_SSID "P502"
#define WIFI_PASSWORD "20032002@"

// Firebase things
#define DATABASE_URL "https://esp32-ledcontroller-default-rtdb.asia-southeast1.firebasedatabase.app/.json"
HTTPClient client;

//definir los pines
#define trigPin 5
#define echoPin 18
#define ledV 2
#define ledR 4
LiquidCrystal_I2C LCD = LiquidCrystal_I2C(0x27, 16, 2);

//variables
int prev = 0;
String tempText = "";
String payload = "";
String sensorID = "HC001";
char timeStringBuff[20];
String ledVBool;
String ledRBool;
String safeDistBool;

void setup()
{
  Serial.begin(115200);

  LCD.init();
  LCD.backlight();
  LCD.setCursor(0, 0);
  LCD.print("Connecting to ");
  LCD.setCursor(0, 1);
  LCD.print("WiFi ");

  // WIFI
  WiFi.mode(WIFI_STA);
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  Serial.print("connecting");
  while (WiFi.status() != WL_CONNECTED) {
    Serial.print(".");
    delay(500);
  }
  Serial.println();
  Serial.print("connected: ");
  Serial.println(WiFi.localIP());

  LCD.clear();
  LCD.setCursor(0, 0);
  LCD.println("Online");
  delay(500);
  LCD.clear();
  LCD.setCursor(0, 0);
  LCD.println("Connecting to");
  LCD.setCursor(0, 1);
  LCD.println("Firebase...");
  Serial.println("connecting...");

  // Firebase
  client.begin(DATABASE_URL);
  int httpResponseCode=client.GET();
  
  if (httpResponseCode>0) {
    LCD.clear();
    LCD.setCursor(0, 0);
    LCD.println("Connected");
    Serial.println("connected, Firebase payload:");
    payload = client.getString();
    Serial.println(payload);
    Serial.println();
  }

  // Componentes
  pinMode(trigPin, OUTPUT); //definir como salida
  pinMode(echoPin, INPUT); //definir como entrada
  pinMode(ledV, OUTPUT);
  pinMode(ledR, OUTPUT);

  Serial.println("Verde = DISTANCIA SEGURA");
  Serial.println("Rojo = NO SEGURA");

  // Indica comienzo de Transmisión de Datos
  for (int i=0; i<5; i++) {
    digitalWrite(ledV, HIGH);
    digitalWrite(ledR, HIGH);
    delay(200);
    digitalWrite(ledV, LOW);
    digitalWrite(ledR, LOW);
    delay(200);
  }

  // Time
  configTime(-9000, -9000, "1.mx.pool.ntp.org");
}

void loop()
{
  long sure, mesafe;
  digitalWrite(trigPin, LOW);
  delayMicroseconds(2);
  digitalWrite(trigPin, HIGH);
  delayMicroseconds(2);
  digitalWrite(trigPin, LOW);
  sure=pulseIn(echoPin, HIGH);
  mesafe=(sure/2)/29.1;


  if(prev!=mesafe) {
    if(mesafe>200 || mesafe<0){
      digitalWrite(ledV, HIGH);
      digitalWrite(ledR, LOW);
      tempText = "DISTANCIA SEGURA: ";
      ledVBool = "true";
      ledRBool = "false";
      safeDistBool = "true";
    }
    else {
      digitalWrite(ledV, LOW);
      digitalWrite(ledR, HIGH);
      tempText = "NO SEGURA: ";
      ledVBool = "false";
      ledRBool = "true";
      safeDistBool = "false";
    }

    struct tm timeinfo;
    if (!getLocalTime(&timeinfo)) {
      Serial.println("Time Err");
    }
    strftime(timeStringBuff, sizeof(timeStringBuff), "%d/%m/%Y %H:%M", &timeinfo);
    Serial.println(String(timeStringBuff));

    client.PATCH("{\"Sensores/time\":\"" + String(timeStringBuff) + "\"}");
    client.PATCH("{\"Led/ledV\":" + ledVBool + "}");
    client.PATCH("{\"Led/ledR\":" + ledRBool + "}");
    client.PATCH("{\"Sensores/Distancia\":" + String(mesafe) + "}");
    client.PATCH("{\"Sensores/safeDist\":" + safeDistBool + "}");
    client.PATCH("{\"Sensores/id\":" + sensorID + "}");

    LCD.clear();
    LCD.setCursor(0,0);
    LCD.print(tempText);
    LCD.setCursor(0,1);
    LCD.print(mesafe);

    Serial.print(tempText);
    Serial.print(mesafe);
    Serial.println(" cm");
  }

  prev = mesafe;
  delay(500);
}
