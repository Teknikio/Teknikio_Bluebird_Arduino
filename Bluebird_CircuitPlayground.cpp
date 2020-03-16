/*!
 * @file Adafruit_CircuitPlayground.cpp
 *
 * @mainpage Adafruit CircuitPlayground Library
 *
 * @section intro_sec Introduction
 *
 * This is the documentation for Adafruit's CircuitPlayground driver for the
 * Arduino platform.  It is designed specifically to work with the
 * Adafruit CircuitPlayground boards:
 *  - https://www.adafruit.com/products/3000
 *  - https://www.adafruit.com/products/3333
 *
 *
 * Adafruit invests time and resources providing this open source code,
 * please support Adafruit and open-source hardware by purchasing
 * products from Adafruit!
 *
 *
 * @section author Author
 *
 * Written by Ladyada and others for Adafruit Industries.
 *
 * @section license License
 *
 * BSD license, all text here must be included in any redistribution.
 *
 */

#include "Bluebird_CircuitPlayground.h"

/**************************************************************************/
/*!
    @brief  Set up the CircuitPlayground hardware
    @param  brightness Optional brightness to set the neopixels to
    @returns True if device is set up, false on any failure
*/
/**************************************************************************/
bool Bluebird_CircuitPlayground::begin(uint8_t brightness) {

  // Pin Initialisation
  pinMode(BLUEBIRD_BUZZER, OUTPUT);
  pinMode(BLUEBIRD_COLOR_ENABLE,OUTPUT);
  pinMode(BLUEBIRD_WIRE_INT,INPUT);
  pinMode(BLUEBIRD_LIGHTSENSOR,INPUT);
  

  digitalWrite(BLUEBIRD_BUZZER      , LOW); 
  digitalWrite(BLUEBIRD_COLOR_ENABLE, LOW); 

  Wire.begin();

  //strip = Adafruit_CPlay_NeoPixel(1,BLUEBIRD_NEOPIXELPIN,NEO_GRB + NEO_KHZ800);
  strip = Adafruit_CPlay_NeoPixel();
  strip.updateType(NEO_GRB + NEO_KHZ800);
  strip.updateLength(1);
  strip.setPin(BLUEBIRD_NEOPIXELPIN);

//  lis = Adafruit_CPlay_LIS3DH(&Wire1); // i2c on wire1

  strip.begin();
  strip.show(); // Initialize all pixels to 'off'
  strip.setBrightness(brightness);

  icm20600 = ICM20600(BLUEBIRD_ICM_ADDRESS);
  icm20600.initialize();


  //return lis.begin(CPLAY_LIS3DH_ADDRESS);
  return true;
}

/**************************************************************************/
/*!
    @brief play a tone on the onboard buzzer
    @param  freq the frequency to play
    @param  time the duration of the tone in milliseconds
    @param  wait Optional flag to wait for time milliseconds after playing the tone. Defaults to true.
    @note The driver circuitry is an on/off transistor driver, so you will only be able to play square waves.
    It is also not the same loudness over all frequencies but is designed to be the loudest at around 4 KHz
*/
/**************************************************************************/
void Bluebird_CircuitPlayground::playTone(
  uint16_t freq, uint16_t time, bool wait) {
  tone(BLUEBIRD_BUZZER, freq, time);
  delay(time); // time argument to tone() isn't working, so...
  if(wait) delay(time);
}

/**************************************************************************/
/*!
    @brief read the onboard lightsensor
    @returns value between 0 and 1023 read from the light sensor
    @note 1000 Lux will roughly read as 2 Volts (or about 680 as a raw analog reading).
      A reading of about 300 is common for most indoor light levels.
      Note that outdoor daylight is 10,000 Lux or even higher, so this sensor is best
      suited for indoor light levels!
*/
/**************************************************************************/
uint16_t Bluebird_CircuitPlayground::lightSensor(void) {
  uint16_t tmp_measure = 0;
  digitalWrite(BLUEBIRD_COLOR_ENABLE, HIGH); 
  delay(50);
  tmp_measure = analogRead(BLUEBIRD_LIGHTSENSOR);
  digitalWrite(BLUEBIRD_COLOR_ENABLE, LOW); 
  return tmp_measure;
}

/**************************************************************************/
/*!
    @brief get a color value from the color wheel.
    @param WheelPos a value 0 to 255
    @returns a color value. The colours are a transition r - g - b - back to r.
*/
/**************************************************************************/
uint32_t Bluebird_CircuitPlayground::colorWheel(uint8_t WheelPos) {
  WheelPos = 255 - WheelPos;
  if (WheelPos < 85) {
    return strip.Color(255 - WheelPos * 3, 0, WheelPos * 3);
  }
  if (WheelPos < 170) {
    WheelPos -= 85;
    return strip.Color(0, WheelPos * 3, 255 - WheelPos * 3);
  }
  WheelPos -= 170;
  return strip.Color(WheelPos * 3, 255 - WheelPos * 3, 0);
}

/**************************************************************************/
/*!
    @brief detect a color value from the light sensor
    @param red the pointer to where the red component should be stored.
    @param green the pointer to where the green component should be stored.
    @param blue the pointer to where the blue component should be stored.
*/
/**************************************************************************/
void Bluebird_CircuitPlayground::senseColor(uint8_t& red, uint8_t& green, uint8_t& blue) {
  // Save the current pixel brightness so it can later be restored.  Then bump
  // the brightness to max to make sure the LED is as bright as possible for
  // the color readings.
  uint8_t old_brightness = strip.getBrightness();
  strip.setBrightness(255);
  // Set pixel 1 (next to the light sensor) to full red, green, blue
  // color and grab a light sensor reading.  Make sure to wait a bit
  // after changing pixel colors to let the light sensor change
  // resistance!
  setPixelColor(1, 255, 0, 0);  // Red
  delay(LIGHT_SETTLE_MS);
  uint16_t raw_red = lightSensor();
  setPixelColor(1, 0, 255, 0);  // Green
  delay(LIGHT_SETTLE_MS);
  uint16_t raw_green = lightSensor();
  setPixelColor(1, 0, 0, 255);  // Blue
  delay(LIGHT_SETTLE_MS);
  uint16_t raw_blue = lightSensor();
  // Turn off the pixel and restore brightness, we're done with readings.
  setPixelColor(1, 0);
  strip.setBrightness(old_brightness);
  // Now scale down each of the raw readings to be within
  // 0 to 255.  Remember each sensor reading is from the ADC
  // which has 10 bits of resolution (0 to 1023), so dividing
  // by 4 will change the range from 0-1023 to 0-255.  Also
  // use the min function to clamp the value to 255 at most (just
  // to prevent overflow from 255.xx to 0).
  red = min(255, raw_red/4);
  green = min(255, raw_green/4);
  blue = min(255, raw_blue/4);
}

/**************************************************************************/
/*!
    @brief check whether or not this device is a CircuitPlayground Express.
    @returns True if the device is a CircuitPlayground Express, false if it is a 'classic'.
*/
/**************************************************************************/
bool Bluebird_CircuitPlayground::isExpress(void) {
#ifdef __AVR__
  return false;
#else
  return true;
#endif
}


int16_t Bluebird_CircuitPlayground::motionX(){
  return icm20600.getAccelerationX();
}
int16_t Bluebird_CircuitPlayground::motionY(){
  return icm20600.getAccelerationY();
}
int16_t Bluebird_CircuitPlayground::motionZ(){
  return icm20600.getAccelerationZ();
}

int16_t Bluebird_CircuitPlayground::rotationX(){
  return icm20600.getGyroscopeX();
}
int16_t Bluebird_CircuitPlayground::rotationY(){
  return icm20600.getGyroscopeY();
}
int16_t Bluebird_CircuitPlayground::rotationZ(){
  return icm20600.getGyroscopeZ();
}

// instantiate static
Bluebird_CircuitPlayground bluebirdCircuitPlayground;
