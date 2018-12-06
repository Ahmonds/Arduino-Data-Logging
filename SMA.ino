#define n  100
#define SensorPin 15  //A1

int readings[n];
byte Pos = 0;
float total = 0;
float average = 0;

void setup() {
  while (!Serial) {}
  Serial.begin(115200);

  for (int thisReading = 0; thisReading < n; thisReading++) {
    readings[thisReading] = 0;
  }
}

void SMA () {
  total -= readings[Pos];   //subtract old reading from total
  readings[Pos] = analogRead(SensorPin);;   //place new reading into the array
  total += readings[Pos];   //add the new reading to the total
  (Pos < n ? Pos++ : Pos = 0);  //Incament Pos until the end is reached
  average = total / n;   //take the current average
}

void loop() {

  SMA();    // Call to run the "Simple Moving Average" action

  Serial.print(average);
  Serial.print(" , ");
  Serial.println(analogRead(SensorPin));
}
