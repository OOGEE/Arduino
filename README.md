아두이노를 이용한 고정식 미세먼지 및 유해가스 검출 장치
=
>I2C LCD, 온습도 측정 센서(DHT22), 유해가스 검출 센서(MQ-2), BUZZER, 미세먼지 센서(PMS7003), RGB LED, 와이파이쉴드(ESP8266) 사용

>> 여기서 유해가스는 가연성 기체라 생각을 하였음(일반인의 신분으로써 유해 가스를 구할 수 있는 방법이 없기 때문)



## I2C LCD 세팅

>사용 전에 스케치 -> 라이브러리 포함하기 -> 라이브러리 관리에서 LiquidCrystal I2C 검색 후 Frank de Brabander가 만든 라이브러리 설치

>파일 -> 예제 -> LiquidCrystal_I2C -> CustomChars 예제 실행 후 나오는 값이 주소 lcd의 주소 값

~~~c
#include <LiquidCrystal_I2C.h> // I2C LCD를 사용하기 위해 헤더선언

LiquidCrystal_I2C lcd(0x27, 16, 2) // lcd를 나중에 코드들에 사용하기 위해 정의, 0x27 값은 지정된 방식을 사용하여 주소 값 확인

---- setup ----
lcd.init();     // lcd를 초기화
lcd.backlight(); // lcd의 백라이트를 켜줌
lcd.clear();    // lcd에 내용을 지운 다음에 커서를 (0,0)으로 옮김

~~~



## 온습도 측정 센서(DHT22) 세팅 및 값 LCD에 표기
>사용 전에 스케치 -> 라이브러리 포함하기 -> 라이브러리 관리에서 DHT sensor library 검색 후 Adafruit가 만든 라이브러리 설치
~~~c
#include <DHT.h>    // DHT22 사용에 앞서 필요한 명령어들을 불러오기 위한 헤더 선언
#include <DHT_U.h>  // DHT.h와 마찬가지

#define DHTTYPE DHT22   // 온습도 센서중 기기의 모델 번호가 DHT22이므로 선언
int DHTPIN = 13    // DHT의 데이터를 받아올 아두이노의 디지털 포트 번호를 13번으로 지정

float hum, temp;    // 온습도값을 받아오기 위한 변수 선언, 실수 값으로 받아오기 때문에 float 선언

DHT dht(DHTPIN, DHTTYPE)    // dht를 나중에 코드들에 사용하기 위해 정의

---- setup ----
dht.begin() // dht22센서를 초기화 하면서 가동시킴

---- loop ----
hum = dht.readMumidity();   // 라이브러리를 사용하여 습도 값 받아옴
temp = dht.readTemperature();   // 라이브러리를 사용하여 온도 값 받아옴

lcd.setCursor(0, 0);        // lcd의 커서 위치 (0,0)으로 옮김
lcd.print("TEMP : ");       // 첫줄에 TEMP : 
lcd.print(temp, 1);         // 온도값 소숫점 첫번째 까지
lcd.print(" C deg");        // C deg 표시
                            // TEMP : ~~.~ C deg로 표현 됨
lcd.setCursor(0,1);         // lcd의 커서 위치 (0,1)으로 옮김, 두번째 줄 첫 칸
lcd.print("HUMIDITY : ");   // 두번째 줄에 HUMIDITY :
lcd.print(hum);             // 습도 표기
lcd.print(" %");            // % 표시
                            // HUMIDITY : ~~.~~ % 로 표시됨

~~~



## 유해가스 검출 센서(MQ-2) 세팅 및 BUZZER와 연동
> 안에 있는 금속막에 공기중의 성분이 달라붙는데 그 성분에 따라 저항값이 달라짐.

> 이를 전압값으로 변환하여 아두이노에 아날로그 값으로 전송해줌

> 이 전압 값은 뒤에있는 가변 저항으로 조절을 해줘야 함(조절하지 않으면 값이 굉장히 민감하게 나올 뿐더러 정확한 측정이 힘들어짐)

~~~c
int gasPin = A0;    // MQ-2의 데이터를 받아올 아두이노의 아날로그 포트 번호를 0번으로  지정
int buzzer = 12;    // 유해가스가 검출되었을 시 BUZZER를 울리기 위해 전압을 가해줘야 하는데 그 위치를 12번 디지털 포트로 지정

---- setup ----
pinMode(buzzer, OUTPUT);    // 버저의 핀을 출력모드로 설정

---- loop -----
if(analogRead(gasPin) > 500) {  // analogRead(gasPin)을 사용하면 MQ-2에서 측정한 전압 값이 나옴
                                // 라이터로 가스를 주입했을때와 평상시의 중간값이 약 4~500정도 된다는걸 알아서 500 이상으로 측정될 시 검출이라고 판단
     Serial.println("검출");    // Serial에 확인하기 위한 print문
     digitalWrite(buzzer, HIGH);    // 버저에 연결되어있는 핀에 전압을 가해 버저가 울리게 만듬
     gas = 1;                   // 이 값은 데이터 전송과 관련이 되어있음
                                // 간단히 설명하면 가스가 검출되었음을 나타냄
     stack += 100;              // 이 값은 데이터 전송 주기와 관련되어 있는 값임
                                // 간단히 설명하면 가스 검출 시 주기를 빠르게 해주는 역할을 함
}
else {
    digitalWrite(buzzer, LOW);  // 검출되지 않았을 시 버저가 울리지 않아야 하기 때문에 전압을 낮춰줌
    gas = 0;                    // 위에서 설명했듯이 데이터 전송과 관련이 있으며 가스가 검출되지 않았음을 의미
}
~~~
> 가스값을 전압 값으로 본다고 하더라고 어떠한 상태인지 인지하기 힘든 값이고 버저로 표현되기 때문에 LCD에 따로 표기하지 않았음



## 미세먼지 검출 센서(PMS7003) 세팅 및 LED와 연동, LCD에 표기

~~~c
#include <SoftwareSerial.h> // PMS7003은 내부에서 측정한 값을 자체적으로 처리하고 시리얼 통신을 하기 때문에 그를 위한 헤더 선언

SoftwareSerial Dust(6, 7)   // Dust를 나중에 코드들에 사용하기 위해 정의, 6, 7번 디지털 포트를 각각 RX, TX에 맞춰 Serial 통신

int blue = 2;   // 전원이 들어오면 3색 LED의 파란색이 점듬됨
int green = 3;  // 전원이 들어오면 3색 LED의 초록색이 점등됨
int red = 4;    // 전원이 들어오면 3색 LED의 빨간색이 점등됨
                // 각각의 색들을 켜거나 꺼서 색들 조합 가능

int PM2_5;      // PM2.5 값을 받아오기 위해 선언
                // PM1.0 값과 PM10 값을 모두 받아올 수 있지만 대략적인 수치 표현과 LCD의 공간적인 한계, 서버 전송을 위해 한 가지 값만 사용

---- setup ----
Dust.begin(9600);   // Serial 통신을 하기 위해 9600의 정해진 Baud Rate을 기입 및 센서 가동
                    // 여기서 Baud Rate는 초당 전송되는 신호의 수으로써 정해진 값을 준수해야함, 높을수록 송수신속도는 빠르지만 데이터 수신에 문제가 발생할 수도 있음

---- loop ----
// PMS7003의 데이터를 정상적으로 받아오게 하기 위한 반복, 조건문 결합
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

PM2_5 = GetPM_Data(chrData, 10, 11);    // PM2.5 값을 받아오기 위해 chrData에 저장되어 있는 값중 일부분을 추출

lcd.setCursor(0, 2);    // lcd의 커서 위치 (0,2)로 옮김, 3번째 줄 첫 칸
lcd.print("DUST : ");   // 세번째줄에 DUST :
lcd.print(PM2_5);       // 미세먼지 값 표기
lcd.print("[ug/m3]");   // [ug/m3] 표시
                        // DUST : ~~[ug/m3]로 표시됨

if(PM2_5 < 30){         // 미세먼지 값의 좋음 범위가 0~30으로 지정되있어 그 수치에 맞춰 LED에 초록색 점등
    digitalWrite(blue, LOW);
    digitalWrite(green, HIGH);
    digitalWrite(red, LOW);
}
  
else if(PM2_5 < 80){    // 미세먼지 값의 보통 범위가 30~80으로 지정되어 있어 그 수치에 맞게 LED에 주황불 점등
    digitalWrite(blue, LOW);
    digitalWrite(green, HIGH);
    digitalWrite(red, HIGH);
    stack += 30;        // 미세먼지 수치가 높을 경우 데이터를 더 자주 수집할 수 있게 주기 조절
}

else {                  // 미세먼지 값의 나쁨 범위가 80~으로 지정되어 있으 그 수치에 맞게 LED에 빨간불 점등
    digitalWrite(blue, LOW);
    digitalWrite(green, LOW);
    digitalWrite(red, HIGH);
    stack += 80;        // 미세먼지 수치가 높을 경우 데이터를 더 자주 수집할 수 있게 주기 조절
}
~~~

## 와이파이 쉴드(ESP8266)를 사용한 데이터 송신 및 주기 조절

> 와이파이 쉴드는 시리얼 통신 방식으로 데이터를 송수신 함

> 실질적으로 온습도값, 미세먼지 값은 그리 자주 바뀌지 않기 때문에 일정 주기를 두고 전송

> 단, 유해가스가 검출 되거나 미세먼지 수치가 높을 시엔 즉시 대처를 해야하기 때문에 데이터 업로드 속도를 조절 함

> 동시에 두가지 이상의 시리얼 통신을 할 수 없기 때문에 그에 관한 기법이 쓰임

~~~
#include "ESP8266.h" // ESP8266을 사용하기 위한 라이브러리

#define SSID        "WIFI-NAME" // WIFI의 이름을 SSID에 정의해놓음
#define PASSWORD    "PASSWORD" // WIFI의 비밀번호를 PASSWORD에 정의해놓음
#define HOST_NAME   "toojs.asuscomm.com" // 서버의 주소를 정의해놓음
#define HOST_PORT   (8099) // 포트번호를 지정해놓음

SoftwareSerial WIFI(8, 9);      // RX와 TX를 각각 8, 9번 핀에 연결
ESP8266 wifi = ESP8266(WIFI);   // ESP8266을 사용하기 위해 정의

int stack = 280;        // 주기를 위한 스택값을 정의
                        // 초반에 잘 송신을 하기 위해 스택 값을 미리 높여놓음
                        
---- loop ----
if(stack > 300){    // 스택이 300 넘을 시 전송을 수행
    Dust.end();     // 동시에 2개의 시리얼 통신을 사용 할 수 없기 때문에
                    // PMS7003의 시리얼 통신을 중단함
    WIFI.begin(9600);   // ESP8266의 시리얼 통신 개시
    
    //통신을 하기 위한 와이파이 쉴드 세팅
    if (wifi.setOprToStationSoftAP()) {
        Serial.print("to station + softap ok\r\n");
    } else {
        Serial.print("to station + softap err\r\n");
    }
 
    if (wifi.joinAP(SSID, PASSWORD)) {  // WIFI에 연결하는 문장
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
     
    // 통신 프로토콜을 맞추기 위해 String에 글 저장
    
    // sens에 측정한 데이터값들을 결합
     String sens = "temperature=";
     sens += temp;
     sens += "&humidity=";
     sens += hum;
     sens += "&gas=";
     sens += gas;
     sens += "&fine_dust=";
     sens += PM2_5;
     sens += "&machine_type=1&machine_num=3";
     // temperature에 온도, humidity에 습도, gas에 유해가스 검출 여부
     // fine_dust에 미세먼지 측정치, machine_type을 1로하여 고정형 표시
     // 위치에 따라 구분을 하기 위한 machine_num 정의
     
     // POST를 하기 위한 프로토콜 문 제작 및 sens 뒤에 이어 붙이기
     String cmd = "POST /data/node/uploadData/hello@naver.com HTTP/1.1\r\nHost: toojs.asuscomm.com:8099\r\nContent-Type: application/x-www-form-urlencoded\r\nContent-Length: ";
     cmd += String(sens.length());
     cmd += "\r\n\r\n";
     cmd += sens;
     
     // 서버에 String 전송
     wifi.send(cmd.c_str(), cmd.length());

    // stack 초기화
     stack = 0;
     
     // ESP8266의 시리얼 통신을 중단하고
     // PMS7003의 시리얼 통신 재개
     WIFI.end();
     Dust.begin(9600);
  }
stack++;  // 문장이 종료되기 전에 stack 증가
delay(1000);
~~~