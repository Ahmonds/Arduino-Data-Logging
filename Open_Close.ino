#include "SdFat.h"
SdFat sd;
ofstream ThisFile;

#define gLED 8
#define rLED 13

bool Switch = true;

void setup () {
  pinMode(gLED, OUTPUT);
  pinMode(rLED, OUTPUT);
  digitalWrite(rLED, LOW);
  digitalWrite(gLED, LOW);
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
      //Error();
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

void loop () {}
