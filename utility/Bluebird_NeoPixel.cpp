/*!
 * Arduino Library for driving Adafruit NeoPixel addressable LEDs,
 * FLORA RGB Smart Pixels and compatible devicess -- WS2811, WS2812, WS2812B,
 * SK6812, etc.

 * This is the documentation for Adafruit's NeoPixel library for the
 * Arduino platform, allowing a broad range of microcontroller boards
 * (most AVR boards, many ARM devices, ESP8266 and ESP32, among others)
 * to control Adafruit NeoPixels, FLORA RGB Smart Pixels and compatible
 * devices -- WS2811, WS2812, WS2812B, SK6812, etc.
 *
 * Adafruit invests time and resources providing this open source code,
 * please support Adafruit and open-source hardware by purchasing products
 * from Adafruit!
 *
 * Written by Phil "Paint Your Dragon" Burgess for Adafruit Industries,
 * with contributions by PJRC, Michael Miller and other members of the
 * open source community.
 * This file is part of the Adafruit_NeoPixel library.
 *
 * Adafruit_NeoPixel is free software: you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public License as
 * published by the Free Software Foundation, either version 3 of the
 * License, or (at your option) any later version.
 *
 * Adafruit_NeoPixel is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with NeoPixel. If not, see
 * <http://www.gnu.org/licenses/>.
 *
 */

#include "Bluebird_NeoPixel.h"

#include "nrf.h"

// Interrupt is only disabled if there is no PWM device available
// Note: Adafruit Bluefruit nrf52 does not use this option
//#define NRF52_DISABLE_INT

/*!
  @brief   NeoPixel constructor when length, pin and pixel type are known
           at compile-time.
  @param   n  Number of NeoPixels in strand.
  @param   p  Arduino pin number which will drive the NeoPixel data in.
  @param   t  Pixel type -- add together NEO_* constants defined in
              Bluebird_NeoPixel.h, for example NEO_GRB+NEO_KHZ800 for
              NeoPixels expecting an 800 KHz (vs 400 KHz) data stream
              with color bytes expressed in green, red, blue order per
              pixel.
*/
Bluebird_NeoPixel::Bluebird_NeoPixel(uint16_t n, uint16_t p, neoPixelType t) :
  begun(false), brightness(0), pixels(NULL), endTime(0) {
  updateType(t);
  updateLength(n);
  setPin(p);
}

/*!
  @brief   "Empty" NeoPixel constructor when length, pin and/or pixel type
           are not known at compile-time, and must be initialized later with
           updateType(), updateLength() and setPin().
  @note    This function is deprecated, here only for old projects that
           may still be calling it. New projects should instead use the
           'new' keyword with the first constructor syntax (length, pin,
           type).
*/
Bluebird_NeoPixel::Bluebird_NeoPixel() :
  is800KHz(true),
  begun(false), numLEDs(0), numBytes(0), pin(-1), brightness(0), pixels(NULL),
  rOffset(1), gOffset(0), bOffset(2), wOffset(1), endTime(0) {
}

/*!
  @brief   Deallocate Bluebird_NeoPixel object, set data pin back to INPUT.
*/
Bluebird_NeoPixel::~Bluebird_NeoPixel() {
  free(pixels);
  if(pin >= 0) pinMode(pin, INPUT);
}

/*!
  @brief   Configure NeoPixel pin for output.
*/
void Bluebird_NeoPixel::begin(void) {
  if(pin >= 0) {
    pinMode(pin, OUTPUT);
    digitalWrite(pin, LOW);
  }
  begun = true;
}

/*!
  @brief   Change the length of a previously-declared Bluebird_NeoPixel
           strip object. Old data is deallocated and new data is cleared.
           Pin number and pixel format are unchanged.
  @param   n  New length of strip, in pixels.
  @note    This function is deprecated, here only for old projects that
           may still be calling it. New projects should instead use the
           'new' keyword with the first constructor syntax (length, pin,
           type).
*/
void Bluebird_NeoPixel::updateLength(uint16_t n) {
  free(pixels); // Free existing data (if any)

  // Allocate new data -- note: ALL PIXELS ARE CLEARED
  numBytes = n * ((wOffset == rOffset) ? 3 : 4);
  if((pixels = (uint8_t *)malloc(numBytes))) {
    memset(pixels, 0, numBytes);
    numLEDs = n;
  } else {
    numLEDs = numBytes = 0;
  }
}

/*!
  @brief   Change the pixel format of a previously-declared
           Bluebird_NeoPixel strip object. If format changes from one of
           the RGB variants to an RGBW variant (or RGBW to RGB), the old
           data will be deallocated and new data is cleared. Otherwise,
           the old data will remain in RAM and is not reordered to the
           new format, so it's advisable to follow up with clear().
  @param   t  Pixel type -- add together NEO_* constants defined in
              Bluebird_NeoPixel.h, for example NEO_GRB+NEO_KHZ800 for
              NeoPixels expecting an 800 KHz (vs 400 KHz) data stream
              with color bytes expressed in green, red, blue order per
              pixel.
  @note    This function is deprecated, here only for old projects that
           may still be calling it. New projects should instead use the
           'new' keyword with the first constructor syntax
           (length, pin, type).
*/
void Bluebird_NeoPixel::updateType(neoPixelType t) {
  boolean oldThreeBytesPerPixel = (wOffset == rOffset); // false if RGBW

  wOffset = (t >> 6) & 0b11; // See notes in header file
  rOffset = (t >> 4) & 0b11; // regarding R/G/B/W offsets
  gOffset = (t >> 2) & 0b11;
  bOffset =  t       & 0b11;
#ifdef NEO_KHZ400
  is800KHz = (t < 256);      // 400 KHz flag is 1<<8
#endif

  // If bytes-per-pixel has changed (and pixel data was previously
  // allocated), re-allocate to new size. Will clear any data.
  if(pixels) {
    boolean newThreeBytesPerPixel = (wOffset == rOffset);
    if(newThreeBytesPerPixel != oldThreeBytesPerPixel) updateLength(numLEDs);
  }
}


/*!
  @brief   Transmit pixel data in RAM to NeoPixels.
  @note    On most architectures, interrupts are temporarily disabled in
           order to achieve the correct NeoPixel signal timing. This means
           that the Arduino millis() and micros() functions, which require
           interrupts, will lose small intervals of time whenever this
           function is called (about 30 microseconds per RGB pixel, 40 for
           RGBW pixels). There's no easy fix for this, but a few
           specialized alternative or companion libraries exist that use
           very device-specific peripherals to work around it.
*/
void Bluebird_NeoPixel::show(void) {

	if(!pixels) return;

  // Data latch = 300+ microsecond pause in the output stream. Rather than
  // put a delay at the end of the function, the ending time is noted and
  // the function will simply hold off (if needed) on issuing the
  // subsequent round of data until the latch time has elapsed. This
  // allows the mainline code to start generating the next frame of data
  // rather than stalling for the latch.
	while(!canShow());
  // endTime is a private member (rather than global var) so that multiple
  // instances on different pins can be quickly issued in succession (each
  // instance doesn't delay the next).

  // In order to make this code runtime-configurable to work with any pin,
  // SBI/CBI instructions are eschewed in favor of full PORT writes via the
  // OUT or ST instructions. It relies on two facts: that peripheral
  // functions (such as PWM) take precedence on output pins, so our PORT-
  // wide writes won't interfere, and that interrupts are globally disabled
  // while data is being issued to the LEDs, so no other code will be
  // accessing the PORT. The code takes an initial 'snapshot' of the PORT
  // state, computes 'pin high' and 'pin low' values, and writes these back
  // to the PORT register as needed.

  // NRF52 may use PWM + DMA (if available), may not need to disable interrupt
#if !( defined(NRF52) || defined(NRF52_SERIES) )
  noInterrupts(); // Need 100% focus on instruction timing
#endif



// Begin of support for nRF52 based boards  -------------------------

#if defined(NRF52) || defined(NRF52_SERIES)
// [[[Begin of the Neopixel NRF52 EasyDMA implementation
//                                    by the Hackerspace San Salvador]]]
// This technique uses the PWM peripheral on the NRF52. The PWM uses the
// EasyDMA feature included on the chip. This technique loads the duty
// cycle configuration for each cycle when the PWM is enabled. For this
// to work we need to store a 16 bit configuration for each bit of the
// RGB(W) values in the pixel buffer.
// Comparator values for the PWM were hand picked and are guaranteed to
// be 100% organic to preserve freshness and high accuracy. Current
// parameters are:
//   * PWM Clock: 16Mhz
//   * Minimum step time: 62.5ns
//   * Time for zero in high (T0H): 0.31ms
//   * Time for one in high (T1H): 0.75ms
//   * Cycle time:  1.25us
//   * Frequency: 800Khz
// For 400Khz we just double the calculated times.
// ---------- BEGIN Constants for the EasyDMA implementation -----------
// The PWM starts the duty cycle in LOW. To start with HIGH we
// need to set the 15th bit on each register.

// WS2812 (rev A) timing is 0.35 and 0.7us
//#define MAGIC_T0H               5UL | (0x8000) // 0.3125us
//#define MAGIC_T1H              12UL | (0x8000) // 0.75us

// WS2812B (rev B) timing is 0.4 and 0.8 us
#define MAGIC_T0H               6UL | (0x8000) // 0.375us
#define MAGIC_T1H              13UL | (0x8000) // 0.8125us

// WS2811 (400 khz) timing is 0.5 and 1.2
#define MAGIC_T0H_400KHz        8UL  | (0x8000) // 0.5us
#define MAGIC_T1H_400KHz        19UL | (0x8000) // 1.1875us

// For 400Khz, we double value of CTOPVAL
#define CTOPVAL                20UL            // 1.25us
#define CTOPVAL_400KHz         40UL            // 2.5us

// ---------- END Constants for the EasyDMA implementation -------------
//
// If there is no device available an alternative cycle-counter
// implementation is tried.
// The nRF52 runs with a fixed clock of 64Mhz. The alternative
// implementation is the same as the one used for the Teensy 3.0/1/2 but
// with the Nordic SDK HAL & registers syntax.
// The number of cycles was hand picked and is guaranteed to be 100%
// organic to preserve freshness and high accuracy.
// ---------- BEGIN Constants for cycle counter implementation ---------
#define CYCLES_800_T0H  18  // ~0.36 uS
#define CYCLES_800_T1H  41  // ~0.76 uS
#define CYCLES_800      71  // ~1.25 uS

#define CYCLES_400_T0H  26  // ~0.50 uS
#define CYCLES_400_T1H  70  // ~1.26 uS
#define CYCLES_400      156 // ~2.50 uS
// ---------- END of Constants for cycle counter implementation --------

  // To support both the SoftDevice + Neopixels we use the EasyDMA
  // feature from the NRF25. However this technique implies to
  // generate a pattern and store it on the memory. The actual
  // memory used in bytes corresponds to the following formula:
  //              totalMem = numBytes*8*2+(2*2)
  // The two additional bytes at the end are needed to reset the
  // sequence.
  //
  // If there is not enough memory, we will fall back to cycle counter
  // using DWT
  uint32_t  pattern_size   = numBytes*8*sizeof(uint16_t)+2*sizeof(uint16_t);
  uint16_t* pixels_pattern = NULL;

  NRF_PWM_Type* pwm = NULL;

  // Try to find a free PWM device, which is not enabled
  // and has no connected pins
  NRF_PWM_Type* PWM[] = {
  	NRF_PWM0, NRF_PWM1, NRF_PWM2
#ifdef NRF_PWM3
  	,NRF_PWM3
#endif
  };

  for(int device = 0; device < (sizeof(PWM)/sizeof(PWM[0])); device++) {
  	if( (PWM[device]->ENABLE == 0)                            &&
  		(PWM[device]->PSEL.OUT[0] & PWM_PSEL_OUT_CONNECT_Msk) &&
  		(PWM[device]->PSEL.OUT[1] & PWM_PSEL_OUT_CONNECT_Msk) &&
  		(PWM[device]->PSEL.OUT[2] & PWM_PSEL_OUT_CONNECT_Msk) &&
  		(PWM[device]->PSEL.OUT[3] & PWM_PSEL_OUT_CONNECT_Msk)
  		) {
  		pwm = PWM[device];
  	break;
  }
}

  // only malloc if there is PWM device available
if ( pwm != NULL ) {
    #ifdef ARDUINO_NRF52_ADAFRUIT // use thread-safe malloc
	pixels_pattern = (uint16_t *) rtos_malloc(pattern_size);
    #else
	pixels_pattern = (uint16_t *) malloc(pattern_size);
    #endif
}

  // Use the identified device to choose the implementation
  // If a PWM device is available use DMA
if( (pixels_pattern != NULL) && (pwm != NULL) ) {
    uint16_t pos = 0; // bit position

    for(uint16_t n=0; n<numBytes; n++) {
    	uint8_t pix = pixels[n];

    	for(uint8_t mask=0x80; mask>0; mask >>= 1) {
        #ifdef NEO_KHZ400
    		if( !is800KHz ) {
    			pixels_pattern[pos] = (pix & mask) ? MAGIC_T1H_400KHz : MAGIC_T0H_400KHz;
    		}else
        #endif
    		{
    			pixels_pattern[pos] = (pix & mask) ? MAGIC_T1H : MAGIC_T0H;
    		}

    		pos++;
    	}
    }

    // Zero padding to indicate the end of que sequence
    pixels_pattern[pos++] = 0 | (0x8000); // Seq end
    pixels_pattern[pos++] = 0 | (0x8000); // Seq end

    // Set the wave mode to count UP
    pwm->MODE = (PWM_MODE_UPDOWN_Up << PWM_MODE_UPDOWN_Pos);

    // Set the PWM to use the 16MHz clock
    pwm->PRESCALER = (PWM_PRESCALER_PRESCALER_DIV_1 << PWM_PRESCALER_PRESCALER_Pos);

    // Setting of the maximum count
    // but keeping it on 16Mhz allows for more granularity just
    // in case someone wants to do more fine-tuning of the timing.
#ifdef NEO_KHZ400
    if( !is800KHz ) {
    	pwm->COUNTERTOP = (CTOPVAL_400KHz << PWM_COUNTERTOP_COUNTERTOP_Pos);
    }else
#endif
    {
    	pwm->COUNTERTOP = (CTOPVAL << PWM_COUNTERTOP_COUNTERTOP_Pos);
    }

    // Disable loops, we want the sequence to repeat only once
    pwm->LOOP = (PWM_LOOP_CNT_Disabled << PWM_LOOP_CNT_Pos);

    // On the "Common" setting the PWM uses the same pattern for the
    // for supported sequences. The pattern is stored on half-word
    // of 16bits
    pwm->DECODER = (PWM_DECODER_LOAD_Common << PWM_DECODER_LOAD_Pos) |
    (PWM_DECODER_MODE_RefreshCount << PWM_DECODER_MODE_Pos);

    // Pointer to the memory storing the patter
    pwm->SEQ[0].PTR = (uint32_t)(pixels_pattern) << PWM_SEQ_PTR_PTR_Pos;

    // Calculation of the number of steps loaded from memory.
    pwm->SEQ[0].CNT = (pattern_size/sizeof(uint16_t)) << PWM_SEQ_CNT_CNT_Pos;

    // The following settings are ignored with the current config.
    pwm->SEQ[0].REFRESH  = 0;
    pwm->SEQ[0].ENDDELAY = 0;

    // The Neopixel implementation is a blocking algorithm. DMA
    // allows for non-blocking operation. To "simulate" a blocking
    // operation we enable the interruption for the end of sequence
    // and block the execution thread until the event flag is set by
    // the peripheral.
//    pwm->INTEN |= (PWM_INTEN_SEQEND0_Enabled<<PWM_INTEN_SEQEND0_Pos);

    // PSEL must be configured before enabling PWM
    pwm->PSEL.OUT[0] = g_ADigitalPinMap[pin];

    // Enable the PWM
    pwm->ENABLE = 1;

    // After all of this and many hours of reading the documentation
    // we are ready to start the sequence...
    pwm->EVENTS_SEQEND[0]  = 0;
    pwm->TASKS_SEQSTART[0] = 1;

    // But we have to wait for the flag to be set.
    while(!pwm->EVENTS_SEQEND[0])
    {
      #ifdef ARDUINO_NRF52_ADAFRUIT
    	yield();
      #endif
    }

    // Before leave we clear the flag for the event.
    pwm->EVENTS_SEQEND[0] = 0;

    // We need to disable the device and disconnect
    // all the outputs before leave or the device will not
    // be selected on the next call.
    // TODO: Check if disabling the device causes performance issues.
    pwm->ENABLE = 0;

    pwm->PSEL.OUT[0] = 0xFFFFFFFFUL;

    #ifdef ARDUINO_NRF52_ADAFRUIT  // use thread-safe free
    rtos_free(pixels_pattern);
    #else
    free(pixels_pattern);
    #endif
  }// End of DMA implementation
  // ---------------------------------------------------------------------
  else{
    // Fall back to DWT
    #ifdef ARDUINO_NRF52_ADAFRUIT
      // Bluefruit Feather 52 uses freeRTOS
      // Critical Section is used since it does not block SoftDevice execution
  	taskENTER_CRITICAL();
    #elif defined(NRF52_DISABLE_INT)
      // If you are using the Bluetooth SoftDevice we advise you to not disable
      // the interrupts. Disabling the interrupts even for short periods of time
      // causes the SoftDevice to stop working.
      // Disable the interrupts only in cases where you need high performance for
      // the LEDs and if you are not using the EasyDMA feature.
  	__disable_irq();
    #endif

  	NRF_GPIO_Type* nrf_port = (NRF_GPIO_Type*) digitalPinToPort(pin);
  	uint32_t pinMask = digitalPinToBitMask(pin);

  	uint32_t CYCLES_X00     = CYCLES_800;
  	uint32_t CYCLES_X00_T1H = CYCLES_800_T1H;
  	uint32_t CYCLES_X00_T0H = CYCLES_800_T0H;

#ifdef NEO_KHZ400
  	if( !is800KHz )
  	{
  		CYCLES_X00     = CYCLES_400;
  		CYCLES_X00_T1H = CYCLES_400_T1H;
  		CYCLES_X00_T0H = CYCLES_400_T0H;
  	}
#endif

    // Enable DWT in debug core
  	CoreDebug->DEMCR |= CoreDebug_DEMCR_TRCENA_Msk;
  	DWT->CTRL |= DWT_CTRL_CYCCNTENA_Msk;

    // Tries to re-send the frame if is interrupted by the SoftDevice.
  	while(1) {
  		uint8_t *p = pixels;

  		uint32_t cycStart = DWT->CYCCNT;
  		uint32_t cyc = 0;

  		for(uint16_t n=0; n<numBytes; n++) {
  			uint8_t pix = *p++;

  			for(uint8_t mask = 0x80; mask; mask >>= 1) {
  				while(DWT->CYCCNT - cyc < CYCLES_X00);
  				cyc  = DWT->CYCCNT;

  				nrf_port->OUTSET |= pinMask;

  				if(pix & mask) {
  					while(DWT->CYCCNT - cyc < CYCLES_X00_T1H);
  				} else {
  					while(DWT->CYCCNT - cyc < CYCLES_X00_T0H);
  				}

  				nrf_port->OUTCLR |= pinMask;
  			}
  		}
  		while(DWT->CYCCNT - cyc < CYCLES_X00);


      // If total time longer than 25%, resend the whole data.
      // Since we are likely to be interrupted by SoftDevice
  		if ( (DWT->CYCCNT - cycStart) < ( 8*numBytes*((CYCLES_X00*5)/4) ) ) {
  			break;
  		}

      // re-send need 300us delay
  		delayMicroseconds(300);
  	}

    // Enable interrupts again
    #ifdef ARDUINO_NRF52_ADAFRUIT
  	taskEXIT_CRITICAL();
    #elif defined(NRF52_DISABLE_INT)
  	__enable_irq();
    #endif
  }
// END of NRF52 implementation


#else
#error Architecture not supported
#endif


// END ARCHITECTURE SELECT ------------------------------------------------

#if !( defined(NRF52) || defined(NRF52_SERIES) )
  interrupts();
#endif

  endTime = micros(); // Save EOD time for latch on next call
}

/*!
  @brief   Set/change the NeoPixel output pin number. Previous pin,
           if any, is set to INPUT and the new pin is set to OUTPUT.
  @param   p  Arduino pin number (-1 = no pin).
*/
void Bluebird_NeoPixel::setPin(uint16_t p) {
	if(begun && (pin >= 0)) pinMode(pin, INPUT);
	pin = p;
	if(begun) {
		pinMode(p, OUTPUT);
		digitalWrite(p, LOW);
	}
}

/*!
  @brief   Set a pixel's color using separate red, green and blue
           components. If using RGBW pixels, white will be set to 0.
  @param   n  Pixel index, starting from 0.
  @param   r  Red brightness, 0 = minimum (off), 255 = maximum.
  @param   g  Green brightness, 0 = minimum (off), 255 = maximum.
  @param   b  Blue brightness, 0 = minimum (off), 255 = maximum.
*/
void Bluebird_NeoPixel::setPixelColor(
 uint16_t n, uint8_t r, uint8_t g, uint8_t b) {

  if(n < numLEDs) {
    if(brightness) { // See notes in setBrightness()
      r = (r * brightness) >> 8;
      g = (g * brightness) >> 8;
      b = (b * brightness) >> 8;
    }
    uint8_t *p;
    if(wOffset == rOffset) { // Is an RGB-type strip
      p = &pixels[n * 3];    // 3 bytes per pixel
    } else {                 // Is a WRGB-type strip
      p = &pixels[n * 4];    // 4 bytes per pixel
      p[wOffset] = 0;        // But only R,G,B passed -- set W to 0
    }
    p[rOffset] = r;          // R,G,B always stored
    p[gOffset] = g;
    p[bOffset] = b;
  }
}

/*!
  @brief   Set a pixel's color using separate red, green, blue and white
           components (for RGBW NeoPixels only).
  @param   n  Pixel index, starting from 0.
  @param   r  Red brightness, 0 = minimum (off), 255 = maximum.
  @param   g  Green brightness, 0 = minimum (off), 255 = maximum.
  @param   b  Blue brightness, 0 = minimum (off), 255 = maximum.
  @param   w  White brightness, 0 = minimum (off), 255 = maximum, ignored
              if using RGB pixels.
*/
void Bluebird_NeoPixel::setPixelColor(
 uint16_t n, uint8_t r, uint8_t g, uint8_t b, uint8_t w) {

  if(n < numLEDs) {
    if(brightness) { // See notes in setBrightness()
      r = (r * brightness) >> 8;
      g = (g * brightness) >> 8;
      b = (b * brightness) >> 8;
      w = (w * brightness) >> 8;
    }
    uint8_t *p;
    if(wOffset == rOffset) { // Is an RGB-type strip
      p = &pixels[n * 3];    // 3 bytes per pixel (ignore W)
    } else {                 // Is a WRGB-type strip
      p = &pixels[n * 4];    // 4 bytes per pixel
      p[wOffset] = w;        // Store W
    }
    p[rOffset] = r;          // Store R,G,B
    p[gOffset] = g;
    p[bOffset] = b;
  }
}

/*!
  @brief   Set a pixel's color using a 32-bit 'packed' RGB or RGBW value.
  @param   n  Pixel index, starting from 0.
  @param   c  32-bit color value. Most significant byte is white (for RGBW
              pixels) or ignored (for RGB pixels), next is red, then green,
              and least significant byte is blue.
*/
void Bluebird_NeoPixel::setPixelColor(uint16_t n, uint32_t c) {
  if(n < numLEDs) {
    uint8_t *p,
      r = (uint8_t)(c >> 16),
      g = (uint8_t)(c >>  8),
      b = (uint8_t)c;
    if(brightness) { // See notes in setBrightness()
      r = (r * brightness) >> 8;
      g = (g * brightness) >> 8;
      b = (b * brightness) >> 8;
    }
    if(wOffset == rOffset) {
      p = &pixels[n * 3];
    } else {
      p = &pixels[n * 4];
      uint8_t w = (uint8_t)(c >> 24);
      p[wOffset] = brightness ? ((w * brightness) >> 8) : w;
    }
    p[rOffset] = r;
    p[gOffset] = g;
    p[bOffset] = b;
  }
}

/*!
  @brief   Fill all or part of the NeoPixel strip with a color.
  @param   c      32-bit color value. Most significant byte is white (for
                  RGBW pixels) or ignored (for RGB pixels), next is red,
                  then green, and least significant byte is blue. If all
                  arguments are unspecified, this will be 0 (off).
  @param   first  Index of first pixel to fill, starting from 0. Must be
                  in-bounds, no clipping is performed. 0 if unspecified.
  @param   count  Number of pixels to fill, as a positive value. Passing
                  0 or leaving unspecified will fill to end of strip.
*/
void Bluebird_NeoPixel::fill(uint32_t c, uint16_t first, uint16_t count) {
  uint16_t i, end;

  if(first >= numLEDs) {
    return; // If first LED is past end of strip, nothing to do
  }

  // Calculate the index ONE AFTER the last pixel to fill
  if(count == 0) {
    // Fill to end of strip
    end = numLEDs;
  } else {
    // Ensure that the loop won't go past the last pixel
    end = first + count;
    if(end > numLEDs) end = numLEDs;
  }

  for(i = first; i < end; i++) {
    this->setPixelColor(i, c);
  }
}

/*!
  @brief   Convert hue, saturation and value into a packed 32-bit RGB color
           that can be passed to setPixelColor() or other RGB-compatible
           functions.
  @param   hue  An unsigned 16-bit value, 0 to 65535, representing one full
                loop of the color wheel, which allows 16-bit hues to "roll
                over" while still doing the expected thing (and allowing
                more precision than the wheel() function that was common to
                prior NeoPixel examples).
  @param   sat  Saturation, 8-bit value, 0 (min or pure grayscale) to 255
                (max or pure hue). Default of 255 if unspecified.
  @param   val  Value (brightness), 8-bit value, 0 (min / black / off) to
                255 (max or full brightness). Default of 255 if unspecified.
  @return  Packed 32-bit RGB with the most significant byte set to 0 -- the
           white element of WRGB pixels is NOT utilized. Result is linearly
           but not perceptually correct, so you may want to pass the result
           through the gamma32() function (or your own gamma-correction
           operation) else colors may appear washed out. This is not done
           automatically by this function because coders may desire a more
           refined gamma-correction function than the simplified
           one-size-fits-all operation of gamma32(). Diffusing the LEDs also
           really seems to help when using low-saturation colors.
*/
uint32_t Bluebird_NeoPixel::ColorHSV(uint16_t hue, uint8_t sat, uint8_t val) {

  uint8_t r, g, b;

  // Remap 0-65535 to 0-1529. Pure red is CENTERED on the 64K rollover;
  // 0 is not the start of pure red, but the midpoint...a few values above
  // zero and a few below 65536 all yield pure red (similarly, 32768 is the
  // midpoint, not start, of pure cyan). The 8-bit RGB hexcone (256 values
  // each for red, green, blue) really only allows for 1530 distinct hues
  // (not 1536, more on that below), but the full unsigned 16-bit type was
  // chosen for hue so that one's code can easily handle a contiguous color
  // wheel by allowing hue to roll over in either direction.
  hue = (hue * 1530L + 32768) / 65536;
  // Because red is centered on the rollover point (the +32768 above,
  // essentially a fixed-point +0.5), the above actually yields 0 to 1530,
  // where 0 and 1530 would yield the same thing. Rather than apply a
  // costly modulo operator, 1530 is handled as a special case below.

  // So you'd think that the color "hexcone" (the thing that ramps from
  // pure red, to pure yellow, to pure green and so forth back to red,
  // yielding six slices), and with each color component having 256
  // possible values (0-255), might have 1536 possible items (6*256),
  // but in reality there's 1530. This is because the last element in
  // each 256-element slice is equal to the first element of the next
  // slice, and keeping those in there this would create small
  // discontinuities in the color wheel. So the last element of each
  // slice is dropped...we regard only elements 0-254, with item 255
  // being picked up as element 0 of the next slice. Like this:
  // Red to not-quite-pure-yellow is:        255,   0, 0 to 255, 254,   0
  // Pure yellow to not-quite-pure-green is: 255, 255, 0 to   1, 255,   0
  // Pure green to not-quite-pure-cyan is:     0, 255, 0 to   0, 255, 254
  // and so forth. Hence, 1530 distinct hues (0 to 1529), and hence why
  // the constants below are not the multiples of 256 you might expect.

  // Convert hue to R,G,B (nested ifs faster than divide+mod+switch):
  if(hue < 510) {         // Red to Green-1
    b = 0;
    if(hue < 255) {       //   Red to Yellow-1
      r = 255;
      g = hue;            //     g = 0 to 254
    } else {              //   Yellow to Green-1
      r = 510 - hue;      //     r = 255 to 1
      g = 255;
    }
  } else if(hue < 1020) { // Green to Blue-1
    r = 0;
    if(hue <  765) {      //   Green to Cyan-1
      g = 255;
      b = hue - 510;      //     b = 0 to 254
    } else {              //   Cyan to Blue-1
      g = 1020 - hue;     //     g = 255 to 1
      b = 255;
    }
  } else if(hue < 1530) { // Blue to Red-1
    g = 0;
    if(hue < 1275) {      //   Blue to Magenta-1
      r = hue - 1020;     //     r = 0 to 254
      b = 255;
    } else {              //   Magenta to Red-1
      r = 255;
      b = 1530 - hue;     //     b = 255 to 1
    }
  } else {                // Last 0.5 Red (quicker than % operator)
    r = 255;
    g = b = 0;
  }

  // Apply saturation and value to R,G,B, pack into 32-bit result:
  uint32_t v1 =   1 + val; // 1 to 256; allows >>8 instead of /255
  uint16_t s1 =   1 + sat; // 1 to 256; same reason
  uint8_t  s2 = 255 - sat; // 255 to 0
  return ((((((r * s1) >> 8) + s2) * v1) & 0xff00) << 8) |
          (((((g * s1) >> 8) + s2) * v1) & 0xff00)       |
         ( ((((b * s1) >> 8) + s2) * v1)           >> 8);
}

/*!
  @brief   Query the color of a previously-set pixel.
  @param   n  Index of pixel to read (0 = first).
  @return  'Packed' 32-bit RGB or WRGB value. Most significant byte is white
           (for RGBW pixels) or 0 (for RGB pixels), next is red, then green,
           and least significant byte is blue.
  @note    If the strip brightness has been changed from the default value
           of 255, the color read from a pixel may not exactly match what
           was previously written with one of the setPixelColor() functions.
           This gets more pronounced at lower brightness levels.
*/
uint32_t Bluebird_NeoPixel::getPixelColor(uint16_t n) const {
  if(n >= numLEDs) return 0; // Out of bounds, return no color.

  uint8_t *p;

  if(wOffset == rOffset) { // Is RGB-type device
    p = &pixels[n * 3];
    if(brightness) {
      // Stored color was decimated by setBrightness(). Returned value
      // attempts to scale back to an approximation of the original 24-bit
      // value used when setting the pixel color, but there will always be
      // some error -- those bits are simply gone. Issue is most
      // pronounced at low brightness levels.
      return (((uint32_t)(p[rOffset] << 8) / brightness) << 16) |
             (((uint32_t)(p[gOffset] << 8) / brightness) <<  8) |
             ( (uint32_t)(p[bOffset] << 8) / brightness       );
    } else {
      // No brightness adjustment has been made -- return 'raw' color
      return ((uint32_t)p[rOffset] << 16) |
             ((uint32_t)p[gOffset] <<  8) |
              (uint32_t)p[bOffset];
    }
  } else {                 // Is RGBW-type device
    p = &pixels[n * 4];
    if(brightness) { // Return scaled color
      return (((uint32_t)(p[wOffset] << 8) / brightness) << 24) |
             (((uint32_t)(p[rOffset] << 8) / brightness) << 16) |
             (((uint32_t)(p[gOffset] << 8) / brightness) <<  8) |
             ( (uint32_t)(p[bOffset] << 8) / brightness       );
    } else { // Return raw color
      return ((uint32_t)p[wOffset] << 24) |
             ((uint32_t)p[rOffset] << 16) |
             ((uint32_t)p[gOffset] <<  8) |
              (uint32_t)p[bOffset];
    }
  }
}


/*!
  @brief   Adjust output brightness. Does not immediately affect what's
           currently displayed on the LEDs. The next call to show() will
           refresh the LEDs at this level.
  @param   b  Brightness setting, 0=minimum (off), 255=brightest.
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
void Bluebird_NeoPixel::setBrightness(uint8_t b) {
  // Stored brightness value is different than what's passed.
  // This simplifies the actual scaling math later, allowing a fast
  // 8x8-bit multiply and taking the MSB. 'brightness' is a uint8_t,
  // adding 1 here may (intentionally) roll over...so 0 = max brightness
  // (color values are interpreted literally; no scaling), 1 = min
  // brightness (off), 255 = just below max brightness.
  uint8_t newBrightness = b + 1;
  if(newBrightness != brightness) { // Compare against prior value
    // Brightness has changed -- re-scale existing data in RAM,
    // This process is potentially "lossy," especially when increasing
    // brightness. The tight timing in the WS2811/WS2812 code means there
    // aren't enough free cycles to perform this scaling on the fly as data
    // is issued. So we make a pass through the existing color data in RAM
    // and scale it (subsequent graphics commands also work at this
    // brightness level). If there's a significant step up in brightness,
    // the limited number of steps (quantization) in the old data will be
    // quite visible in the re-scaled version. For a non-destructive
    // change, you'll need to re-render the full strip data. C'est la vie.
    uint8_t  c,
            *ptr           = pixels,
             oldBrightness = brightness - 1; // De-wrap old brightness value
    uint16_t scale;
    if(oldBrightness == 0) scale = 0; // Avoid /0
    else if(b == 255) scale = 65535 / oldBrightness;
    else scale = (((uint16_t)newBrightness << 8) - 1) / oldBrightness;
    for(uint16_t i=0; i<numBytes; i++) {
      c      = *ptr;
      *ptr++ = (c * scale) >> 8;
    }
    brightness = newBrightness;
  }
}

/*!
  @brief   Retrieve the last-set brightness value for the strip.
  @return  Brightness value: 0 = minimum (off), 255 = maximum.
*/
uint8_t Bluebird_NeoPixel::getBrightness(void) const {
  return brightness - 1;
}

/*!
  @brief   Fill the whole NeoPixel strip with 0 / black / off.
*/
void Bluebird_NeoPixel::clear(void) {
  memset(pixels, 0, numBytes);
}