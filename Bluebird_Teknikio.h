/*!
 * This is adapted by Teknikio from  Adafruit's CircuitPlayground driver for the Arduino platform.  It is
 * designed specifically to work with the Teknikio Bluebird boards.
 *
 * Originally written by Ladyada and others for Adafruit Industries.
 *
 * BSD license, all text here must be included in any redistribution.
 *
 */

#ifndef _BLUEBIRD_TEKNIKIO_H_
#define _BLUEBIRD_TEKNIKIO_H_

#include <Arduino.h>
#include <Servo.h>
#include "utility/Bluebird_NeoPixel.h"
#include "utility/TK_Firmata.h"
#include "utility/ICM20600.h"
#include "Servo.h"



#ifndef NOT_AN_INTERRUPT // Not defined in Arduino 1.0.5
  #define NOT_AN_INTERRUPT -1 ///< Pin is not on an interrupt
#endif

#define BLUEBIRD_NEOPIXELPIN  PIN_NEOPIXEL ///< neopixel pin

#if defined(_VARIANT_BLUEBIRD_)
  #define BLUEBIRD_BUZZER       PIN_BUZZER ///< buzzer pin
  #define BLUEBIRD_LIGHTSENSOR  A4 ///< light sensor pin  
  #define BLUEBIRD_COLOR_ENABLE PIN_COLOR_ENABLE
  #define BLUEBIRD_ICM_ADDRESS  false
#endif


#define BLUEBIRD_LED_1 	26
#define BLUEBIRD_LED_2 	5
#define BLUEBIRD_LED_3 	4
#define BLUEBIRD_LED_4 	6
#define BLUEBIRD_LED_5 	41
#define BLUEBIRD_LED_6 	12
#define ROW_SIZE		5
#define COLUMN_SIZE		6


#define BLUEBIRD_WIRE_INT     PIN_WIRE_INT


#define BLUEBIRD_MIN_CALIB 1
#define BLUEBIRD_MAX_CALIB 2
#define BLUEBIRD_CALIB_SAMPLES 10



/********************************************************************
 * Musical Notes via https://www.arduino.cc/en/Tutorial/ToneMelody  *
 ********************************************************************/

#define NOTE_B0  31
#define NOTE_C1  33
#define NOTE_CS1 35
#define NOTE_D1  37
#define NOTE_DS1 39
#define NOTE_E1  41
#define NOTE_F1  44
#define NOTE_FS1 46
#define NOTE_G1  49
#define NOTE_GS1 52
#define NOTE_A1  55
#define NOTE_AS1 58
#define NOTE_B1  62
#define NOTE_C2  65
#define NOTE_CS2 69
#define NOTE_D2  73
#define NOTE_DS2 78
#define NOTE_E2  82
#define NOTE_F2  87
#define NOTE_FS2 93
#define NOTE_G2  98
#define NOTE_GS2 104
#define NOTE_A2  110
#define NOTE_AS2 117
#define NOTE_B2  123
#define NOTE_C3  131
#define NOTE_CS3 139
#define NOTE_D3  147
#define NOTE_DS3 156
#define NOTE_E3  165
#define NOTE_F3  175
#define NOTE_FS3 185
#define NOTE_G3  196
#define NOTE_GS3 208
#define NOTE_A3  220
#define NOTE_AS3 233
#define NOTE_B3  247
#define NOTE_C4  262
#define NOTE_CS4 277
#define NOTE_D4  294
#define NOTE_DS4 311
#define NOTE_E4  330
#define NOTE_F4  349
#define NOTE_FS4 370
#define NOTE_G4  392
#define NOTE_GS4 415
#define NOTE_A4  440
#define NOTE_AS4 466
#define NOTE_B4  494
#define NOTE_C5  523
#define NOTE_CS5 554
#define NOTE_D5  587
#define NOTE_DS5 622
#define NOTE_E5  659
#define NOTE_F5  698
#define NOTE_FS5 740
#define NOTE_G5  784
#define NOTE_GS5 831
#define NOTE_A5  880
#define NOTE_AS5 932
#define NOTE_B5  988
#define NOTE_C6  1047
#define NOTE_CS6 1109
#define NOTE_D6  1175
#define NOTE_DS6 1245
#define NOTE_E6  1319
#define NOTE_F6  1397
#define NOTE_FS6 1480
#define NOTE_G6  1568
#define NOTE_GS6 1661
#define NOTE_A6  1760
#define NOTE_AS6 1865
#define NOTE_B6  1976
#define NOTE_C7  2093
#define NOTE_CS7 2217
#define NOTE_D7  2349
#define NOTE_DS7 2489
#define NOTE_E7  2637
#define NOTE_F7  2794
#define NOTE_FS7 2960
#define NOTE_G7  3136
#define NOTE_GS7 3322
#define NOTE_A7  3520
#define NOTE_AS7 3729
#define NOTE_B7  3951
#define NOTE_C8  4186
#define NOTE_CS8 4435
#define NOTE_D8  4699
#define NOTE_DS8 4978

/*! 
  @brief Configuration to tune the color sensing logic:
   Amount of time (in milliseconds) to wait between 
   changing the pixel color and reading the light
    sensor.
*/
#define LIGHT_SETTLE_MS 100  



/**************************************************************************/
/*! 
    @brief  Class that stores state and functions for interacting with Bluebird hardware
*/
/**************************************************************************/
class Bluebird {
public:
	bool begin(uint8_t brightness=20);

  Bluebird_NeoPixel strip; ///< the neopixel strip object
  ICM20600 icm20600;
  Servo servo1;
  Servo servo2;

  uint8_t min_red;
  uint8_t min_green;
  uint8_t min_blue;

  uint8_t max_red;
  uint8_t max_green;
  uint8_t max_blue;

  uint8_t row_index;
  uint8_t col_index;
  uint8_t ledpattern[ROW_SIZE][COLUMN_SIZE] = {0};


  void playTone(uint16_t freq, uint16_t time, bool wait=true);
  uint16_t lightSensor(void);

    // Accelerometer
  int16_t motionX(void);
  int16_t motionY(void);
  int16_t motionZ(void);

  int16_t rotationX(void);
  int16_t rotationY(void);
  int16_t rotationZ(void);

  int16_t getTemperature(void);


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
  void setBrightness(uint8_t b);
  // Basic RGB color sensing with the light sensor and nearby neopixel.
  // Both functions do the same thing and just differ in how they return the
  // result, either as explicit RGB bytes or a 24-bit RGB color value.
  void senseColor(uint8_t& red, uint8_t& green, uint8_t& blue);

  // Basic RGB color sensing with the light sensor and nearby neopixel.
  // Both functions do the same thing and just differ in how they return the
  // result, either as explicit RGB bytes or a 24-bit RGB color value.
  void calibratesenseColor(uint8_t mode,uint8_t& red, uint8_t& green, uint8_t& blue);


  void resetLedMatrix(void);

  void setLedMatrix(int row,int column);

  void startTimer(void);

  void matrixHandler(void);

  void changepattern(void *m_ledpattern);
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
private:


};


extern Bluebird bluebird; ///< instantiated by default

#endif
