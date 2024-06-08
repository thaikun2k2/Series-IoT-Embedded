#include <WiFi.h>
#include<Firebase_ESP_Client.h>
#include"addons/TokenHelper.h"
#include"addons/RTDBHelper.h"

#define WIFI_SSID "Discipline"
#define WIFI_PASSWORD "12345666"
#define API_KEY "AIzaSyACCEf-vwqItlM0fE0ChqKLJfLqqAdhSNk"
#define DATABASE_URL "https://project5-a83b2-default-rtdb.firebaseio.com/"

#define Relay1_PIN 13
#define Relay2_PIN 12
#define Relay3_PIN 14
#define Relay4_PIN 27


const int freg = 5000;
const int resolution = 8;


FirebaseData fbdo;
FirebaseAuth auth;
FirebaseConfig config;

unsigned long sendDataPrevMillis = 0;
bool signupOK = false;


bool relay1Status = false;
bool relay2Status = false;
bool relay3Status = false;
bool relay4Status = false;


void setup() {
  pinMode(Relay1_PIN, OUTPUT);
  pinMode(Relay2_PIN, OUTPUT);
  pinMode(Relay3_PIN, OUTPUT);
  pinMode(Relay4_PIN, OUTPUT);



  Serial.begin(115200);
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  Serial.print("Connecting to WI-Fi!");
  while(WiFi.status() != WL_CONNECTED) {
    Serial.print("."); delay(100); 
  }
  Serial.println();
  Serial.print("Connected with IP: ");
  Serial.println(WiFi.localIP());
  Serial.println();

  config.api_key = API_KEY;
  config.database_url = DATABASE_URL;
  if(Firebase.signUp(&config, &auth, "", "")) {
    Serial.println("signUp OK");
    signupOK = true;
  }else{
    Serial.println(config.signer.signupError.message.c_str());
  }

  config.token_status_callback = tokenStatusCallback;
  Firebase.begin(&config, &auth);
  Firebase.reconnectWiFi(true);

}

void loop() {
  if(Firebase.ready() && signupOK && (millis() - sendDataPrevMillis > 5000 || sendDataPrevMillis == 0)) {
    sendDataPrevMillis = millis();

    if(Firebase.RTDB.getBool(&fbdo, "/Relay/Relay1")) {
      if(fbdo.dataType() == "boolean") {
        relay1Status = fbdo.boolData();
        Serial.print("Successful READ from: " + fbdo.dataPath() + ": " + relay1Status + " (" + fbdo.dataType() + ") ");
        digitalWrite(Relay1_PIN, relay1Status);
      }
    }else{
      Serial.println("FAILED: " + fbdo.errorReason());
    }

    if(Firebase.RTDB.getBool(&fbdo, "/Relay/Relay2")) {
      if(fbdo.dataType() == "boolean") {
        relay2Status = fbdo.boolData();
        Serial.print("Successful READ from: " + fbdo.dataPath() + ": " + relay2Status + " (" + fbdo.dataType() + ") ");
        digitalWrite(Relay2_PIN, relay2Status);
      }
    }else{
      Serial.println("FAILED: " + fbdo.errorReason());
    }

    if(Firebase.RTDB.getBool(&fbdo, "/Relay/Relay3")) {
      if(fbdo.dataType() == "boolean") {
        relay3Status = fbdo.boolData();
        Serial.print("Successful READ from: " + fbdo.dataPath() + ": " + relay3Status + " (" + fbdo.dataType() + ") ");
        digitalWrite(Relay3_PIN, relay3Status);
      }
    }else{
      Serial.println("FAILED: " + fbdo.errorReason());
    }

    if(Firebase.RTDB.getBool(&fbdo, "/Relay/Relay4")) {
      if(fbdo.dataType() == "boolean") {
        relay4Status = fbdo.boolData();
        Serial.print("Successful READ from: " + fbdo.dataPath() + ": " + relay4Status + " (" + fbdo.dataType() + ") ");
        digitalWrite(Relay4_PIN, relay4Status);
      }
    }else{
      Serial.println("FAILED: " + fbdo.errorReason());
    }
  }
}
