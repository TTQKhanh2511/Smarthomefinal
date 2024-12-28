#include "ACS712.h"
#include <Wire.h>

#include <Firebase_ESP_Client.h>
#include <addons/RTDBHelper.h>
#include <LiquidCrystal_I2C.h>

#define WIFI_SSID "NEYU"
#define WIFI_PASSWORD "0987654321"

//------------------------------------------------------define BTN
#define auto_btn 15
#define man_btn 2
#define bulb_btn 4
#define fan_btn 5
#define fan 33
#define bulb 13

//----------------------------------------------------define FireBase
#define DATABASE_URL "project1-ee980-default-rtdb.firebaseio.com"
#define API_KEY "AIzaSyBgG9M6LrqzQEngs0Xt6A5QDg5LS-UzN2A"

FirebaseData fbdo;
FirebaseData fbdo1; //3 sensors
FirebaseData fbdo2;

FirebaseData FirebaseDataMode;

FirebaseData FirebaseDataDev1;
FirebaseData FirebaseDataDev2;

FirebaseAuth auth; //Khai báo object auth để xác thực với Firebase
FirebaseConfig config;
unsigned long senDataPrevMillis = 0;
bool signupOK = false;

//define LCD
int lcdColumns = 16;
int lcdRows = 2;
LiquidCrystal_I2C lcd(0x27, lcdColumns, lcdRows);

int i = 1; //change Display LCD
int fanState, bulbState;
void setup() {
  Serial.begin(115200);
  Wire.begin(21, 22);
  pinMode(auto_btn,INPUT_PULLUP);
  pinMode(man_btn,INPUT_PULLUP);
  pinMode(bulb_btn,INPUT_PULLUP);
  pinMode(fan_btn,INPUT_PULLUP);

  pinMode(fan, OUTPUT);
  pinMode(bulb, OUTPUT);
  
//.................................set up LCD..........................................
  lcd.init();
  // turn on LCD backlight                      
  lcd.backlight();


//---------------------------------Firebase----------------------------------------------------//
  Serial.printf("Firebase Client v%s\n\n", FIREBASE_CLIENT_VERSION); //Khởi tạo kết nối tới Firebase
  config.database_url = DATABASE_URL;
  config.signer.test_mode = true;
  Firebase.reconnectWiFi(true);
  Firebase.begin(&config, &auth);


//----------------------------------WIFI----------------------------------------------//
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
  float humid_value, temp_value, light_value;
  String mode, fanStatus, bulbStatus;


  if (Firebase.RTDB.getString(&FirebaseDataMode, "/Mode")) {
    mode = FirebaseDataMode.stringData();
    delay(300);
  }

  if (Firebase.RTDB.getString(&FirebaseDataDev1, "/Device/Bulb")) {
    bulbStatus = FirebaseDataDev1.stringData();
    delay(300);
  }

  if (Firebase.RTDB.getString(&FirebaseDataDev2, "/Device/Fan")) {
    fanStatus = FirebaseDataDev2.stringData();
    delay(300);
  }

  if (fanStatus =="OFF"){
    fanState = 0;
    delay(200);
  }else{
    fanState = 1;
    delay(200);
  }

  if (bulbStatus =="OFF"){
    bulbState = 0;
    delay(200);
  }else{
    bulbState = 1;
    delay(200);
  }

  if (Firebase.RTDB.getFloat(&fbdo, "/Sensor/Humidity")) {
    humid_value = fbdo.floatData();
    delay(300);
  }
  
  if (Firebase.RTDB.getFloat(&fbdo1, "/Sensor/Temperature")) {
    temp_value = fbdo1.floatData();
    delay(300);
  }

  if (Firebase.RTDB.getFloat(&fbdo2, "/Sensor/Bright")) {
    light_value = fbdo2.floatData();
    delay(300);
  }

  if (Firebase.RTDB.getString(&FirebaseDataMode, "/Mode")) {
    mode = FirebaseDataMode.stringData();
    delay(300);
  }

  Serial.println(temp_value);
  Serial.println(humid_value);
  Serial.println(light_value);


  //.....................display temperature................................
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Temp:");
  lcd.print(temp_value);
  lcd.print(" oC");
  //.....................display humidity....................................
  lcd.setCursor(0, 1);
  lcd.print("Hum:");
  lcd.print(humid_value);
  lcd.print(" %");
  delay(1500);
  
  //.....................display Light.....................................
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Light:");
  lcd.print(light_value);
  lcd.print(" lux");
  //.....................display Mode.....................................
  lcd.setCursor(0, 1);
  lcd.print("MODE:");
  lcd.print(mode);
  delay(1500);


  //.....................check BUTTON.....................................
  if (digitalRead(man_btn)==1){
    mode = "Manual";
    Serial.println("Manual"); } 
  if (digitalRead(auto_btn)==0){
    mode = "Auto";
    Serial.println("Auto"); }
  
  int check_fan = digitalRead(fan_btn); //điều khiển bằng tay quạt
  if (check_fan == 0){
    fanState = !fanState;
    Serial.println(fanState);
  }
  if (fanState ==0){
    fanStatus = "OFF";
    Serial.println(fanStatus);
    delay(200);
  }else{
    fanStatus = "ON";
    Serial.println(fanStatus);
    delay(200);
  }

  int check_bulb = digitalRead(bulb_btn); //điều khiển bằng tay đèn
  if (check_bulb == 0){
    bulbState = !bulbState;
    Serial.println(bulbState);
  }
  if (bulbState ==0){
    bulbStatus = "OFF";
    Serial.println(bulbStatus);
    delay(200);
  }else{
    bulbStatus = "ON";
    Serial.println(bulbStatus);
    delay(200);
  }

  //.................................cập nhập trạng thái mode và device lên firebase
  Firebase.RTDB.setString(&FirebaseDataMode, "/Mode",mode);
  Firebase.RTDB.setString(&FirebaseDataDev1, "/Device/Bulb",bulbStatus);
  Firebase.RTDB.setString(&FirebaseDataDev2, "/Device/Fan",fanStatus);
  delay(5000);
  //.............................lấy trạng thái mode và device từ firebase
  if (Firebase.RTDB.getString(&FirebaseDataMode, "/Mode")) {
    mode = FirebaseDataMode.stringData();
    delay(300);
  }

  if (Firebase.RTDB.getString(&FirebaseDataDev1, "/Device/Bulb")) {
    bulbStatus = FirebaseDataDev1.stringData();
    delay(300);
  }

  if (Firebase.RTDB.getString(&FirebaseDataDev2, "/Device/Fan")) {
    fanStatus = FirebaseDataDev2.stringData();
    delay(300);
  }
  //.....................display Mode.....................................
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Fan:");
  lcd.print(fanStatus);
  lcd.setCursor(8, 0);
  lcd.print("Bulb:");
  lcd.print(bulbStatus);

  lcd.setCursor(0, 1);
  lcd.print("MODE: ");
  lcd.print(mode);
  delay(1500);

  if (bulbStatus =="OFF"){
    digitalWrite(bulb,LOW);
    Serial.println("OFF bulb");
    delay(200);
  }
  if (bulbStatus =="ON"){
    digitalWrite(bulb,HIGH);
    Serial.println("ON bulb");
    delay(200);
  }

  if (fanStatus =="OFF"){
    digitalWrite(fan,LOW);
    Serial.println("OFF fan");
    delay(200);
  }
  if (fanStatus =="ON"){
    digitalWrite(fan,HIGH);
    Serial.println("ON fan");
    delay(200);
  }
}