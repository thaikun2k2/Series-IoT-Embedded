#include <LiquidCrystal_I2C.h>

#include <WiFi.h>
#include <FirebaseESP32.h>
#include <Adafruit_Sensor.h>
#include <DHT.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>


#define FIREBASE_HOST "https://project4-27d70-default-rtdb.firebaseio.com/"
#define FIREBASE_AUTH "JK9mYBWsoY35lCJihC68ky613MXlCKXqWGe1KnoK"

#define WIFI_SSID "HuyHoang"
#define WIFI_PASSWORD "12345678"

#define DHTPIN 25
#define DHTTYPE    DHT11
DHT dht(DHTPIN, DHTTYPE); 

#define RELAY_PIN 13

#define LCD_COLS 16
#define LCD_ROWS 2


LiquidCrystal_I2C lcd(0x3F, 16, 2);


FirebaseData firebaseData;
WiFiClient wifiClient;

bool relayState = false;  // Trạng thái relay (tắt)

void setup() {
    Serial.begin(115200);
    
  // Kết nối đến mạng WiFi
    WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
    while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println();
  Serial.print("WiFi connected, IP address: ");
  Serial.println(WiFi.localIP());

  // Khởi tạo kết nối Firebase
  Firebase.begin(FIREBASE_HOST, FIREBASE_AUTH);

  // Khởi tạo màn hình LCD

  Wire.begin(21,22);               //Thiết lập chân kết nối I2C (SDA,SCL);
  lcd.init();                      //Khởi tạo LCD
  lcd.clear();                     //Xóa màn hình
  lcd.backlight();                 //Bật đèn nền
  lcd.setCursor(0,0);              //Đặt vị trí muốn hiển thị ô thứ 1 trên dòng 1
//  lcd.begin(LCD_COLS, LCD_ROWS);
//  lcd.setCursor(0, 0);
  lcd.print("Nguyen Van Thai");
  lcd.setCursor(0, 1);
  lcd.print("MSV: 10120707");
  // Đặt chế độ OUTPUT cho chân Relay
  pinMode(RELAY_PIN, OUTPUT);

  // Khởi tạo cảm biến DHT11
  dht.begin();
  delay(200);
}

void loop() {
  // Đọc dữ liệu từ DHT11
  float t = dht.readTemperature();
  float h = dht.readHumidity();



  if (isnan(h) || isnan(t)) {
    Serial.println("Failed to read from DHT sensor!");
    return;
  }
  
  // Gửi dữ liệu lên Firebase
  Firebase.setInt(firebaseData, "/Nhiệt độ", t);
  Firebase.setInt(firebaseData, "/Độ ẩm", h);
  Firebase.pushInt(firebaseData, "/timestamp", millis());

  // Hiển thị dữ liệu trên màn hình LCD
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("T: ");
  lcd.print(t);
  lcd.print("C");
  lcd.print("   AQ:");
  lcd.setCursor(0, 1);
  lcd.print("H: ");
  lcd.print(h);
  lcd.print("% ");
  lcd.print(airQuality);

  Serial.print("Nhiet do: ");
  Serial.print(t);
  Serial.print("*C  ");
  Serial.print("Do am: ");
  Serial.print(h);
  Serial.println("%");

  Firebase.setInt(firebaseData, "/Trạng thái quạt", relayState);
  Firebase.pushInt(firebaseData, "/timestamp", millis());
  // Đọc trạng thái từ Firebase và điều khiển relay
  Firebase.getString(firebaseData, "/relay", "");
  if (firebaseData.dataType() == "string")
  {
    if (firebaseData.stringData() == "on")
    {
      // Bật relay
      relayState = true;
      digitalWrite(RELAY_PIN, HIGH);
    }
    else if (firebaseData.stringData() == "off")
    {
      // Tắt relay
      relayState = false;
      digitalWrite(RELAY_PIN, LOW);
    }
  }

  // Kiểm tra trạng thái relay để bật/tắt relay khi nhấn nút
  if (digitalRead(RELAY_PIN) == HIGH && relayState == true)
  {
    // Relay đang được bật, nhưng relayState = true => giữ relay bật
    digitalWrite(RELAY_PIN, HIGH);
  }
  else if (digitalRead(RELAY_PIN) == LOW && relayState == false)
  {
    // Relay đang tắt, nhưng relayState = false => giữ relay tắt
    digitalWrite(RELAY_PIN, LOW);
  }


//  if (Firebase.getBool(firebaseData, "/relay")) {
//    digitalWrite(RELAY_PIN, HIGH);  // Bật relay
//  } else {
//    digitalWrite(RELAY_PIN, LOW);   // Tắt relay
//  }

  delay(1000);
}
