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

#define n 25
int readings[n];
byte readIndex = 0;
int total;
int SMA;

#define CutOFF   543    //equal to 3.5V on BatteryPin (3.3 ref)
bool Switch = false;
#define DelayTime  1000  //delay in micros
uint32_t PastMicros = 0;

#define DebounceDelay 100
bool ButtonState = false;
byte ButtonReading;
uint32_t PrevBounceMillis = 0;

void setup() {
  pinMode(ButtonPin, INPUT);
  pinMode(SensorPin, INPUT);
  pinMode(BatteryPin, INPUT);
  pinMode(TestPin, OUTPUT);
  pinMode(gLED, OUTPUT);
  pinMode(rLED, OUTPUT);
  digitalWrite(rLED, LOW);

  sd.begin(SD_CS);
  SDCARD_SPI.beginTransaction(SPISettings(48000000, MSBFIRST, SPI_MODE0));
  digitalWrite(SD_CS, HIGH);
  pinMode(SD_CS, OUTPUT);

  for (byte InitReading = 0; InitReading < n; InitReading++) {
    readings[InitReading] = 0;
  }
}

void Debounce() {
  ButtonReading = digitalRead(ButtonPin);
  if (!ButtonState) {
    if (ButtonReading == ButtonState) {
      PrevBounceMillis = millis();
    }
    if (millis() - PrevBounceMillis > DebounceDelay) {
      ButtonState = true;
    }
  }
  if (ButtonState) {
    if (ButtonReading == ButtonState) {
      PrevBounceMillis = millis();
    }
    if (millis() - PrevBounceMillis > DebounceDelay) {
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
      if (! sd.exists(DataFile)) {
        break;
      }
    }
    ThisFile.open(DataFile, O_CREAT | O_APPEND | O_WRITE);
    if (!ThisFile.is_open()) {
      ThisFile.close();
      Error();
      return;
    }
    Switch = true;
    delayMicroseconds(500000);
    digitalWrite(gLED, LOW);
    return;
  }
  else {
    digitalWrite(rLED, HIGH);
    ThisFile.close();
    Switch = false;
    delayMicroseconds(500000);
    digitalWrite(rLED, LOW);
    return;
  }
}

void BatteryMonitor() {
  total -= readings[readIndex];
  readings[readIndex] = analogRead(BatteryPin);
  total += readings[readIndex];
  readIndex += 1;
  if (readIndex >= n) {
    readIndex = 0;
  }
  SMA = total / n;
}

void loop() {
  BatteryMonitor();
  if (SMA < CutOFF) {
    if (Switch) {
      digitalWrite(rLED, HIGH);
      Switch = false;
      ThisFile.close();
      delayMicroseconds(500000);
      digitalWrite(rLED, LOW);
    }
    else {
      return;
    }
  }
  Debounce();

  if (Switch) {
    if (micros() - PastMicros >= DelayTime) {
      PastMicros = micros();
      ThisFile << analogRead(SensorPin) << ',' << micros() << endl;
    }
  }
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
