#include <Adafruit_Sensor.h>
#include <DHT.h>
#include <DHT_U.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include "ESP8266.h"
#include <SoftwareSerial.h>

#define DHTTYPE DHT22
#define SSID        "GOMAD"
#define PASSWORD    "a2345678"
#define HOST_NAME   "toojs.asuscomm.com"
#define HOST_PORT   (8099)

const int gasPin = A0;
int blue = 2;
int green = 3;
int red = 4;
int buzzer = 12;
int DHTPIN = 13;

SoftwareSerial WIFI(8, 9);
ESP8266 wifi = ESP8266(WIFI);
SoftwareSerial Dust(6, 7);
DHT dht(DHTPIN, DHTTYPE);
LiquidCrystal_I2C lcd(0x27, 16, 2);

float hum;
float temp;

int PM2_5;

int stack = 280;

byte bytCount = 0;

unsigned char chrData[30];
unsigned char chrRecv;

void setup(){
  Serial.begin(9600);
  dht.begin();
  Dust.begin(9600);
  pinMode(buzzer, OUTPUT);
  pinMode(blue, OUTPUT);
  pinMode(green, OUTPUT);
  pinMode(red, OUTPUT);
  lcd.init();
  lcd.backlight();
  lcd.clear();

  delay(1000);
}
 
void loop(){
  hum = dht.readHumidity();
  temp = dht.readTemperature();
  int gas = 0;

  for(int i = 0; i < 32; i++) {
    chrRecv = Dust.read();
    if(chrRecv == 77) {
      bytCount = 1;
      break;
    }
  }

  if(bytCount == 1) {
    bytCount == 0;
    for(int i = 0; i < 30; i++) {
      chrData[i] = Dust.read();
    }
  }

  PM2_5 = GetPM_Data(chrData, 10, 11);
  Serial.println(PM2_5);

  lcd.setCursor(0, 0);
  lcd.print("TEMP : ");
  lcd.print(temp, 1);
  lcd.print(" C deg");
  lcd.setCursor(0,1);
  lcd.print("HUMIDITY : ");
  lcd.print(hum);
  lcd.print(" %");
  lcd.setCursor(0, 2);
  lcd.print("DUST : ");
  lcd.print(PM2_5);
  lcd.print("[ug/m3]");

  if(PM2_5 < 40){
    digitalWrite(blue, LOW);
    digitalWrite(green, HIGH);
    digitalWrite(red, LOW);
  }
  
  else if(PM2_5 < 80){
    digitalWrite(blue, LOW);
    digitalWrite(green, HIGH);
    digitalWrite(red, HIGH);
    stack += 30;
  }

  else {
    digitalWrite(blue, LOW);
    digitalWrite(green, LOW);
    digitalWrite(red, HIGH);
    stack += 80;
  }
  
  if(analogRead(gasPin) > 500) {
     Serial.println("검출");
     digitalWrite(buzzer, HIGH);
     gas = 1;
     stack += 100;
  }
  else {
    digitalWrite(buzzer, LOW);
    gas = 0;
  }
  
  if(stack > 300){
    Dust.end();
    WIFI.begin(9600);
    
    if (wifi.setOprToStationSoftAP()) {
        Serial.print("to station + softap ok\r\n");
    } else {
        Serial.print("to station + softap err\r\n");
    }
 
    if (wifi.joinAP(SSID, PASSWORD)) {
        Serial.print("Join AP success\r\n");
        Serial.print("IP:");
        Serial.println(wifi.getLocalIP().c_str());       
    } else {
        Serial.print("Join AP failure\r\n");
    }
    
    if (wifi.disableMUX()) {
        Serial.print("single ok\r\n");
    } else {
        Serial.print("single err\r\n");
    }
    
    if (wifi.createTCP(HOST_NAME, HOST_PORT)) {
         Serial.print("create tcp ok\r\n");
     } else {
         Serial.print("create tcp err\r\n");
     }
      
     String sens = "temperature=";
     sens += temp;
     sens += "&humidity=";
     sens += hum;
     sens += "&gas=";
     sens += gas;
     sens += "&fine_dust=";
     sens += PM2_5;
     sens += "&machine_type=1&machine_num=3";
     
     String cmd = "POST /data/node/uploadData/hello@naver.com HTTP/1.1\r\nHost: toojs.asuscomm.com:8099\r\nContent-Type: application/x-www-form-urlencoded\r\nContent-Length: ";
     cmd += String(sens.length());
     cmd += "\r\n\r\n";
     cmd += sens;
     wifi.send(cmd.c_str(), cmd.length());

     stack = 0;
     WIFI.end();
     Dust.begin(9600);
  }
  stack++;
  delay(1000);
}

unsigned int GetPM_Data(unsigned char chrSrc[], byte bytHigh, byte bytLow)
{
  return (chrSrc[bytHigh] << 8) + chrSrc[bytLow];
}
