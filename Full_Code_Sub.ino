#include "SdFat.h"
#include <SPI.h>
SdFat sd;
ofstream ThisFile;

#define ButtonPin 16
#define SensorPin 15
#define BatteryPin 9
#define gLED 8
#define rLED 13
#define SD_CS 4
#define MISO 22
#define MOSI 23
#define SCK 24
#define TestPin 11
#define CardDetect 7

#define n  50
int readings[n];
byte Pos = 0;
float total = 0;
float SMA = 0;

#define A_RES 12

#if A_RES == 10
#define CutOFF 543    //3V5 on BatteryPin w/ 3V3 ref, 10bit res
#elif A_RES == 12
#define CutOFF 2153   //3V5 on BatteryPin w/ 3V3 ref, 12bit res
#endif

bool Switch = false;
#define DelayTime  1000  //delay in micros
uint32_t PastMicros = 0;

#define dbDelay 150
bool ButtonState = false;
byte ButtonReading = LOW;
uint32_t PrevBounceMillis = 0;

void Debounce() {

  if (!ButtonState) {
    ButtonReading = digitalRead(ButtonPin);
    if (ButtonReading == ButtonState) PrevBounceMillis = millis();
    if (millis() - PrevBounceMillis > dbDelay) ButtonState = true;
  }

  if (ButtonState) {
    ButtonReading = digitalRead(ButtonPin);
    if (ButtonReading == ButtonState) PrevBounceMillis = millis();
    if (millis() - PrevBounceMillis > dbDelay) {
      ButtonState = false;
      Open_or_Close();
    }
  }
}

void Open_or_Close () {

  if (!Switch) {
    if (!digitalRead(CardDetect)) {
      Error();
      return;
    }

    digitalWrite(gLED, HIGH);
    sd.begin(SD_CS);
    SDCARD_SPI.beginTransaction(SPISettings(48000000, MSBFIRST, SPI_MODE0));
    pinMode(SD_CS, OUTPUT);
    digitalWrite(SD_CS, HIGH);

    char DataFile[10];
    strcpy(DataFile, "file00.txt");
    for (byte i = 0; i < 100; i++) {
      DataFile[4] = '0' + i / 10;
      DataFile[5] = '0' + i % 10;
      if (!sd.exists(DataFile)) break;
    }
    ThisFile.open(DataFile, O_WRITE | O_APPEND | O_CREAT);
    if (!ThisFile.is_open()) {
      ThisFile.close();
      Error();
    }
    Switch = true;
    delayMicroseconds(300000);
    digitalWrite(gLED, LOW);
    return;
  }

  else {
    digitalWrite(rLED, HIGH);
    ThisFile.close();
    Switch = false;
    delayMicroseconds(300000);
    digitalWrite(rLED, LOW);
    return;
  }
}

void BatteryMonitor() {
  total -= readings[Pos];
  readings[Pos] = analogRead(BatteryPin);
  total += readings[Pos];
  (Pos < n ? Pos++ : Pos = 0);  //(Condition ? true : false)
  SMA = total / n;
}

void Error() {
  digitalWrite(7, LOW);
  digitalWrite(rLED, HIGH);
  delayMicroseconds(200000);
  digitalWrite(rLED, LOW);
  delayMicroseconds(200000);
  digitalWrite(rLED, HIGH);
  delayMicroseconds(200000);
  digitalWrite(rLED, LOW);
}

void setup() {
  pinMode(14 , OUTPUT);
  pinMode(17 , OUTPUT);
  pinMode(18 , OUTPUT);
  pinMode(19 , OUTPUT);
  pinMode(12 , OUTPUT);
  pinMode(10 , OUTPUT);
  pinMode(6 , OUTPUT);
  pinMode(5 , OUTPUT);
  pinMode(21 , OUTPUT);
  pinMode(20 , OUTPUT);

  pinMode(CardDetect, INPUT_PULLUP);
  pinMode(ButtonPin, INPUT);
  pinMode(SensorPin, INPUT);
  pinMode(BatteryPin, INPUT);
  pinMode(TestPin, OUTPUT);
  pinMode(gLED, OUTPUT);
  pinMode(rLED, OUTPUT);
  digitalWrite(rLED, LOW);
  analogReadResolution(A_RES);
  analogReference(AR_DEFAULT);  //Default == operating voltage (3V3)
  analogRead(SensorPin);

  for (byte InitReading = 0; InitReading < n; InitReading++) {
    readings[InitReading] = 0;
  }
}

void loop() {
  BatteryMonitor();

  if (SMA < CutOFF) {
    if (Switch) {
      Open_or_Close();
      Error();
      return;
    }
    else return;
  }

  Debounce();

  if (Switch) {
    if (micros() - PastMicros >= DelayTime) {
      PastMicros = micros();
      digitalWrite(TestPin, HIGH);
      ThisFile  << micros() << ',' << analogRead(SensorPin) << '\n';
      digitalWrite(TestPin, LOW);
    }
  }
}
