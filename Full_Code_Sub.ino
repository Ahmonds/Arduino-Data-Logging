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

#define n  40
int readings[n];
byte Pos = 0;
float total = 0;
float SMA = 0;

#define CutOFF   2153    //equal to 3.5V on BatteryPin (3V3 ref, 12bit res)
bool Switch = false;
#define DelayTime  1000  //delay in micros
uint32_t PastMicros = 0;

#define dbDelay 100
bool ButtonState = false;
byte ButtonReading = LOW;
uint32_t PrevBounceMillis = 0;

void setup() {
  pinMode(14 ,OUTPUT);
  pinMode(17 ,OUTPUT);
  pinMode(18 ,OUTPUT);
  pinMode(19 ,OUTPUT);
  pinMode(12 ,OUTPUT);
  pinMode(10 ,OUTPUT);
  pinMode(6 ,OUTPUT);
  pinMode(5 ,OUTPUT);
  pinMode(21 ,OUTPUT);
  pinMode(20 ,OUTPUT);

  pinMode(ButtonPin, INPUT);
  pinMode(SensorPin, INPUT);
  pinMode(BatteryPin, INPUT);
  pinMode(TestPin, OUTPUT);
  pinMode(gLED, OUTPUT);
  pinMode(rLED, OUTPUT);
  digitalWrite(rLED, LOW);
  analogReadResolution(12);
  analogReference(AR_DEFAULT);
  analogRead(SensorPin);

  sd.begin(SD_CS);
  SDCARD_SPI.beginTransaction(SPISettings(48000000, MSBFIRST, SPI_MODE0));
  pinMode(SD_CS, OUTPUT);
  digitalWrite(SD_CS, HIGH);

  for (byte InitReading = 0; InitReading < n; InitReading++) {
    readings[InitReading] = 0;
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
  digitalWrite(rLED, HIGH);
  delayMicroseconds(200000);
  digitalWrite(rLED, LOW);
  delayMicroseconds(200000);
  digitalWrite(rLED, HIGH);
  delayMicroseconds(200000);
  digitalWrite(rLED, LOW);
}

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
    digitalWrite(gLED, HIGH);
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
      return;
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
      ThisFile << analogRead(SensorPin) << ',' << micros() << '\n';
      digitalWrite(TestPin, LOW);
    }
  }
}
