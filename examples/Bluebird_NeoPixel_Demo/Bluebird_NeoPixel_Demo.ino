#include <Bluebird_Teknikio.h>
//Bluebird Neopixel demo

void setup() {
 bluebird.begin();
 Serial.begin(9600);
     Serial.println("NEOPIXEL test: START");


}

void loop() {

/*Make LED onBluebird fade up to and down from maximum brightness.
Warning LED gets VERY bright.*/
//fade up to 255
   bluebird.setPixelColor(0,255,255,255 ); // Led is set to WHITE
   Serial.println("Bluebird Brightness test: fading up to 100%");
   for(int i =1 ; i<255;i++){
    bluebird.setBrightness(i);
    delay(50);
  }
  
    //fade down to 0
  Serial.println("Bluebird Brightness test: fading down to 0%");
  
  for(int i =0 ; i<255;i++){
    bluebird.setBrightness(255-i);
    delay(50);
  }
  bluebird.setBrightness(0); //turn LED off
  Serial.println("Bluebird Brightness test: END");
  delay(100);

  
  // COLOR TEST
  //to setPixelColor use (0, R, G, B)
  //0 refers to position of neopixel
  //RGB value can range bewteen 0 and 255

  //rainbow cycle
  bluebird.setBrightness(10);
  Serial.println("Bluebird RAINBOW CYCLE");
  bluebird.setPixelColor(0,255,0,0 ); // Red
  delay(200);
  bluebird.setPixelColor(0,255,105,0 ); // Orange
  delay(200);
  bluebird.setPixelColor(0,255,235,0 ); // Yellow
  delay(200);
  bluebird.setPixelColor(0,0,255,0 ); // Green
  delay(200);
  bluebird.setPixelColor(0, 0,255,255); //Teal
  delay(100);
  bluebird.setPixelColor(0,0,0,255 ); // Blue
  delay(100);
  bluebird.setPixelColor(0,255,0,255 ); // purple
  delay(100);
  bluebird.setPixelColor(0,255,255,255 ); // white
  delay(100);

    Serial.println("NEOPIXEL test: END");

  
}
