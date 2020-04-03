/*!
 *
 * @section intro_sec Introduction
 *
 * This is adapted by Teknikio  from Adafruit's CircuitPlayground driver for the
 * Arduino platform.  It is designed specifically to work with the
 * Teknikio Bluebird boards.
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

#include "Bluebird_Teknikio.h"

/**************************************************************************/
/*!
    @brief  Set up the Bluebird hardware
    @param  brightness Optional brightness to set the neopixels to
    @returns True if device is set up, false on any failure
*/
/**************************************************************************/
bool Bluebird::begin(uint8_t brightness) {

  // Pin Initialisation
  pinMode(BLUEBIRD_BUZZER, OUTPUT);
  pinMode(BLUEBIRD_COLOR_ENABLE,OUTPUT);
  pinMode(BLUEBIRD_WIRE_INT,INPUT);
  pinMode(BLUEBIRD_LIGHTSENSOR,INPUT);
  

  digitalWrite(BLUEBIRD_BUZZER      , LOW); 
  digitalWrite(BLUEBIRD_COLOR_ENABLE, LOW); 

  Wire.begin();

  //strip = Bluebird_NeoPixel(1,BLUEBIRD_NEOPIXELPIN,NEO_GRB + NEO_KHZ800);
  strip = Bluebird_NeoPixel();
  strip.updateType(NEO_GRB + NEO_KHZ800);
  strip.updateLength(1);
  strip.setPin(BLUEBIRD_NEOPIXELPIN);

  strip.begin();
  clearPixels(); // Initialize all pixels to 'off'
  strip.setBrightness(brightness);

  icm20600 = ICM20600(BLUEBIRD_ICM_ADDRESS);
  icm20600.initialize();

  min_red = 255;
  min_green = 255;
  min_blue = 255;

  max_red = 0;
  max_green = 0;
  max_blue = 0;


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
void Bluebird::playTone(
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
uint16_t Bluebird::lightSensor(void) {
  uint16_t tmp_measure = 0;
  digitalWrite(BLUEBIRD_COLOR_ENABLE, HIGH); 
  delay(50);
  tmp_measure = analogRead(BLUEBIRD_LIGHTSENSOR);
  digitalWrite(BLUEBIRD_COLOR_ENABLE, LOW); 
  return tmp_measure;
}

/**************************************************************************/
/*!
    @brief detect a color value from the light sensor
    @param red the pointer to where the red component should be stored.
    @param green the pointer to where the green component should be stored.
    @param blue the pointer to where the blue component should be stored.
*/
/**************************************************************************/
void Bluebird::senseColor(uint8_t& red, uint8_t& green, uint8_t& blue) {
  // Save the current pixel brightness so it can later be restored.  Then bump
  // the brightness to max to make sure the LED is as bright as possible for
  // the color readings.
  uint8_t old_brightness = strip.getBrightness();
  strip.setBrightness(255);
  // Set pixel 1 (next to the light sensor) to full red, green, blue
  // color and grab a light sensor reading.  Make sure to wait a bit
  // after changing pixel colors to let the light sensor change
  // resistance!
  setPixelColor(0, 255, 0, 0);  // Red
  delay(LIGHT_SETTLE_MS);
  uint16_t raw_red = lightSensor();
  setPixelColor(0, 0, 255, 0);  // Green
  delay(LIGHT_SETTLE_MS);
  uint16_t raw_green = lightSensor();
  setPixelColor(0, 0, 0, 255);  // Blue
  delay(LIGHT_SETTLE_MS);
  uint16_t raw_blue = lightSensor();
  // Turn off the pixel and restore brightness, we're done with readings.
  setPixelColor(0, 0);
  strip.setBrightness(old_brightness);
  // Now scale down each of the raw readings to be within
  // 0 to 255.  Remember each sensor reading is from the ADC
  // which has 10 bits of resolution (0 to 1023), so dividing
  // by 4 will change the range from 0-1023 to 0-255.  Also
  // use the min function to clamp the value to 255 at most (just
  // to prevent overflow from 255.xx to 0).

  if( ((min_red == 255) || (max_red == 0))||((min_green == 255) || (max_green == 0))||((min_blue == 255) || (max_blue == 0))){
    red = min(255, raw_red/4);
    green = min(255, raw_green/4);
    blue = min(255, raw_blue/4);  
  }
  else
  {
    red = (((raw_red/4)-min_red)*100)/max_red;
    green = (((raw_green/4)-min_green)*100)/max_green;
    blue = (((raw_blue/4)-min_blue)*100)/max_blue;
  }
  
}

/**************************************************************************/
/*!
    @brief measure the value, and integers the max/min inside the bluebird
    @param mode the milestone to be modified (min or max)
    @param red the pointer to where the red component should be stored.
    @param green the pointer to where the green component should be stored.
    @param blue the pointer to where the blue component should be stored.
*/
/**************************************************************************/

void Bluebird::calibratesenseColor(uint8_t mode,uint8_t& red, uint8_t& green, uint8_t& blue){

  uint8_t red_tmp;
  uint8_t green_tmp;
  uint8_t blue_tmp;


  if(mode == BLUEBIRD_MIN_CALIB){
    for (int i =0 ;i < BLUEBIRD_CALIB_SAMPLES; i++)
    {
       uint8_t old_brightness = strip.getBrightness();
      strip.setBrightness(255);
      // Set pixel 1 (next to the light sensor) to full red, green, blue
      // color and grab a light sensor reading.  Make sure to wait a bit
      // after changing pixel colors to let the light sensor change
      // resistance!
      setPixelColor(0, 255, 0, 0);  // Red
      delay(LIGHT_SETTLE_MS);
      uint16_t raw_red = lightSensor();
      setPixelColor(0, 0, 255, 0);  // Green
      delay(LIGHT_SETTLE_MS);
      uint16_t raw_green = lightSensor();
      setPixelColor(0, 0, 0, 255);  // Blue
      delay(LIGHT_SETTLE_MS);
      uint16_t raw_blue = lightSensor();
      // Turn off the pixel and restore brightness, we're done with readings.
      setPixelColor(0, 0);
      strip.setBrightness(old_brightness);
      // Scale between 0 and 255
      red_tmp = min(255, raw_red/4);
      green_tmp = min(255, raw_green/4);
      blue_tmp = min(255, raw_blue/4);  
      // Store the value if necessary
      red = min(red_tmp,red);
      green = min(green_tmp,green);
      blue = min(blue_tmp,blue);
    }
  }
  else if(mode == BLUEBIRD_MAX_CALIB)
  {
    for (int i =0 ;i < BLUEBIRD_CALIB_SAMPLES; i++)
    {
      uint8_t old_brightness = strip.getBrightness();
      strip.setBrightness(255);
      // Set pixel 1 (next to the light sensor) to full red, green, blue
      // color and grab a light sensor reading.  Make sure to wait a bit
      // after changing pixel colors to let the light sensor change
      // resistance!
      setPixelColor(0, 255, 0, 0);  // Red
      delay(LIGHT_SETTLE_MS);
      uint16_t raw_red = lightSensor();
      setPixelColor(0, 0, 255, 0);  // Green
      delay(LIGHT_SETTLE_MS);
      uint16_t raw_green = lightSensor();
      setPixelColor(0, 0, 0, 255);  // Blue
      delay(LIGHT_SETTLE_MS);
      uint16_t raw_blue = lightSensor();
      // Turn off the pixel and restore brightness, we're done with readings.
      setPixelColor(0, 0);
      strip.setBrightness(old_brightness);
      // Scale between 0 and 255
      red_tmp = min(255, raw_red/4);
      green_tmp = min(255, raw_green/4);
      blue_tmp = min(255, raw_blue/4);  
      // Store the value if needed
      red = max(red_tmp,red);
      green = max(green_tmp,green);
      blue = max(blue_tmp,blue);
    } 
  }
}


/**************************************************************************/
/*!
    @brief return the value for the X axis acceleration
    @returns value in mg read from the accelerometer
*/
/**************************************************************************/
int16_t Bluebird::motionX(){
  return icm20600.getAccelerationX();
}

/**************************************************************************/
/*!
    @brief return the value for the Y axis acceleration
    @returns value in mg read from the accelerometer
*/
/**************************************************************************/
int16_t Bluebird::motionY(){
  return icm20600.getAccelerationY();
}

/**************************************************************************/
/*!
    @brief return the value for the Z axis acceleration
    @returns value in mg read from the accelerometer
*/
/**************************************************************************/
int16_t Bluebird::motionZ(){
  return icm20600.getAccelerationZ();
}
/**************************************************************************/
/*!
    @brief return the value for the X axis gyroscope
    @returns value in dps read from the gyroscope
*/
/**************************************************************************/
int16_t Bluebird::rotationX(){
  return icm20600.getGyroscopeX();
}
/**************************************************************************/
/*!
    @brief return the value for the Y axis gyroscope
    @returns value in dps read from the gyroscope
*/
/**************************************************************************/
int16_t Bluebird::rotationY(){
  return icm20600.getGyroscopeY();
}
/**************************************************************************/
/*!
    @brief return the value for the Z axis gyroscope
    @returns value in dps read from the gyroscope
*/
/**************************************************************************/
int16_t Bluebird::rotationZ(){
  return icm20600.getGyroscopeZ();
}
/**************************************************************************/
/*!
    @brief return the value from the thermometer
    @returns value in °C
*/
/**************************************************************************/
int16_t Bluebird::getTemperature(){
  return bluebird.icm20600.getTemperature();
}
// instantiate static
Bluebird bluebird;
