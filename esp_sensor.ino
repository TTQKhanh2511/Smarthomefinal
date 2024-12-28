

#include <BH1750.h>
#include <Wire.h>
#include <DHT.h>
#include <FirebaseESP32.h>
#include <addons/RTDBHelper.h>
#include <WiFi.h>
#include <WiFiClient.h>
// #include <WiFiManager.h>
#include "DHT.h"


//--------------------------------------------------Sensor--------------------------------------------------------//
#define DHTPIN 25
#define DHTTYPE DHT11

const int trig2 = 18;
const int echo2 = 19;
const int trig1 = 12;
const int echo1 = 13;

unsigned long thoigian1;
unsigned long thoigian2;

int kccb1bandau;
int kccb2bandau;
int khoangcach1;
int khoangcach2;
int prevKhoangcach1;
int prevKhoangcach2;

int timeout = 5;

int songuoitrongphong;
String hangdoi = "";
int timeoutcounter = 0;

#define WIFI_SSID "NEYU"
#define WIFI_PASSWORD "0987654321"

BH1750 lightMeter;
DHT dht(DHTPIN, DHTTYPE);

#define DATABASE_URL "https://project1-ee980-default-rtdb.firebaseio.com"
#define API_KEY "AIzaSyBgG9M6LrqzQEngs0Xt6A5QDg5LS-UzN2A"
FirebaseData FirebaseData; //Khai báo object fbdo để lưu trữ dữ liệu Firebase
FirebaseAuth auth; //Khai báo object auth để xác thực với Firebase
FirebaseConfig config;

TaskHandle_t core1TaskHandle = NULL;
TaskHandle_t core0TaskHandle = NULL;

unsigned long lastTimePeopleZero = 0;

unsigned long senDataPrevMillis = 0;
bool signupOK = false;

void setup() {
  Serial.begin(115200);
  dht.begin();
  Wire.begin();
  lightMeter.begin();
  pinMode(trig1, OUTPUT);
  pinMode(echo1, INPUT);
  pinMode(trig2, OUTPUT);
  pinMode(echo2, INPUT);



  songuoitrongphong = 0;
  delay(500);
  dokhoangcach1();
  kccb1bandau = khoangcach1;
  dokhoangcach2();
  kccb2bandau = khoangcach2;
  Serial.print(kccb1bandau); Serial.print("   ");
  Serial.println(kccb2bandau);



  WiFi.begin(WIFI_SSID,WIFI_PASSWORD);
  Serial.print("connecting wifi.....");
  while (WiFi.status() != WL_CONNECTED)
  {
    Serial.print(".");
    delay(300);
  }
  Serial.println();
  Serial.print("connected with IP: ");
  Serial.println(WiFi.localIP());
  Serial.println();

  config.database_url = DATABASE_URL;
  config.api_key = API_KEY;
  if(Firebase.signUp(&config, &auth,"","")){
    Serial.println("signUp OK");
    signupOK = true;
  }else{
    Serial.printf("%s\n", config.signer.signupError.message.c_str());
  }

  Firebase.begin(&config, &auth); 
  Firebase.reconnectWiFi(true);

  Serial.printf("Firebase Client v%s\n\n", FIREBASE_CLIENT_VERSION); //Khởi tạo kết nối tới Firebase
  config.signer.test_mode = true;


}

void loop() {
   prevKhoangcach1 = khoangcach1;
    prevKhoangcach2 = khoangcach2;
    khoangcach1 = 0;
    khoangcach2 = 0;
    dokhoangcach1();
    dokhoangcach2();

    if (prevKhoangcach1 < 16 && khoangcach1 < 16 && hangdoi.charAt(0) != '1') {
      hangdoi += "1";
    } else if (prevKhoangcach2 < 16 && khoangcach2 < 16 && hangdoi.charAt(0) != '2') {
      hangdoi += "2";
    }

    if (hangdoi.equals("12")) {
      songuoitrongphong++;
      Serial.print("Hang doi: ");
      Serial.println(hangdoi);
      hangdoi = "";
      delay(1000);
    } else if (hangdoi.equals("21") && songuoitrongphong > 0) {
      songuoitrongphong--;
      Serial.print("Hang doi: ");
      Serial.println(hangdoi);
      hangdoi = "";
      delay(1000);
    }

    if (hangdoi.length() > 2 || hangdoi.equals("11") || hangdoi.equals("22") || timeoutcounter > 200) {
      hangdoi = "";
    }

    if (hangdoi.length() == 1) {
      timeoutcounter++;
    } else {
      timeoutcounter = 0;
    }

    Serial.print("Hang doi: ");
    Serial.print(hangdoi);
    Serial.print(" CB1: ");
    Serial.print(khoangcach1);
    Serial.print(" CB2: ");
    Serial.println(khoangcach2);
    Serial.println(songuoitrongphong);
    
    Firebase.setInt(FirebaseData, "/People", songuoitrongphong);

    float t = dht.readTemperature(); //Đọc nhiệt độ từ DHT11
  float h = dht.readHumidity(); //Đọc độ ẩm từ DHT11
  float lux = lightMeter.readLightLevel(); //đọc giá trị ánh sáng từ BH1750



  Firebase.setInt(FirebaseData, "/Sensor/Temperature", t);
  delay(500);
  Firebase.setInt(FirebaseData, "/Sensor/Humidity", h);
  delay(500);
  Firebase.setInt(FirebaseData, "/Sensor/Bright", lux);
  delay(400);
  
}




void dokhoangcach1() {
  digitalWrite(trig1, LOW);
  delayMicroseconds(2);
  digitalWrite(trig1, HIGH);
  delayMicroseconds(10);
  digitalWrite(trig1, LOW);
  thoigian1 = pulseIn(echo1, HIGH);
  khoangcach1 = thoigian1 / 2 / 29.412;
}

void dokhoangcach2() {
  digitalWrite(trig2, LOW);
  delayMicroseconds(2);
  digitalWrite(trig2, HIGH);
  delayMicroseconds(10);
  digitalWrite(trig2, LOW);
  thoigian2 = pulseIn(echo2, HIGH);
  khoangcach2 = thoigian2 / 2 / 29.412;
}





