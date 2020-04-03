#include <Bluebird_Teknikio.h>



void setup() {
  // put your setup code here, to run once:
  Serial.begin(9600);
  delay(4000);
  Serial.println("Bluebird Light Sensor test: START");
  bluebird.begin();

  bluebird.setPixelColor(0,0,0,0 ); // Red
  delay(50);
}

void loop() {
  // put your main code here, to run repeatedly:
  Serial.print("Light = ");
  Serial.print(bluebird.lightSensor());
  Serial.println(" ");
  delay(100);
}
