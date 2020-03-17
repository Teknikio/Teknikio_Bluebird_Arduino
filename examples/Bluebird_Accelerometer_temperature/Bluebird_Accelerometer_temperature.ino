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
  Serial.print("x = ");
  Serial.print(bluebirdCircuitPlayground.rotationX());
  Serial.println(" dps");
  Serial.print("y = ");
  Serial.print(bluebirdCircuitPlayground.rotationY());
  Serial.println(" dps");
  Serial.print("z = ");
  Serial.print(bluebirdCircuitPlayground.rotationZ());
  Serial.println(" dps");
  Serial.print("x = ");
  Serial.print(bluebirdCircuitPlayground.motionX());
  Serial.println(" mg");
  Serial.print("y = ");
  Serial.print(bluebirdCircuitPlayground.motionY());
  Serial.println(" mg");
  Serial.print("z = ");
  Serial.print(bluebirdCircuitPlayground.motionZ());
  Serial.println(" mg");
  Serial.print("t = ");
  Serial.print(bluebirdCircuitPlayground.icm20600.getTemperature());
  Serial.println(" °C");
  delay(100);
  Serial.println("Bluebird Neopixel test: End of Sequence");
}
