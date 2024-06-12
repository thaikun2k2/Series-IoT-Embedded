#include <EEPROM.h>
#include <Wire.h>
//#include <ESP8266WiFi.h>

#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <FirebaseESP8266.h>

#include <ESP8266WebServer.h>
//#include <Firebase_ESP_Client.h>
#include <addons/TokenHelper.h>
#include <addons/RTDBHelper.h>
#define API_KEY "AIzaSyDnpUyQ-m1XG0ChkbBdtc9yDJwLlDEw2RI"
#define REFERENCE_URL "https://test3-5a96d-default-rtdb.firebaseio.com"
FirebaseData fbdo;
FirebaseData fbdo1;
FirebaseAuth auth;
FirebaseConfig config;

#include <NTPClient.h>
#include <WiFiUdp.h>
WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP, "europe.pool.ntp.org", 25200);

#include <SimpleDHT.h>
int pinDHT22 = 2;
SimpleDHT22 dht22(pinDHT22);

#include <ArduinoJson.h> 

#define pumpPin  D0
#define TRIG_PIN D1
#define ECHO_PIN D2
#define rainPin  D3
#define lightPin  D5
#define soilMoisturePin  A0   
int Doamdat;
#define SWITCH1_PIN D6
#define SWITCH2_PIN 3 
#define int1 D7

#define int2 D8



int gio, phut;

const char* apSSID = "ESP8266_AP";
const char* apPassword = "12345678";
ESP8266WebServer server(80); 

bool signupOK = false;
bool scheduleChanged = false;
int startHour, startMinute, stopHour, stopMinute;
bool pumpState = false;
bool ena = 0, k =0;

unsigned long sendDataPrevMillis = 0;

unsigned long previousMillisGetData = 0;
unsigned long previousMillisRainCheck = 0;
const long intervalGetData = 5000;
const long intervalRainCheck = 1000;
const int maxDistance = 10;

//Gửi dữ liệu mỗi 5s
unsigned long previousMillis = 0;
unsigned long previousMillis1 = 0;
const long interval = 5000;

//Đọc dữ liệu từ Firebase
const long interval1 = 1000;


unsigned long t1 = 0;
int GT =0, GTS = 0;
byte giobd, phutbd, giokt, phutkt, kiemtra;

String bomCambienCheck;
int bomCambienDoamBatBom;
String bomHengioCheck;
int bomHengioStartHour;
int bomHengioStartMinute;
int bomHengioStopHour;
int bomHengioStopMinute;
String bomThucongCheck;

int denDosangden;
String denHengioCheck;
int denHengioStartHour;
int denHengioStartMinute;
int denHengioStopHour;
int denHengioStopMinute;
String denThucongCheck;

String maicheHengioCheck;
int maicheHengioStartHour;
int maicheHengioStartMinute;
int maicheHengioStopHour;
int maicheHengioStopMinute;
String maicheThucongCheck;

int giobdtuoicay, giokttuoicay, giobdsuoiam, gioktsuoiam, giobdmaiche, gioktmaiche;
int phutbdtuoicay , phutkttuoicay, phutbdsuoiam, phutktsuoiam, phutbdmaiche, phutktmaiche;

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
    WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
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

void checkRainSensor() {
  int rainValue = digitalRead(rainPin);
  Serial.println(rainValue);

  if (rainValue == 0) {
    digitalWrite(pumpPin, LOW);
  } else {
    Serial.println("No rain detected.");
  }
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


  pinMode(pumpPin, OUTPUT);
  digitalWrite(pumpPin, LOW);
  pinMode(A0, INPUT);
  pinMode(rainPin, INPUT);
  pinMode(lightPin, OUTPUT);
  analogWrite(lightPin, 0);
  pinMode(TRIG_PIN, OUTPUT); // Thiết lập chân Trig là OUTPUT
  pinMode(ECHO_PIN, INPUT); // Thiết lập chân Echo là INPUT
  pinMode(SWITCH1_PIN, INPUT_PULLUP);
  pinMode(SWITCH2_PIN, INPUT_PULLUP);
  pinMode(int1, OUTPUT);
  digitalWrite(int1,LOW);
  pinMode(int2, OUTPUT);

  if (restoreConfig()) {
    if (checkConnection()) {
      config.api_key = API_KEY;
      config.database_url = REFERENCE_URL;

      /* Assign the user sign in credentials */
      auth.user.email = "vanheoxinhdz2002@gmail.com";
      auth.user.password = "kenkaneki2002";
      
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

  timeClient.begin();


}

//void readTemperatureHumidity(float& temperature, float& humidity) {
//  // Đọc dữ liệu từ cảm biến
//  humidity = dht.readHumidity();     // Độ ẩm
//  temperature = dht.readTemperature(); // Nhiệt độ
//  
//  // Kiểm tra nếu không đọc được dữ liệu từ cảm biến
//  if (isnan(humidity) || isnan(temperature)) {
//    Serial.println("Failed to read from DHT sensor!");
//    return;
//  }
//}

int readSoilMoisture() {
  int soilMoistureValue = analogRead(soilMoisturePin);
  // Chuyển đổi giá trị analog thành phần trăm độ ẩm đất
  int Doamdat = map(soilMoistureValue, 0, 1023, 0, 100);
  return Doamdat;
}

void data_distance(int &distance) {
  unsigned long duration;
  digitalWrite(TRIG_PIN, LOW);
  delayMicroseconds(2);
  digitalWrite(TRIG_PIN, HIGH);
  delayMicroseconds(10);
  digitalWrite(TRIG_PIN, LOW);

  duration = pulseIn(ECHO_PIN, HIGH, 30000); // Đọc tín hiệu từ chân echo, thời gian chờ tối đa 30ms
  if (duration == 0) {
    distance = maxDistance;
  } else {
    distance = int(duration / 2 / 29.1); // Chuyển đổi thời gian sang khoảng cách
  }
  distance = constrain(distance, 0, maxDistance);
  Serial.print("Khoảng cách: ");
  Serial.print(distance);
  Serial.println("cm");
}

int calculateWaterPercentage(int distance) {
  int amountofwater = map(distance, 0, maxDistance, 100, 0);
  return amountofwater;
}

void DisplaySrialDataDive_readFromFriebase(){
  Serial.println("Bom:");
  Serial.print("  Cambien Check: "); Serial.println(bomCambienCheck);
  Serial.print("  Cambien DoamBatBom: "); Serial.println(bomCambienDoamBatBom);
  Serial.print("  Hengio Check: "); Serial.println(bomHengioCheck);
  Serial.print("  Hengio StartHour: "); Serial.println(bomHengioStartHour);
  Serial.print("  Hengio StartMinute: "); Serial.println(bomHengioStartMinute);
  Serial.print("  Hengio StopHour: "); Serial.println(bomHengioStopHour);
  Serial.print("  Hengio StopMinute: "); Serial.println(bomHengioStopMinute);
  Serial.print("  Thucong Check: "); Serial.println(bomThucongCheck);

  Serial.println("Den:");
  Serial.print("  Dosangden: "); Serial.println(denDosangden);
  Serial.print("  Hengio Check: "); Serial.println(denHengioCheck);
  Serial.print("  Hengio StartHour: "); Serial.println(denHengioStartHour);
  Serial.print("  Hengio StartMinute: "); Serial.println(denHengioStartMinute);
  Serial.print("  Hengio StopHour: "); Serial.println(denHengioStopHour);
  Serial.print("  Hengio StopMinute: "); Serial.println(denHengioStopMinute);
  Serial.print("  Thucong Check: "); Serial.println(denThucongCheck);

  Serial.println("Maiche:");
  Serial.print("  Hengio Check: "); Serial.println(maicheHengioCheck);
  Serial.print("  Hengio StartHour: "); Serial.println(maicheHengioStartHour);
  Serial.print("  Hengio StartMinute: "); Serial.println(maicheHengioStartMinute);
  Serial.print("  Hengio StopHour: "); Serial.println(maicheHengioStopHour);
  Serial.print("  Hengio StopMinute: "); Serial.println(maicheHengioStopMinute);
  Serial.print("  Thucong Check: "); Serial.println(maicheThucongCheck);
}

void getDataDiveFromFirebase(){ 
  unsigned long currentMillis1 = millis();
  if(currentMillis1 - previousMillis1 >= interval1){
    previousMillis1 = currentMillis1; 
    String path = "/data/Device/";
    if (Firebase.getJSON(fbdo1, path.c_str())) {
      FirebaseJson &json = fbdo1.jsonObject();
      String jsonStr;
      json.toString(jsonStr, true);
      StaticJsonDocument<1024> doc;
      DeserializationError error = deserializeJson(doc, jsonStr);
      if (error) {
        Serial.print(F("deserializeJson() failed: ")); Serial.println(error.f_str());
        return;
      } 
           // Bóc tách dữ liệu từ JSON
      bomCambienCheck = doc["Bom"]["Cambien"]["Check"].as<const char*>();
      bomCambienDoamBatBom = doc["Bom"]["Cambien"]["DoamBatBom"];
      bomHengioCheck = doc["Bom"]["Hengio"]["Check"].as<const char*>();
      bomHengioStartHour = doc["Bom"]["Hengio"]["StartHour"];
      bomHengioStartMinute = doc["Bom"]["Hengio"]["StartMinute"];
      bomHengioStopHour = doc["Bom"]["Hengio"]["StopHour"];
      bomHengioStopMinute = doc["Bom"]["Hengio"]["StopMinute"];
      bomThucongCheck = doc["Bom"]["Thucong"]["Check"].as<const char*>();
    
      denDosangden = doc["Den"]["Dosangden"];
      denHengioCheck = doc["Den"]["Hengio"]["Check"].as<const char*>();
      denHengioStartHour = doc["Den"]["Hengio"]["StartHour"];
      denHengioStartMinute = doc["Den"]["Hengio"]["StartMinute"];
      denHengioStopHour = doc["Den"]["Hengio"]["StopHour"];
      denHengioStopMinute = doc["Den"]["Hengio"]["StopMinute"];
      denThucongCheck = doc["Den"]["Thucong"]["Check"].as<const char*>();
    
      maicheHengioCheck = doc["Maiche"]["Hengio"]["Check"].as<const char*>();
      maicheHengioStartHour = doc["Maiche"]["Hengio"]["StartHour"];
      maicheHengioStartMinute = doc["Maiche"]["Hengio"]["StartMinute"];
      maicheHengioStopHour = doc["Maiche"]["Hengio"]["StopHour"];
      maicheHengioStopMinute = doc["Maiche"]["Hengio"]["StopMinute"];
      maicheThucongCheck = doc["Maiche"]["Thucong"]["Check"].as<const char*>(); 

      DisplaySrialDataDive_readFromFriebase(); 
      
    } else {
      Serial.print("Failed to read data: ");
      Serial.println(fbdo1.errorReason());
    }
  }
}

// const char* bomCambienCheck;
// int bomCambienDoamBatBom;
// const char* bomHengioCheck;
// int bomHengioStartHour;
// int bomHengioStartMinute;
// int bomHengioStopHour;
// int bomHengioStopMinute;
// const char* bomThucongCheck;

// int denDosangden;
// const char* denHengioCheck;
// int denHengioStartHour;
// int denHengioStartMinute;
// int denHengioStopHour;
// int denHengioStopMinute;
// const char* denThucongCheck;

// const char* maicheHengioCheck;
// int maicheHengioStartHour;
// int maicheHengioStartMinute;
// int maicheHengioStopHour;
// int maicheHengioStopMinute;
// const char* maicheThucongCheck;

String bom; 

void Tuoicay(){
  Serial.print("Giá trị của bomThucongCheck/////////////////////////////////: ");
  Serial.println(bomThucongCheck);

  if (bomCambienCheck == "Off" && bomHengioCheck == "Off"){
    if(bomThucongCheck == "On"){
      Serial.println("Thủ công bật bơm");
      digitalWrite(pumpPin, HIGH);
    }else{
      Serial.println("Thủ công tắt bơm");
      digitalWrite(pumpPin, LOW);
    }
  }
  
  if (bomThucongCheck == "Off" && bomHengioCheck == "Off"){
    if(bomCambienCheck == "On"){
      if(Doamdat < bomCambienDoamBatBom){
        Serial.println("////////////////////////////   check bom 2 ");
        digitalWrite(pumpPin, HIGH);
      }
      else if(Doamdat > bomCambienDoamBatBom){
        Serial.println("////////////////////////////   check bom 3 ");
        digitalWrite(pumpPin, LOW);
      }
    }
  }

  if (bomCambienCheck == "Off" && bomThucongCheck == "Off"){
    if(bomHengioCheck == "On"){
      if(gio == bomHengioStartHour  && phut == bomHengioStartMinute){
        Serial.println("////////////////////////////   check bom 4 ");
        digitalWrite(pumpPin, HIGH);
      }
      if(gio == bomHengioStopHour  && phut == bomHengioStopMinute){
        Serial.println("////////////////////////////   check bom 5 ");
        digitalWrite(pumpPin, LOW);
      }
    }
  }

}


void Suoiam(){
  Serial.print("Giá trị của DenThucongCheck/////////////////////////////////: ");
  Serial.println(denThucongCheck);

  if (denHengioCheck == "Off"){
    if(denThucongCheck == "On"){
      Serial.println("Thủ công bật den");
      analogWrite(lightPin, 255);
    }else{
      Serial.println("Thủ công tắt den");
      analogWrite(lightPin, 0);
    }
  }
  

  if (denThucongCheck == "Off"){
    if(denHengioCheck == "On"){
      if(gio == denHengioStartHour  && phut == denHengioStartMinute){
        Serial.println("////////////////////////////   check den 4 ");
        analogWrite(lightPin, 255);
      }
      if(gio == denHengioStopHour  && phut == denHengioStopMinute){
        Serial.println("////////////////////////////   check den 5 ");
        analogWrite(lightPin, 0);
      }
    }
  }

}


void loop() {

  server.handleClient();
  if (!timeClient.update()) {
    timeClient.forceUpdate();
  }
  // Lấy thời gian
  time_t now = timeClient.getEpochTime();
  struct tm *timeinfo = localtime(&now);

  gio = timeinfo->tm_hour;
  phut = timeinfo->tm_min;
  Serial.print("Gio hien tại: ");Serial.println(gio);
  Serial.print("Phut hien tại: ");Serial.println(phut);


  // Định dạng thời gian
  char formattedTime[25];
  strftime(formattedTime, sizeof(formattedTime), "%H:%M:%S", timeinfo);
  // Định dạng ngày
  char formattedDate[25];
  strftime(formattedDate, sizeof(formattedTime), "%d-%m-%Y", timeinfo);

  // In ra Serial Monitor
  Serial.println(formattedDate);
  Serial.println(formattedTime);

  float temperature = 0;
  float humidity = 0;
  int err = SimpleDHTErrSuccess;
  if ((err = dht22.read2(&temperature, &humidity, NULL)) != SimpleDHTErrSuccess) {
    Serial.print("Read DHT22 failed, err="); Serial.print(SimpleDHTErrCode(err));
    Serial.print(","); Serial.println(SimpleDHTErrDuration(err)); delay(2000);
    temperature = 1;
    humidity = 1;
  }
  
  Serial.print("Sample OK: ");
  Serial.print((int)temperature); Serial.print(" *C, ");
  Serial.print((int)humidity); Serial.println(" RH%");

  int soilMoistureValue = analogRead(soilMoisturePin);
  int Doamdat = (1024 - soilMoistureValue) / 1023.0 * 100;
  // Đọc dữ liệu cảm biến mưa
  int rainValue = digitalRead(rainPin);
  Serial.print("value Rain: ");
  Serial.println(rainValue);

  // Đọc dữ liệu cảm biến siêu âm

  //Gửi nhiệt độ, độ ẩm, độ ẩm đất, trạng thái mưa, mực nước mỗi 5s
  unsigned long currentMillis = millis();
  if (currentMillis - previousMillis >= interval) {

    FirebaseJson json;

    // Dữ liệu DHT11 TimeOld
    json.set("/Do am", humidity);
    json.set("/Nhiet do", temperature);
    
    // Dữ liệu Soli TimeOld
    json.set("/Do am dat", Doamdat);

    // Dữ liệu Rain
    json.set("/Mua", rainValue);

    // Gửi dữ liệu lên Firebase
    String path = "/data/Sensor/" + String(formattedDate) + "/" + String(formattedTime);
    if (Firebase.set(fbdo1, path, json)) {
      Serial.println("Dữ liệu đã được gửi thành công");
    } else {
      Serial.println("Gửi dữ liệu thất bại");
      Serial.println("LÝ DO: " + fbdo1.errorReason());
    }

  
  
  previousMillis = currentMillis;
} 
  getDataDiveFromFirebase();

  Tuoicay();

  Suoiam();

  DieuKhien_MaiChe();

}

void maiChe_Keora(){
  digitalWrite(int1,HIGH);
  
}

void maiChe_Keovao(){
  digitalWrite(int1,LOW);
  
}



void DieuKhien_MaiChe(){
  if(digitalRead(rainPin) == 0){ // Khi có mưa cảm biến bằng 0
    maiChe_Keovao();
    Serial.println("Kéo vào khi trời có mưa");
  }else {  // khi không có mưa hoạt động theo ý người điều khiển

    ///  chế độ thủ công
    if(maicheHengioCheck == "Off"){
      if(maicheThucongCheck == "On"){
        Serial.println("Mở mái che thủ công"); 
        maiChe_Keora();
        
      }else if (maicheThucongCheck == "Off"){
        Serial.println("Đóng mái che thủ công"); 
        maiChe_Keovao();
      }
    }


    //Chế độ theo thời gian
    if(maicheThucongCheck == "Off"){
      if(maicheHengioCheck == "On"){
        if(gio == maicheHengioStartHour  && phut == maicheHengioStartMinute){
          Serial.println("Hẹn giờ mở mái che");
          maiChe_Keora();
        }
        if(gio == maicheHengioStopHour  && phut == maicheHengioStopMinute){
          Serial.println("Hẹn giờ đóng mái che");
          maiChe_Keovao();
        }
      }
    }


  } 
}














