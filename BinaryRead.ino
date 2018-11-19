#include "SdFat.h"
#include <SPI.h>

SdFat sd;
SdFile ThisFile;

#define SD_CS 4
#define SensorPin 15
#define ButtonPin 16
#define gLED 8
#define rLED 13

#define n 25
int readings[n];
byte readIndex = 0;
int total;
int SMA;

#define DebounceDelay 100  //milliseconds required for constant reading
bool Switch = false;
bool ButtonState = false;
byte ButtonReading;
uint32_t PrevBounceMillis = 0;

char DataFile[] = "file06.txt";

struct DataStruct {
  int Sensor1;
  uint32_t Timing;
};
struct DataStruct InData;

void setup() {
  Serial.begin(115200);
  while (!Serial) {
    SysCall::yield();
  }
  Serial.print("Press button to read from ");
  Serial.print(DataFile);
  Serial.println(" to serial monitor.");
  Serial.println("Or press '0' to delete file00.txt");
  Serial.println("(you must stop reading to delete the file)");
  pinMode(ButtonPin, INPUT);
  pinMode(SensorPin, INPUT);
  pinMode(rLED, OUTPUT);
  digitalWrite(rLED, LOW);
  pinMode(gLED, OUTPUT);

  sd.begin(SD_CS);
  SDCARD_SPI.beginTransaction(SPISettings(48000000, MSBFIRST, SPI_MODE0));
  digitalWrite(SD_CS, HIGH);
  pinMode(SD_CS, OUTPUT);
}

void Debounce() {
  ButtonReading = digitalRead(ButtonPin);
  if (ButtonState == false) {
    if (ButtonReading == ButtonState) {
      PrevBounceMillis = millis();
    }
    if (millis() - PrevBounceMillis > DebounceDelay) {
      ButtonState = true;
    }
  }
  if (ButtonState == true) {
    if (ButtonReading == ButtonState) {
      PrevBounceMillis = millis();
    }
    if (millis() - PrevBounceMillis > DebounceDelay) {
      ButtonState = false;
      rOpen_or_Close();
    }
  }
}

void rOpen_or_Close () {
  if (Switch == false) {
    digitalWrite(gLED, HIGH);
    ThisFile.open(DataFile, O_READ);
    if (!sd.open(DataFile)) {
      ThisFile.close();
      Serial.println("Enter the file you would like to open first (Ex. file00.txt)");
      return;
    }
    Switch = true;
    delay(500);
    digitalWrite(gLED, LOW);
    return;
  }
  else {
    digitalWrite(rLED, HIGH);
    ThisFile.close();
    Switch = false;
    delay(500);
    digitalWrite(rLED, LOW);
    return;
  }
}

void loop() {
  Debounce();

  if (Switch == true) {
    if (ThisFile.available()) {
      ThisFile.read((byte *)&InData, sizeof(InData));
      analogWrite(gLED, map(InData.Sensor1, 0 , 1023, 0, 255));
      Serial.print(InData.Sensor1);
      Serial.print(", ");
      Serial.println(InData.Timing);
      return;
    }
    analogWrite(gLED, 0);
    analogWrite(rLED, 0);
    rOpen_or_Close();
  }
  if (Switch == false && Serial.read() == '0') {
    ThisFile.remove();
    Serial.println("file00.txt removed from SD");
  }
}

void Error() {
  digitalWrite(rLED, HIGH);
  delayMicroseconds(100000);
  digitalWrite(rLED, LOW);
  delayMicroseconds(100000);
  digitalWrite(rLED, HIGH);
  delayMicroseconds(100000);
  digitalWrite(rLED, LOW);
}
