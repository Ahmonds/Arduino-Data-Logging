#define gLED 8
#define rLED 13
#define ButtonPin 16

#define dbDelay 100
bool Switch = false;
bool ButtonState = false;
uint32_t PrevBounceMillis = 0;
byte ButtonReading = LOW;
uint32_t SerialMillis = 0;

void setup () {
  Serial.begin(115200);
  pinMode(gLED, OUTPUT);
  pinMode(rLED, OUTPUT);
  pinMode(ButtonPin, INPUT_PULLUP);
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
    }
  }
}

void loop () {

  Debounce();

  if (millis() - SerialMillis > 500) {
    Serial.print("Switch state is ");
    Serial.println(Switch);
    SerialMillis = millis();
  }

  digitalWrite(gLED, Switch);
  digitalWrite(rLED, ButtonState);
}
