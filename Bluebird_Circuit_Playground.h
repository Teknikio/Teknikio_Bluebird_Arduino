/*!
 * @file Adafruit_Circuit_Playground.h
 *
 * This is part of Adafruit's CircuitPlayground driver for the Arduino platform.  It is
 * designed specifically to work with the Adafruit CircuitPlayground boards.
 *
 * Adafruit invests time and resources providing this open source code,
 * please support Adafruit and open-source hardware by purchasing
 * products from Adafruit!
 *
 * Written by Ladyada and others for Adafruit Industries.
 *
 * BSD license, all text here must be included in any redistribution.
 *
 */

#ifndef _BLUEBIRD_CIRCUITPLAYGROUND_H_
#define _BLUEBIRD_CIRCUITPLAYGROUND_H_

#include <Arduino.h>
#include "utility/Adafruit_CPlay_NeoPixel.h"
#include "utility/CP_Firmata.h"
#include "utility/ICM20600.h"

#ifndef NOT_AN_INTERRUPT // Not defined in Arduino 1.0.5
  #define NOT_AN_INTERRUPT -1 ///< Pin is not on an interrupt
#endif

#define BLUEBIRD_NEOPIXELPIN  PIN_NEOPIXEL ///< neopixel pin
#define BLUEBIRD_BUZZER       PIN_BUZZER ///< buzzer pin
#define BLUEBIRD_LIGHTSENSOR  A4 ///< light sensor pin  
#define BLUEBIRD_WIRE_INT     PIN_WIRE_INT
#define BLUEBIRD_COLOR_ENABLE PIN_COLOR_ENABLE
#define BLUEBIRD_ICM_ADDRESS  false


/*! 
  @brief Configuration to tune the color sensing logic:
   Amount of time (in milliseconds) to wait between 
   changing the pixel color and reading the light
    sensor.
*/
#define LIGHT_SETTLE_MS 100  

/**************************************************************************/
/*! 
    @brief  Class that stores state and functions for interacting with CircuitPlayground hardware
*/
/**************************************************************************/
class Bluebird_CircuitPlayground {
 public:
  bool begin(uint8_t brightness=20);

  Adafruit_CPlay_NeoPixel strip; ///< the neopixel strip object
  ICM20600 icm20600;

  void playTone(uint16_t freq, uint16_t time, bool wait=true);
  uint16_t lightSensor(void);
//  float temperature(void);
//  float temperatureF(void);

  // Accelerometer
  int16_t motionX(void);
  int16_t motionY(void);
  int16_t motionZ(void);

  int16_t rotationX(void);
  int16_t rotationY(void);
  int16_t rotationZ(void);

/**************************************************************************/
/*! 
  @brief set the range of the MEMS accelerometer.
  @param range the range to set the accelerometer to. LIS3DH_RANGE_2_G
        is the smallest (+-2G) but will give the greatest precision, while LIS3DH_RANGE_8_G
        is the largest (+-8G) but with the lease precision. LIS3DH_RANGE_4_G is in the middle.
*/
/**************************************************************************/
  //void setAccelRange(lis3dh_range_t range) { lis.setRange(range); }
/**************************************************************************/
/*! 
  @brief turn on tap detection. Tap detection can detect single taps or 'double taps' 
  (like a double-click).
  @param c If c is 1 you will only detect single taps, one at a time. 
            If c is 2, you will be able to detect both single taps and double taps.
  @param clickthresh the threshold over which to register a tap
*/
/**************************************************************************/
 // void setAccelTap(uint8_t c, uint8_t clickthresh) 
 //   { lis.setClick(c, clickthresh, 10, 20, 255); }

/**************************************************************************/
/*! 
  @brief test whether or not a tap has been detected
  @return 0 if no tap is detected, 1 if a single tap is detected, and 2 or 3 if double tap is detected. 
*/
/**************************************************************************/
  //uint8_t getAccelTap(void) { return (lis.getClick() >> 8) & 0x3; }


/**************************************************************************/
/*! 
  @brief turn off all neopixels on the board
*/
/**************************************************************************/
  void clearPixels(void) { strip.clear(); strip.show(); }

/**************************************************************************/
/*! 
  @brief set the color of a neopixel on the board
  @param p the pixel to set. Pixel 0 is above the pad labeled 'GND' right next to the
      USB connector, while pixel 9 is above the pad labeled '3.3V' on the other side of
      the USB connector.
  @param c a 24bit color value to set the pixel to
*/
/**************************************************************************/
  void setPixelColor(uint8_t p, uint32_t c) {strip.setPixelColor(p, c); strip.show();}

/**************************************************************************/
/*! 
  @brief set the color of a neopixel on the board
  @param p the pixel to set. Pixel 0 is above the pad labeled 'GND' right next to the
      USB connector, while pixel 9 is above the pad labeled '3.3V' on the other side of
      the USB connector.
  @param r a 0 to 255 value corresponding to the red component of the desired color.  
  @param g a 0 to 255 value corresponding to the green component of the desired color.
  @param b a 0 to 255 value corresponding to the blue component of the desired color.  
*/
/**************************************************************************/
  void setPixelColor(uint8_t p, uint8_t r, uint8_t g, uint8_t b) {strip.setPixelColor(p, r, g, b); strip.show();}
  
/*!  @brief set the global brightness of all neopixels.
     @param b a 0 to 255 value corresponding to the desired brightness. The default brightness
     of all neopixels is 30. */
  void setBrightness(uint16_t b){strip.setBrightness(b);}

/*!  @brief Get a sinusoidal value from a sine table
     @param x a 0 to 255 value corresponding to an index to the sine table
     @returns An 8-bit sinusoidal value back */
  uint8_t sine8(uint8_t x) { return strip.sine8(x); }

/*!  @brief Get a gamma-corrected value from a gamma table
     @param x a 0 to 255 value corresponding to an index to the gamma table
     @returns An 8-bit gamma-corrected value back */
  uint8_t gamma8(uint8_t x) { return strip.gamma8(x); }

  uint32_t colorWheel(uint8_t x);

  // Basic RGB color sensing with the light sensor and nearby neopixel.
  // Both functions do the same thing and just differ in how they return the
  // result, either as explicit RGB bytes or a 24-bit RGB color value.
  void senseColor(uint8_t& red, uint8_t& green, uint8_t& blue);

/**************************************************************************/
/*! 
  @brief detect a color using the onboard light sensor
  @return a 24 bit color. The most significant byte is red, followed by green, and
          the least significant byte is blue.
*/
/**************************************************************************/
  uint32_t senseColor() {
    // Use the individual color component color sense function and then recombine
    // tbe components into a 24-bit color value.
    uint8_t red, green, blue;
    senseColor(red, green, blue);
    return ((uint32_t)red << 16) | ((uint32_t)green << 8) | blue;
  }

  bool isExpress(void);

 private:


};


extern Bluebird_CircuitPlayground bluebirdCircuitPlayground; ///< instantiated by default

#endif
