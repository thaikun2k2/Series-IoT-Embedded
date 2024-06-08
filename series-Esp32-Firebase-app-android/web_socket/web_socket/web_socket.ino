#include <WiFi.h>
#include <Firebase_ESP_Client.h>
#include "addons/TokenHelper.h"
#include "addons/RTDBHelper.h"
#include <LiquidCrystal_I2C.h>
#include <DHT.h>
#include <NTPClient.h>
#include <WiFiUdp.h>
#include <Wire.h>
#include <ArduinoJson.h>
#include <EEPROM.h>
#include <WebServer.h>

// Wi-Fi configuration
const char* apSSID = "ESP32";
const char* apPassword = "12345678";
WebServer server(80);

// Firebase configuration
#define DATABASE_URL "https://project5-c5e9d-default-rtdb.firebaseio.com/"
#define API_KEY "AIzaSyBRYZvibVMMQHM4aCan6G7oxkrk9DQzEls"

FirebaseData fbdo;
FirebaseAuth auth;
FirebaseConfig config;

boolean restoreConfig() {
  Serial.println("Reading EEPROM...");
  String WIFI_SSID = "";
  String WIFI_PASSWORD = "";
  if (EEPROM.read(0) != 0) {
    for (int i = 0; i < 32; ++i) {
      WIFI_SSID += char(EEPROM.read(i));
    }
    for (int i = 32; i < 96; ++i) {
      WIFI_PASSWORD += char(EEPROM.read(i));
    }
    Serial.print("SSID: ");
    Serial.println(WIFI_SSID);
    Serial.print("Password: ");
    Serial.println(WIFI_PASSWORD);
    WiFi.begin(WIFI_SSID.c_str(), WIFI_PASSWORD.c_str());
    return true;
  } else {
    Serial.println("Config not found.");
    delay(300);
    return false;
  }
}

boolean checkConnection() {
  int count = 0;
  Serial.print("Waiting for Wi-Fi connection");
  while (count < 30) {
    if (WiFi.status() == WL_CONNECTED) {
      Serial.println();
      Serial.print("Connected: ");
      Serial.println(WiFi.localIP());
      return true;
    }
    delay(500);
    Serial.print(".");
    count++;
  }
  Serial.println("Timed out.");
  return false;
}

void handleRoot() {
  String html = "<html>\
  <body>\
  <h1>Configure WiFi</h1>\
  <form action=\"/submit\" method=\"post\">\
  SSID: <input type=\"text\" name=\"ssid\"><br>\
  Password: <input type=\"password\" name=\"password\"><br>\
  <input type=\"submit\" value=\"Submit\">\
  </form>\
  </body>\
  </html>";
  server.send(200, "text/html", html);
}

void handleSubmit() {
  String ssid = server.arg("ssid");
  String password = server.arg("password");

  if (ssid.length() > 0 && password.length() > 0) {
    Serial.println("SSID: " + ssid);
    Serial.println("Password: " + password);

    for (int i = 0; i < 96; ++i) { EEPROM.write(i, 0); }
    Serial.println("Writing SSID to EEPROM...");
    for (int i = 0; i < ssid.length(); ++i) { EEPROM.write(i, ssid[i]); }
    Serial.println("Writing Password to EEPROM...");
    for (int i = 0; i < password.length(); ++i) { EEPROM.write(32 + i, password[i]); }
    EEPROM.commit();
    ESP.restart();
  } else {
    server.send(200, "text/html", "<html><body><h1>Invalid SSID or Password. Please try again.</h1></body></html>");
  }
}

void setup() {
  Serial.begin(115200);
  EEPROM.begin(512);

  if (restoreConfig()) {
    if (checkConnection()) {
      config.api_key = API_KEY;
      config.database_url = DATABASE_URL;

      auth.user.email = "thainguyenvan1042002@gmail.com";
      auth.user.password = "thaidz1234";
      
      config.token_status_callback = tokenStatusCallback;
      Firebase.begin(&config, &auth);
      Firebase.reconnectWiFi(true);
      return;
    }
  }

  WiFi.softAP(apSSID, apPassword);
  Serial.println("Access Point started");
  Serial.print("IP address: ");
  Serial.println(WiFi.softAPIP());

  server.on("/", handleRoot);
  server.on("/submit", HTTP_POST, handleSubmit);

  server.begin();
  Serial.println("HTTP server started");
}

void loop() {
  server.handleClient();
}
