#include <Bluebird_Circuit_Playground.h>



uint8_t pixeln = 1;
byte val = 0;

void setup() {
  // put your setup code here, to run once:
 Serial.begin(9600);
 Serial.println("Bluebird Neopixel test: START");
 bluebirdCircuitPlayground.begin();

 bluebirdCircuitPlayground.setPixelColor(0, bluebirdCircuitPlayground.colorWheel(25 * pixeln));
 delay(500);
 //bluebirdCircuitPlayground.icm20600.initialize();
 
 bluebirdCircuitPlayground.speaker.enable(true);
 bluebirdCircuitPlayground.playTone(500 + pixeln * 500, 100);
 
}

void loop() {
  // put your main code here, to run repeatedly:
  bluebirdCircuitPlayground.setPixelColor(0, bluebirdCircuitPlayground.colorWheel(25 * pixeln));
  delay(500);
  bluebirdCircuitPlayground.setPixelColor(0, bluebirdCircuitPlayground.colorWheel(125 * pixeln));
  delay(500);
  Serial.println("Bluebird Neopixel test: End of Sequence");
}
