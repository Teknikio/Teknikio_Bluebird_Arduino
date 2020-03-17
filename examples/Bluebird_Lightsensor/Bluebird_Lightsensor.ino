#include <Bluebird_Circuit_Playground.h>

void setup() {
  // put your setup code here, to run once:
  Serial.begin(9600);
  delay(4000);
  Serial.println("Bluebird Neopixel test: START");
  bluebirdCircuitPlayground.begin();
  delay(50);
}

void loop() {
  // put your main code here, to run repeatedly:
  Serial.print("Light = ");
  Serial.print(bluebirdCircuitPlayground.lightSensor());
  Serial.println(" ");
  delay(100);
}
