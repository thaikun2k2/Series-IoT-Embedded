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
const char* apPassword = "66666666";
WebServer server(80);

// Firebase configuration
#define DATABASE_URL "https://project5-c5e9d-default-rtdb.firebaseio.com/"
#define API_KEY "AIzaSyBRYZvibVMMQHM4aCan6G7oxkrk9DQzEls"

// DHT11 sensor
#define DHTPIN 19
#define DHTTYPE DHT11

// Soil Moisture sensor
#define SOIL_MOISTURE_PIN 35

// Water level sensor
#define TRIG_PIN 5
#define ECHO_PIN 18
const int maxDistance = 10;
#define CheNang_PIN 33

// Relay pins
#define Relay2_PIN 12
#define Relay3_PIN 14
#define Relay4_PIN 27
#define CheNang_PIN 33

FirebaseData fbdo;
FirebaseAuth auth;
FirebaseConfig config;
FirebaseJson json;
WiFiClient wifiClient;

unsigned long sendDataPrevMillis = 0;
bool signupOK = false;

DHT dht(DHTPIN, DHTTYPE);

// LCD display I2C address
LiquidCrystal_I2C lcd(0x27, 16, 2);

// Relay states
bool relay2Status = false;
bool relay3Status = false;
bool relay4Status = false;
bool CheNang = true;

bool autoRelay3 = false;
bool autoRelay4 = false;

bool TimerRelay2 = false;
bool TimerRelay3 = false;
bool TimerRelay4 = false;
bool TimerCheNang = true;

// Control modes
int Mode4 = 0;  // 0: Manual, 1: Auto, 2: Timer
int Mode1 = 0;  // 0: Manual, 1: Auto, 2: Timer
int Mode2 = 0;  // 0: Manual, 1: Auto, 2: Timer
int Mode3 = 0;  // 0: Manual, 1: Auto, 2: Timer


// Khai báo các biến toàn cục
// Auto
int temperatureOn;
int temperatureOff;
int SoilMoistureOff;
int SoilMoistureOn;

//Timer
int hourOffRelay2;
int hourOnRelay2;
int minuteOffRelay2;
int minuteOnRelay2;

int hourOffRelay3;
int hourOnRelay3;
int minuteOffRelay3;
int minuteOnRelay3;

int hourOffRelay4;
int hourOnRelay4;
int minuteOffRelay4;
int minuteOnRelay4;

int hourOffTimerCheNang;
int hourOnTimerCheNang;
int minuteOffTimerCheNang;
int minuteOnTimerCheNang;

// NTP client for time synchronization
WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP, "europe.pool.ntp.org", 25200);

// Function to measure water level
void data_distance(int &distance) {
  unsigned long duration;
  digitalWrite(TRIG_PIN, LOW);
  delayMicroseconds(2);
  digitalWrite(TRIG_PIN, HIGH);
  delayMicroseconds(10);
  digitalWrite(TRIG_PIN, LOW);

  duration = pulseIn(ECHO_PIN, HIGH, 30000);
  if (duration == 0) {
    distance = maxDistance;
  } else {
    distance = int(duration / 2 / 29.1);
  }
  distance = constrain(distance, 0, maxDistance);
}

int calculateWaterPercentage(int distance) {
  return (maxDistance - distance) * 100 / maxDistance;
}
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

  //
  Serial.begin(115200);
  EEPROM.begin(512);
  Wire.begin(21,22); 
  lcd.init();
  lcd.backlight();
  lcd.setCursor(0, 0);
  lcd.print("Nguyen Van Thai");
  lcd.setCursor(0, 1);
  lcd.print("MSV: 10120707");
  delay(100);

  // Set relay pins as output
  pinMode(Relay2_PIN, OUTPUT);
  pinMode(Relay3_PIN, OUTPUT);
  pinMode(Relay4_PIN, OUTPUT);

  pinMode(CheNang_PIN, OUTPUT);

  // Set water level sensor pins
  pinMode(TRIG_PIN, OUTPUT);
  pinMode(ECHO_PIN, INPUT);

  // Initialize DHT sensor
  dht.begin();
  pinMode(SOIL_MOISTURE_PIN, INPUT);

  // Connect to Wi-Fi
  // connectWiFi();
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
  // Initialize NTP client
  timeClient.begin();
}

// // Connecting Wi-Fi
// void connectWiFi() {
//   Serial.print("Connecting to Wi-Fi");
//   WiFi.begin(WIFI_SSID, WIFI_PASSWORD);

//   int retry_count = 0;
//   while (WiFi.status() != WL_CONNECTED) {
//     delay(500);
//     Serial.print(".");
//     retry_count++;
//     if (retry_count > 10) {  // Try to reconnect for 15 seconds
//       Serial.println("Failed to connect to Wi-Fi");
//       ESP.restart();
//     }
//   }

//   Serial.println();
//   Serial.print("Connected with IP: ");
//   Serial.println(WiFi.localIP());
// }

void hienThiSerial_updateSettingsFromFirebase(){

  // Print values to serial monitor with labels
  Serial.println("Setting Common:");
  Serial.print("Mode1: ");
  Serial.println(Mode1);
  Serial.print("Mode2: ");
  Serial.println(Mode2);
  Serial.print("Mode3: ");
  Serial.println(Mode3);
  Serial.print("Mode4: ");
  Serial.println(Mode4);

  Serial.println("Setting Auto:");
  Serial.print("SoilMoistureOff: ");
  Serial.println(SoilMoistureOff);
  Serial.print("SoilMoistureOn: ");
  Serial.println(SoilMoistureOn);
  Serial.print("temperatureOff: ");
  Serial.println(temperatureOff);
  Serial.print("temperatureOn: ");
  Serial.println(temperatureOn);

  Serial.println("Setting Timer:");
  
  Serial.println("Light Timer:");

  Serial.print("  hourOffRelay2: ");
  Serial.println(hourOffRelay2);
  Serial.print("  hourOnRelay2: ");
  Serial.println(hourOnRelay2);
  Serial.print("  minuteOffRelay2: ");
  Serial.println(minuteOffRelay2);
  Serial.print("  minuteOnRelay2: ");
  Serial.println(minuteOnRelay2);

  Serial.println("Fan Timer:");
  Serial.print("  hourOffRelay3: ");
  Serial.println(hourOffRelay3);
  Serial.print("  hourOnRelay3: ");
  Serial.println(hourOnRelay3);
  Serial.print("  minuteOffRelay3: ");
  Serial.println(minuteOffRelay3);
  Serial.print("  minuteOnRelay3: ");
  Serial.println(minuteOnRelay3);

  Serial.println("MistSpray Timer:");
  Serial.print("  hourOffRelay4: ");
  Serial.println(hourOffRelay4);
  Serial.print("  hourOnRelay4: ");
  Serial.println(hourOnRelay4);
  Serial.print("  minuteOffRelay4: ");
  Serial.println(minuteOffRelay4);
  Serial.print("  minuteOnRelay4: ");
  Serial.println(minuteOnRelay4);

  Serial.println("Sunshape Timer:");
  Serial.print("  hourOffTimerCheNang: ");
  Serial.println(hourOffTimerCheNang);
  Serial.print("  hourOnTimerCheNang: ");
  Serial.println(hourOnTimerCheNang);
  Serial.print("  minuteOffTimerCheNang: ");
  Serial.println(minuteOffTimerCheNang);
  Serial.print("  minuteOnTimerCheNang: ");
  Serial.println(minuteOnTimerCheNang);
}

void Get_Settings_Firebase(){
    if (Firebase.RTDB.getJSON(&fbdo, "/Setting")) {
      FirebaseJson &json = fbdo.jsonObject();
      String jsonStr;
      json.toString(jsonStr, true);
      // Serial.print("STR Json: ");
      // Serial.println(jsonStr);
      StaticJsonDocument<1024> doc; 
      // Deserialize the JSON document
      DeserializationError error = deserializeJson(doc, jsonStr);
      if (error) {
        Serial.print(F("deserializeJson() failed: "));
        Serial.println(error.f_str());
        return;
      }
      Mode1 = doc["Common"]["Mode1"];
      Mode2 = doc["Common"]["Mode2"];
      Mode3 = doc["Common"]["Mode3"];
      Mode4 = doc["Common"]["Mode4"];

      // Extract values
      temperatureOn = doc["Auto"]["temperatureOn"];
      temperatureOff = doc["Auto"]["temperatureOff"];
      SoilMoistureOff = doc["Auto"]["SoilMoistureOff"];
      SoilMoistureOn = doc["Auto"]["SoilMoistureOn"];

      hourOffRelay2 = doc["Timer"]["Light"]["hourOffRelay2"];
      hourOnRelay2 = doc["Timer"]["Light"]["hourOnRelay2"];
      minuteOffRelay2 = doc["Timer"]["Light"]["minuteOffRelay2"];
      minuteOnRelay2 = doc["Timer"]["Light"]["minuteOnRelay2"];

      hourOffRelay3 = doc["Timer"]["Fan"]["hourOffRelay3"];
      hourOnRelay3 = doc["Timer"]["Fan"]["hourOnRelay3"];
      minuteOffRelay3 = doc["Timer"]["Fan"]["minuteOffRelay3"];
      minuteOnRelay3 = doc["Timer"]["Fan"]["minuteOnRelay3"];

      hourOffRelay4 = doc["Timer"]["MistSpray"]["hourOffRelay4"];
      hourOnRelay4 = doc["Timer"]["MistSpray"]["hourOnRelay4"];
      minuteOffRelay4 = doc["Timer"]["MistSpray"]["minuteOffRelay4"];
      minuteOnRelay4 = doc["Timer"]["MistSpray"]["minuteOnRelay4"];

      hourOffTimerCheNang = doc["Timer"]["Sunshape"]["hourOffTimerCheNang"];
      hourOnTimerCheNang = doc["Timer"]["Sunshape"]["hourOnTimerCheNang"];
      minuteOffTimerCheNang = doc["Timer"]["Sunshape"]["minuteOffTimerCheNang"];
      minuteOnTimerCheNang = doc["Timer"]["Sunshape"]["minuteOnTimerCheNang"];

      hienThiSerial_updateSettingsFromFirebase();
    }
}

void Auto_mode() {
  float temperature = dht.readTemperature();
  int soilMoisture = analogRead(SOIL_MOISTURE_PIN);
  float doamdat = map(soilMoisture, 100, 4095, 0, 100);
    // Relay3
    if (temperature <= temperatureOn) {
      autoRelay3 = true;
    } else if (temperature >= temperatureOff) {
      autoRelay3 = false;
    } else {
      autoRelay3 = false;
    }

    // Relay4
    if (doamdat <= SoilMoistureOn) {
      autoRelay4 = true;
    } else if (doamdat >= SoilMoistureOff) {
      autoRelay4 = false;
    }else {
      autoRelay4 = false;
    }
}

void loop() { 
  Get_Settings_Firebase();
  // Set_Settings_Firebase();
  server.handleClient();

  // Hiển thị thời gian
  timeClient.update();
    // Lấy thời gian
  int hour = timeClient.getHours();
  int minute = timeClient.getMinutes();
  int second = timeClient.getSeconds();

  Serial.print("Time: ");
  Serial.print(hour);
  Serial.print(":");
  if (minute < 10) {
    Serial.print("0");
  }
  Serial.print(minute);
  Serial.print(":");
  if (second < 10) {
    Serial.print("0");
  }
  Serial.println(second);

  // Get sensor readings
  float temperature = dht.readTemperature();
  float humidity = dht.readHumidity();
  int soilMoisture = analogRead(SOIL_MOISTURE_PIN);
  float doamdat = map(soilMoisture, 100, 4095, 0, 100);

  int distance;
  data_distance(distance);
  int waterPercentage = calculateWaterPercentage(distance);

  // Print sensor readings to the Serial Monitor
  Serial.print("Temperature: ");
  Serial.println(temperature);
  Serial.print("Humidity: ");
  Serial.println(humidity);
  Serial.print("Soil Moisture: ");
  Serial.println(doamdat);
  Serial.print("Water Percentage: ");
  Serial.println(waterPercentage);

  // Hiển thị thông tin lên LCD
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("T:");
  lcd.print(temperature);
  lcd.print("C ");
  lcd.print(" S:");
  lcd.print(doamdat);

  lcd.setCursor(0, 1); // Đặt con trỏ hiển thị tại hàng 2, cột 1
  lcd.print("H:"); // In "H:"
  lcd.print(humidity); // In độ ẩm
  lcd.print("%  ");
  lcd.print("W:"); // In "W:"
  lcd.print(waterPercentage); // In tỷ lệ nước
  lcd.print("%");

  if (Firebase.ready() && (millis() - sendDataPrevMillis > 5000 || sendDataPrevMillis == 0)) { // 5000 - 5s gửi dũ liệu 1 lần lên firebase 
        // Update water level on Firebase
    if (Firebase.RTDB.setInt(&fbdo, "Sensor/WaterLevel", waterPercentage)) {
      Serial.println("Successful WRITE to: Sensor/WaterLevel: " + String(waterPercentage));
    } else {
      Serial.println("FAILED WRITE: Sensor/WaterLevel" + fbdo.errorReason());
    }

    // Update temperature on Firebase
    if (Firebase.RTDB.setFloat(&fbdo, "Sensor/Temperature", temperature)) {
      Serial.println("Successful WRITE to: Sensor/Temperature: " + String(temperature));
    } else {
      Serial.println("FAILED WRITE: Sensor/Temperature" + fbdo.errorReason());
    }

    // Update humidity on Firebase
    if (Firebase.RTDB.setFloat(&fbdo, "Sensor/Humidity", humidity)) {
      Serial.println("Successful WRITE to: Sensor/Humidity: " + String(humidity));
    } else {
      Serial.println("FAILED WRITE: Sensor/Humidity" + fbdo.errorReason());
    }

    // Update soil moisture on Firebase
    if (Firebase.RTDB.setInt(&fbdo, "Sensor/SoilMoisture", doamdat)) {
      Serial.println("Successful WRITE to: Sensor/SoilMoisture: " + String(doamdat));
    } else {
      Serial.println("FAILED WRITE: Sensor/SoilMoisture" + fbdo.errorReason());
    }
 
  }
  if(Mode4 == 0) {
        
    if(Firebase.RTDB.getBool(&fbdo, "/Device/Relay4")) {
      if(fbdo.dataType() == "boolean") {
        relay4Status = fbdo.boolData();
        Serial.print("\nSuccessful READ from: " + fbdo.dataPath() + ": " + relay4Status + " (" + fbdo.dataType() + ") ");
        digitalWrite(Relay4_PIN, relay4Status);
      }
    }else{
      Serial.println("FAILED: /Device/Relay4" + fbdo.errorReason());
    }

  }else if (Mode4 == 1) {
    // Auto mode
    Auto_mode();
    digitalWrite(Relay4_PIN, autoRelay4);

  }else if (Mode4 == 2) {    
    if (hour == hourOnRelay4) {
      if(minute == minuteOnRelay4){
        TimerRelay4 = true;
      }
    } else if (hour == hourOffRelay4) {
      if(minute == minuteOffRelay4) {
        TimerRelay4 = false;
      }
    }
    digitalWrite(Relay4_PIN, TimerRelay4);
  }

  if(Mode3 == 0) {

    if (Firebase.RTDB.getBool(&fbdo, "/Device/CheNang")) {
      if (fbdo.dataType() == "boolean") {
        bool CheNang = fbdo.boolData();
        Serial.print("\nSuccessful READ from: " + fbdo.dataPath() + ": " + (CheNang ? "true" : "false") + " (" + fbdo.dataType() + ") ");
        if (!CheNang) {
          digitalWrite(CheNang_PIN, HIGH);
        } else {
          digitalWrite(CheNang_PIN, LOW);
        }  
      }
    } else {
      Serial.println("FAILED: /Device/CheNang " + fbdo.errorReason());
    }
  } else if(Mode3 == 2){
    if (hour == hourOnTimerCheNang ) {
      if(minute == hourOffTimerCheNang){
        digitalWrite(CheNang_PIN, HIGH);
      }
    } else if (hour == hourOffTimerCheNang) {
      if(minute == minuteOffTimerCheNang) {
        digitalWrite(CheNang_PIN, LOW);
      }
    }
  }

  if(Mode2 == 0) {
    if(Firebase.RTDB.getBool(&fbdo, "/Device/Relay3")) {
      if(fbdo.dataType() == "boolean") {
        relay3Status = fbdo.boolData();
        Serial.print("\nSuccessful READ from: " + fbdo.dataPath() + ": " + relay3Status + " (" + fbdo.dataType() + ") ");
        digitalWrite(Relay3_PIN, relay3Status);
      }
    }else{
      Serial.println("FAILED: /Device/Relay3" + fbdo.errorReason());
    }
  } else if (Mode2 == 1) {
    // Auto mode
    Auto_mode();
    digitalWrite(Relay3_PIN, autoRelay3);
  }else if (Mode2 == 2) {
        // Timer Light
    //Timer Fan
    if (hour == hourOnRelay3) {
      if(minute == minuteOnRelay3){
        TimerRelay3 = true;
      }
    } else if (hour == hourOffRelay3) {
      if(minute == minuteOffRelay3) {
        TimerRelay3 = false;
      }
    }
    digitalWrite(Relay3_PIN, TimerRelay3);
  }
        // Update relay states based on control mode
  if (Mode1 == 0) {
    // Manual mode
    if(Firebase.RTDB.getBool(&fbdo, "/Device/Relay2")) {
      if(fbdo.dataType() == "boolean") {
        relay2Status = fbdo.boolData();
        Serial.print("\nSuccessful READ from: " + fbdo.dataPath() + ": " + relay2Status + " (" + fbdo.dataType() + ") ");
        digitalWrite(Relay2_PIN, relay2Status);
      }
    }else{
      Serial.println("FAILED: /Device/Relay2" + fbdo.errorReason());
    }
  } else if (Mode1 == 2) {
        // Timer Light
    if (hour == hourOnRelay2) {
      if (minute == minuteOnRelay2) {
        TimerRelay2 = true;
      }
    } else if (hour == hourOffRelay2) {
      if(minute == minuteOffRelay2) {
        TimerRelay2 = false;
      }
    }
    digitalWrite(Relay2_PIN, TimerRelay2);

  }
}