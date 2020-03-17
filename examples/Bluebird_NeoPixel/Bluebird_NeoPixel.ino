#include <Bluebird_Circuit_Playground.h>

uint8_t pixeln = 1;

void setup() {
 bluebirdCircuitPlayground.begin();
 bluebirdCircuitPlayground.setPixelColor(0, bluebirdCircuitPlayground.colorWheel(25 * pixeln));
 //bluebirdCircuitPlayground.setPixelColor(0,255,255,255 );
 delay(500);
 
}

void loop() {
  // Define the color by a position on the color wheel
  bluebirdCircuitPlayground.setPixelColor(0, bluebirdCircuitPlayground.colorWheel(25 * pixeln));
  // Can be used in RGB mode
  //bluebirdCircuitPlayground.setPixelColor(0,255,0,0 );
  delay(500);
  bluebirdCircuitPlayground.setPixelColor(0, bluebirdCircuitPlayground.colorWheel(125 * pixeln));
  //bluebirdCircuitPlayground.setPixelColor(0,0,255,0 );
  delay(500);



  
  Serial.println("Bluebird Neopixel test: End of Sequence");
}
