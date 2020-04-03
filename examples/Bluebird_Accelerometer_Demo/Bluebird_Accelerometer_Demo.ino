#include <Bluebird_Teknikio.h>

//Bluebird Accelerometer demo

void setup() {
  // put your setup code here, to run once:
  Serial.begin(9600);
  delay(4000);
  bluebird.begin();
  delay(50);
}

void loop() { 
  Serial.print("Accelerometer values:");
  Serial.print("\t x = ");
  Serial.print(bluebird.rotationX());
  Serial.print(" dps");
  Serial.print("\t y = ");
  Serial.print(bluebird.rotationY());
  Serial.print(" dps");
  Serial.print("\t z = ");
  Serial.print(bluebird.rotationZ());
  Serial.print(" dps");
  Serial.print("\t  x = "); 
  Serial.print(bluebird.motionX());
  Serial.print(" mg-force");
  delay(100);
  Serial.print("\t  y = ");
  Serial.print(bluebird.motionY());
  Serial.print(" mg-force");
  delay(100);
  Serial.print(" \t  z = ");
  Serial.print(bluebird.motionZ());
  Serial.print("  mg-force");
  Serial.println();
  delay(500);
}
