#include <Bluebird_Circuit_Playground.h>

uint8_t pixeln = 1;
byte val = 0;
int16_t motionZ;
uint16_t measure = 0;

void setup() {
  // put your setup code here, to run once:
  bluebirdCircuitPlayground.begin();
  delay(50);

  //bluebirdCircuitPlayground.speaker.enable(true);
  bluebirdCircuitPlayground.playTone(500 + pixeln * 500, 100);

}

void loop() {
  // put your main code here, to run repeatedly:
  delay(100);
  bluebirdCircuitPlayground.playTone(500 + pixeln * 500, 100);
  delay(100);
  bluebirdCircuitPlayground.playTone(500 + (pixeln+5) * 500, 100);
  delay(100);

}
