//Learn to light up LEDs on the LED matrix individually

#include <Bluebird_Teknikio.h>

void setup() {
  // put your setup code here, to run once:
  Serial.begin(9600);
  delay(400);
  bluebird.begin();
  Serial.println("START");

}

void loop() {

  // put your main code here, to run repeatedly:

  bluebird.setLEDMatrix(1, 1); // light up LEDs individually using setLedMatrix(int row,int column)

  //now let's scroll through all of the LEDs!
  for (int r = 1; r < 6; r++) {
    for (int c = 1; c < 7; c++) {
      bluebird.setLEDMatrix( r, c);
      delay(100);
    }
  }

}