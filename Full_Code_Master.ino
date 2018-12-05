/*          Completely Functioning Code with,
       Simple Moving Average to monitor battery
       Logging Cut-off when battery voltage falls below threshold
       Debounce for Button
       Creates new file##.txt each time logging starts
       Switch on/off for logging
       Delay interval between data points for consistancy
      **Faster SPI**
*/

#include "SdFat.h"        //Add SdFat library, SPI, and set Out Stream
#include <SPI.h>
SdFat sd;
ofstream ThisFile;

#define ButtonPin 16       //Set pin numbers
#define SensorPin 15
#define gLED 8
#define rLED 13
#define BatteryPin 9
#define MISO 22
#define MOSI 23
#define SCK 24
#define SD_CS 4

#define n 40               //n samples to average
int readings[n];           //create moving average buffer
byte Pos = 0;
float total = 0;
float SMA = 0;

#define CutOFF 2153         //3.5V on BatteryPin w/ 3.3V reference && 12bit resolution /
bool Switch = false;        //Set varaibles and constans for logging
#define DelayTime 1000      //Time between readings in microseconds (+ a few uS for run time)
uint32_t PastMicros = 0;

bool ButtonState = false;  //Set varaibles for debounce
byte ButtonReading = LOW;
uint32_t PrevBounceMillis = 0;
#define dbDelay 100          //Constant reading for this many milliseconds to debounce

//==================================================================================================================

void setup() {              //Setup
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
  pinMode(11, OUTPUT);

  pinMode(ButtonPin, INPUT);                                                  //Set pins as out/input
  pinMode(SensorPin, INPUT);
  pinMode(BatteryPin, INPUT);
  pinMode(gLED, OUTPUT);
  pinMode(rLED, OUTPUT);
  digitalWrite(rLED, LOW);
  analogReadResolution(12);   //12 bit ADC resolution
  analogReference(AR_DEFAULT);    //The default reference is the boards operating voltage (3V3)
  analogRead(SensorPin);

  sd.begin(SD_CS);                                                           //Initialize SD card
  SDCARD_SPI.beginTransaction(SPISettings(48000000, MSBFIRST, SPI_MODE0));   //Begin SPI manually
  pinMode(SD_CS, OUTPUT);                                                    //Begin SPI manually
  digitalWrite(SD_CS, HIGH);                                                 //Begin SPI manually

  for (byte InitReading = 0; InitReading < n; InitReading++) {               //Create initial buffer for SMA
    readings[InitReading] = 0;
  }
}

//==================================================================================================================

void Debounce() {           //Debounce action

  if (!ButtonState) {                            //If we are in the low state and the Battery is charged,
    ButtonReading = digitalRead(ButtonPin);      //hold button reading
    if (ButtonReading == ButtonState) {          //Check and wait for the button to go high,
      PrevBounceMillis = millis();               //If the button goes high, save that time...
    }
    if (millis() - PrevBounceMillis > dbDelay) { //Once the reading has consistantly been high for 100 mS,
      ButtonState = true;                        //set the state to true
    }
  }

  if (ButtonState) {                         //If we are currently in the high state (the button has been pressed),
    ButtonReading = digitalRead(ButtonPin);  //hold button reading
    if (ButtonReading == ButtonState) {      //Check and wait for the button to go low,
      PrevBounceMillis = millis();           //If the button goes low, save that time...
    }
    if (millis() - PrevBounceMillis > dbDelay) { //Once the reading has consistantly been low for 100 mS,
      ButtonState = false;                       //set the state back to false,
      Open_or_Close();                           //and either open or close a file...
    }
  }
}

//==================================================================================================================

void Open_or_Close () {     //Open or Close file action
  if (!Switch) {                                              //If the Switch was off (data was not logging)
    digitalWrite(gLED, HIGH);
    char DataFile[10];
    strcpy(DataFile, "File00.TXT");                           //Create a new File##
    for (byte i = 0; i < 100; i++) {
      DataFile[4] = '0' + i / 10;
      DataFile[5] = '0' + i % 10;
      if (! sd.exists(DataFile)) break;
    }
    ThisFile.open(DataFile, O_WRITE | O_APPEND | O_CREAT);    //Create and open that File## for writing
    if (!ThisFile.is_open()) {
      ThisFile.close();
      Switch = false;
      Error();
    }
    Switch = true;                                            //Turn Switch on
    delayMicroseconds(300000);
    digitalWrite(gLED, LOW);                                  //Blink Green LED once
    return;
  }
  else {                                                      //If the switch was already on (data has been logging)
    digitalWrite(rLED, HIGH);
    ThisFile.close();                                         //Stop logging and the close the open File##
    Switch = false;                                           //Turn Switch off
    delayMicroseconds(300000);
    digitalWrite(rLED, LOW);                                  //Blink red LED once
    return;
  }
}

//==================================================================================================================

void BatteryMonitor() {     //Battery Monitoring action (Simple Moving Average)
  total -= readings[Pos];                         //Subtract the old reading from the total
  readings[Pos] = analogRead(BatteryPin);         //Add new reading in place of old reading
  total += readings[Pos];                         //Sum the new reading into the total
  (Pos < n ? Pos++ : Pos = 0);                    //Loop to beging at end of buffer
  SMA = total / n;                                //Divide the sum by the amount summed (average)
}

//==================================================================================================================

void loop() {               //Main Loop
  BatteryMonitor();                         //Check Battery...

  if (SMA < CutOFF) {                       //If the Battery voltage is too low,
    if (Switch) {                           //if data has been logging, -
      Open_or_Close();                      //Call to close the file.
      Error();                              //Blink Red LED twice...
      return;
    }
    else return;                            //- else, return to beging of loop...
  }

  Debounce();                               //Check Button...

  if (Switch) {
    if (micros() - PastMicros >= DelayTime) {
      //If the Switch is on and the time interval has been met, log a data entry below...
      PastMicros = micros();
      ThisFile << analogRead(SensorPin) << ',' << micros() << '\n';
    }
  }
}

//==================================================================================================================

void Error() {              //Simple Error (Blink Red LED twice)
  digitalWrite(rLED, HIGH);
  delayMicroseconds(200000);
  digitalWrite(rLED, LOW);
  delayMicroseconds(200000);
  digitalWrite(rLED, HIGH);
  delayMicroseconds(200000);
  digitalWrite(rLED, LOW);
}

//==================================================================================================================
