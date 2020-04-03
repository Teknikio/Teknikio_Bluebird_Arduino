#include <Bluebird_Teknikio.h>
//Bluebird Library and board demo of all sensors, neopixel and speaker


uint8_t value_red = 0;
uint8_t value_blue = 0;
uint8_t value_green = 0;

void setup() {
  // put your setup code here, to run once:
  Serial.begin(9600);
  Serial.println("Bluebird full test: START");
  bluebird.begin();

}

void loop() {
  Serial.println();
  Serial.println("Bluebird full test: START");

  //Rainbow cycle
  Serial.println();

  Serial.println("LED Rainbow cycle");
  bluebird.setPixelColor(0, 255, 0, 0 ); // Red
  delay(200);
  bluebird.setPixelColor(0, 255, 105, 0 ); // Orange
  delay(200);
  bluebird.setPixelColor(0, 255, 235, 0 ); // Yellow
  delay(200);
  bluebird.setPixelColor(0, 0, 255, 0 ); // Green
  delay(200);
  bluebird.setPixelColor(0, 0, 255, 255); //Teal
  delay(100);
  bluebird.setPixelColor(0, 0, 0, 255 ); // Blue
  delay(100);
  bluebird.setPixelColor(0, 255, 0, 255 ); // purple
  delay(100);
  bluebird.setPixelColor(0, 255, 255, 255 ); // white
  Serial.println();

  delay(500);

  //Play a C-scale
  Serial.println("Speaker Play C scale");
  int melody[] = {
    NOTE_C5, NOTE_D5, NOTE_E5, NOTE_F5,  NOTE_G5, NOTE_A5, NOTE_B5, NOTE_C6
  };
  for (int n = 0; n < 8; n++) {
      bluebird.playTone(melody[n], 100);
    delay(100);
  }
  Serial.println();

  //Display Temperature Data in Celsius
  Serial.print("Temperature: ");
  Serial.print(bluebird.getTemperature());
  Serial.print(" °C");
  delay(500);
  Serial.println();

  //Display Light Sensor Data
  Serial.println();

  Serial.print("Light Level= ");
  Serial.print(bluebird.lightSensor() * 3.2);
  //multiply by 3.2 to convert to lux
  Serial.print("lux");
  delay(500);
  Serial.println();


  //Display Color Sensor Data. This is not calibrated, use Calibration Example for accurate reading.
  Serial.println();
  Serial.println("Color percent, place a solid color over LED and light sensor: ");
  delay(1000); // time to place object
  bluebird.senseColor(value_red, value_green, value_blue);
  Serial.print("Red : ");
  Serial.print(value_red);
  Serial.print("%");
  Serial.print("\t Green : ");
  Serial.print(value_green);
  Serial.print("%");
  Serial.print("\t Blue : ");
  Serial.print(value_blue);
  Serial.print("%");
  Serial.println();

  delay(500);

  //  Accelerometer data
  Serial.println();
  Serial.println("Accelerometer rotation: ");
  Serial.print(" x = ");
  Serial.print(bluebird.rotationX());
  Serial.print(" dps");
  Serial.print("\t y = ");
  Serial.print(bluebird.rotationY());
  Serial.print(" dps");
  Serial.print("\t z = ");
  Serial.print(bluebird.rotationZ());
  Serial.print(" dps");
  delay(100);

  Serial.println();
  Serial.println();

  Serial.println("Bluebird full Test: END");
  Serial.println("*******************************");
  delay(5000); //pause before repeating


}
