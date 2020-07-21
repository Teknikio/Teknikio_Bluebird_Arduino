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

extern "C" {
  void TIMER2_IRQHandler(void) {  
    if ((NRF_TIMER2->EVENTS_COMPARE[0] != 0) &&
      ((NRF_TIMER2->INTENSET & TIMER_INTENSET_COMPARE0_Msk) != 0)) {
        NRF_TIMER2->EVENTS_COMPARE[0] = 0; // Clear compare register 0 event
      bluebird.matrixHandler();
      NRF_TIMER2->CC[0] += 100;
    }  
  }
}


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

  row_index = 0;
  col_index = 0;

  Wire.begin();

  strip = Bluebird_NeoPixel();
  strip.updateType(NEO_GRB + NEO_KHZ800);
  strip.updateLength(1);
  strip.setPin(BLUEBIRD_NEOPIXELPIN);

  strip.begin();
  clearPixels(); // Initialize all pixels to 'off'
  strip.setBrightness(brightness);

  icm20600 = ICM20600(BLUEBIRD_ICM_ADDRESS);
  icm20600.initialize();

  servo0.attach(PIN_SERVO0);
  servo1.attach(PIN_SERVO1);

  startTimer();

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
/*!
  @brief   Adjust output brightness. Does not immediately affect what's
           currently displayed on the LEDs. 
  @param   b  Brightness setting, 0=minimum (off), 255=brightest. with a logarithmic scale
  @note    This was intended for one-time use in one's setup() function,
           not as an animation effect in itself. Because of the way this
           library "pre-multiplies" LED colors in RAM, changing the
           brightness is often a "lossy" operation -- what you write to
           pixels isn't necessary the same as what you'll read back.
           Repeated brightness changes using this function exacerbate the
           problem. Smart programs therefore treat the strip as a
           write-only resource, maintaining their own state to render each
           frame of an animation, not relying on read-modify-write.
*/
void Bluebird::setBrightness(uint8_t b) {
  int valeur;

  if (b == 0){
    strip.setBrightness(0);
    strip.show();
  }
  else if(b <6){
    valeur = int(255.0*log10((float(6)*10.0/255.0)));
    strip.setBrightness(valeur);
    strip.show();
  }
  else{
    valeur = int(255.0*log10((float(b)*10.0/255.0)));
    strip.setBrightness(valeur);
    strip.show();
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
/**************************************************************************/
/*!
    @brief reset the LED shield
    @returns none
*/
/**************************************************************************/
void Bluebird::resetLedMatrix(){
  pinMode(BLUEBIRD_LED_1, INPUT);
  pinMode(BLUEBIRD_LED_2, INPUT);
  pinMode(BLUEBIRD_LED_3, INPUT);
  pinMode(BLUEBIRD_LED_4, INPUT);
  pinMode(BLUEBIRD_LED_5, INPUT);
  pinMode(BLUEBIRD_LED_6, INPUT);
  digitalWrite(BLUEBIRD_LED_1, LOW);
  digitalWrite(BLUEBIRD_LED_2, LOW);
  digitalWrite(BLUEBIRD_LED_3, LOW);
  digitalWrite(BLUEBIRD_LED_4, LOW);
  digitalWrite(BLUEBIRD_LED_5, LOW);
  digitalWrite(BLUEBIRD_LED_6, LOW);
}

/**************************************************************************/
/*!
    @brief reset the LED shield
    @returns none
*/
/**************************************************************************/
void Bluebird::setLedMatrix(int row,int column){
  int pin_high;
  int pin_low;

  resetLedMatrix();
  
  if(row == 1   && column == 1)
  {
    pin_high  = BLUEBIRD_LED_6;
    pin_low   = BLUEBIRD_LED_1;
  }
  else if(row == 1 && column == 2)
  {
    pin_high  = BLUEBIRD_LED_6;
    pin_low   = BLUEBIRD_LED_2;
  }
  else if(row == 1 && column == 3)
  {
    pin_high  = BLUEBIRD_LED_6;
    pin_low   = BLUEBIRD_LED_3;
  }
  else if(row == 1 && column == 4)
  {
    pin_high  = BLUEBIRD_LED_6;
    pin_low   = BLUEBIRD_LED_4;
  }
  else if(row == 1 && column == 5)
  {
    pin_high  = BLUEBIRD_LED_6;
    pin_low   = BLUEBIRD_LED_5;
  }
  else if(row == 1 && column == 6)
  {
    pin_high  = BLUEBIRD_LED_5;
    pin_low   = BLUEBIRD_LED_6; 
  }
  else if(row == 2 && column == 1)
  {
    pin_high  = BLUEBIRD_LED_5;
    pin_low   = BLUEBIRD_LED_1; 
  }
  else if(row == 2 && column == 2)
  { 
    pin_high  = BLUEBIRD_LED_5;
    pin_low   = BLUEBIRD_LED_2; 
  }
  else if(row == 2 && column == 3)
  {
    pin_high  = BLUEBIRD_LED_5;
    pin_low   = BLUEBIRD_LED_3; 
  }
  else if(row == 2 && column == 4)
  {
    pin_high  = BLUEBIRD_LED_5;
    pin_low   = BLUEBIRD_LED_4; 
  }
  else if(row == 2 && column == 5)
  { 
    pin_high  = BLUEBIRD_LED_4;
    pin_low   = BLUEBIRD_LED_5; 
  }
  else if(row == 2 && column == 6)
  { 
    pin_high  = BLUEBIRD_LED_4;
    pin_low   = BLUEBIRD_LED_6; 
  }
  else if(row == 3 && column == 1)
  { 
    pin_high  = BLUEBIRD_LED_4;
    pin_low   = BLUEBIRD_LED_1; 
  }
  else if(row == 3 && column == 2)
  { 
    pin_high  = BLUEBIRD_LED_4;
    pin_low   = BLUEBIRD_LED_2; 
  }
  else if(row == 3 && column == 3)
  { 
    pin_high  = BLUEBIRD_LED_4;
    pin_low   = BLUEBIRD_LED_3; 
  }
  else if(row == 3 && column == 4)
  { 
    pin_high  = BLUEBIRD_LED_3;
    pin_low   = BLUEBIRD_LED_4; 
  }
  else if(row == 3 && column == 5)
  { 
    pin_high  = BLUEBIRD_LED_3;
    pin_low   = BLUEBIRD_LED_5; 
  }
  else if(row == 3 && column == 6)
  { 
    pin_high  = BLUEBIRD_LED_3;
    pin_low   = BLUEBIRD_LED_6; 
  }
  else if(row == 4 && column == 1)
  { 
    pin_high  = BLUEBIRD_LED_3;
    pin_low   = BLUEBIRD_LED_1; 
  }
  else if(row == 4 && column == 2)
  { 
    pin_high  = BLUEBIRD_LED_3;
    pin_low   = BLUEBIRD_LED_2; 
  }
  else if(row == 4 && column == 3)
  { 
    pin_high  = BLUEBIRD_LED_2;
    pin_low   = BLUEBIRD_LED_3; 
  }
  else if(row == 4 && column == 4)
  { 
    pin_high  = BLUEBIRD_LED_2;
    pin_low   = BLUEBIRD_LED_4; 
  }
  else if(row == 4 && column == 5)
  { 
    pin_high  = BLUEBIRD_LED_2;
    pin_low   = BLUEBIRD_LED_5; 
  }
  else if(row == 4 && column == 6)
  { 
    pin_high  = BLUEBIRD_LED_2;
    pin_low   = BLUEBIRD_LED_6; 
  }

  else if(row == 5 && column == 1)
  { 
    pin_high  = BLUEBIRD_LED_2;
    pin_low   = BLUEBIRD_LED_1; 
  }
  else if(row == 5 && column == 2)
  { 
    pin_high  = BLUEBIRD_LED_1;
    pin_low   = BLUEBIRD_LED_2; 
  }
  else if(row == 5 && column == 3)
  { 
    pin_high  = BLUEBIRD_LED_1;
    pin_low   = BLUEBIRD_LED_3; 
  }
  else if(row == 5 && column == 4)
  { 
    pin_high  = BLUEBIRD_LED_1;
    pin_low   = BLUEBIRD_LED_4; 
  }
  else if(row == 5 && column == 5)
  { 
    pin_high  = BLUEBIRD_LED_1;
    pin_low   = BLUEBIRD_LED_5; 
  }
  else if(row == 5 && column == 6)
  { 
    pin_high  = BLUEBIRD_LED_1;
    pin_low   = BLUEBIRD_LED_6; 
  }

  if( pin_high != 0 && pin_low != 0)
  {
    pinMode(pin_high, OUTPUT);
    pinMode(pin_low, OUTPUT);
    digitalWrite(pin_high, HIGH);
    digitalWrite(pin_low,LOW);
  }
}

void Bluebird::startTimer(void) {
  NRF_TIMER2->MODE = TIMER_MODE_MODE_Timer; // Set the timer in Counter Mode
  NRF_TIMER2->TASKS_CLEAR = 1; // clear the task first to be usable for later
  NRF_TIMER2->PRESCALER = 5;
  NRF_TIMER2->BITMODE =
      TIMER_BITMODE_BITMODE_16Bit; // Set counter to 16 bit resolution
  NRF_TIMER2->CC[0] = 1000;        // Set value for TIMER2 compare register 0
  NRF_TIMER2->CC[1] = 0;           // Set value for TIMER2 compare register 1

  // Enable interrupt on Timer 2, both for CC[0] and CC[1] compare match events
  NRF_TIMER2->INTENSET =
      (TIMER_INTENSET_COMPARE0_Enabled << TIMER_INTENSET_COMPARE0_Pos);
  NVIC_EnableIRQ(TIMER2_IRQn);

  NRF_TIMER2->TASKS_START = 1; // Start TIMER2
}

/*!
 *    @brief Matrix object function called by IRQ handler for each row
 *    This is not optimized at all but its not so bad either!
 */
void Bluebird::matrixHandler(void) {
  // disable current row
  if(ledpattern[row_index][col_index] == 1)
  {
    setLedMatrix(row_index+1,col_index+1);
  }

  if( row_index<(ROW_SIZE-1))
  {
    row_index++;
  }
  else
  {
    row_index = 0;
      if( col_index<(COLUMN_SIZE -1))
      {
        col_index++;
      }
      else
      {
        col_index = 0;
      }
    }
}

void Bluebird::changepattern(void *m_ledpattern){
  memcpy(*ledpattern, m_ledpattern, sizeof(ledpattern));  
}
// instantiate static
Bluebird bluebird;
