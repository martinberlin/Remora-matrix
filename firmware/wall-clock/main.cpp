#include <SPI.h>
#include <Wire.h> 
// Super proud of amazing Marc: https://github.com/marcmerlin All credits for this go to him!
#include <Adafruit_GFX.h>
// FONT for the clock
#include <Fonts/Ubuntu_M8pt8b.h>
GFXfont Font1 = Ubuntu_M8pt8b;
#include <FastLED_NeoMatrix.h>
#include <FastLED.h>
#include <LEDMatrix.h>

#include "WiFi.h"
#include "freertos/FreeRTOS.h"
#include "freertos/timers.h"
// Set initial RTC data
uint8_t hr = 12;
uint8_t m = 19;
int year = 2024;
int month = 8;
int day = 31;
// BitBank RTC magic
#include <bb_rtc.h>
BBRTC rtc;
#define SET_CLOCK false
// Define your Matrix following Adafruit_NeoMatrix Guide: https://learn.adafruit.com/adafruit-neopixel-uberguide/neomatrix-library
// cLEDMatrix defines 
cLEDMatrix<-MATRIX_WIDTH, -10, HORIZONTAL_ZIGZAG_MATRIX,
    1, 2, HORIZONTAL_BLOCKS> ledmatrix;
// cLEDMatrix creates a FastLED array inside its object and we need to retrieve
// a pointer to its first element to act as a regular FastLED array, necessary
// for NeoMatrix and other operations that may work directly on the array like FadeAll.
CRGB *leds = ledmatrix[0];

// Check here additionally how to define this matrix:
// https://github.com/marcmerlin/FastLED_NeoMatrix
FastLED_NeoMatrix *matrix = new FastLED_NeoMatrix(leds, MATRIX_WIDTH, 10,
1, 2,
  NEO_MATRIX_TOP + NEO_MATRIX_LEFT +
    NEO_MATRIX_ROWS + NEO_MATRIX_ZIGZAG +
    NEO_TILE_TOP + NEO_TILE_LEFT +  NEO_TILE_PROGRESSIVE);
    

#define LED_RED_VERYLOW 	(3 <<  11)
#define LED_RED_LOW 		(7 <<  11)
#define LED_RED_MEDIUM 		(15 << 11)
#define LED_RED_HIGH 		(31 << 11)
#define LED_GREEN_VERYLOW	(1 <<  5)   
#define LED_GREEN_LOW 		(15 << 5)  
#define LED_GREEN_MEDIUM 	(31 << 5)  
#define LED_GREEN_HIGH 		(63 << 5)  
#define LED_BLUE_VERYLOW	3
#define LED_BLUE_LOW 	    7
#define LED_BLUE_MEDIUM 	15
#define LED_BLUE_HIGH 		31

#define LED_ORANGE_VERYLOW	(LED_RED_VERYLOW + LED_GREEN_VERYLOW)
#define LED_ORANGE_LOW		(LED_RED_LOW     + LED_GREEN_LOW)
#define LED_ORANGE_MEDIUM	(LED_RED_MEDIUM  + LED_GREEN_MEDIUM)
#define LED_ORANGE_HIGH		(LED_RED_HIGH    + LED_GREEN_HIGH)

#define LED_PURPLE_VERYLOW	(LED_RED_VERYLOW + LED_BLUE_VERYLOW)
#define LED_PURPLE_LOW		(LED_RED_LOW     + LED_BLUE_LOW)
#define LED_PURPLE_MEDIUM	(LED_RED_MEDIUM  + LED_BLUE_MEDIUM)
#define LED_PURPLE_HIGH		(LED_RED_HIGH    + LED_BLUE_HIGH)

#define LED_CYAN_VERYLOW	(LED_GREEN_VERYLOW + LED_BLUE_VERYLOW)
#define LED_CYAN_LOW		(LED_GREEN_LOW     + LED_BLUE_LOW)
#define LED_CYAN_MEDIUM		(LED_GREEN_MEDIUM  + LED_BLUE_MEDIUM)
#define LED_CYAN_HIGH		(LED_GREEN_HIGH    + LED_BLUE_HIGH)

#define LED_WHITE_VERYLOW	(LED_RED_VERYLOW + LED_GREEN_VERYLOW + LED_BLUE_VERYLOW)
#define LED_WHITE_LOW		(LED_RED_LOW     + LED_GREEN_LOW     + LED_BLUE_LOW)
#define LED_WHITE_MEDIUM	(LED_RED_MEDIUM  + LED_GREEN_MEDIUM  + LED_BLUE_MEDIUM)
#define LED_WHITE_HIGH		(LED_RED_HIGH    + LED_GREEN_HIGH    + LED_BLUE_HIGH)

uint16_t rndColor(uint8_t randomColor) {
  uint16_t color = LED_RED_LOW;
switch (randomColor)
    {
    case 1:
      color = LED_RED_HIGH;
      break;
    case 2:
      color = LED_GREEN_HIGH;
      break;
    case 3:
      color = LED_BLUE_HIGH;
      break;
    case 4:
      color = LED_PURPLE_HIGH;
      break;
    case 5:
      color = LED_CYAN_HIGH;
      break;
    case 6:
      color = LED_PURPLE_VERYLOW;
      break;
    case 7:
      color = matrix->Color(100,100,0);
      break;
    case 8:
      color = matrix->Color(150,150,0);
      break;
    case 9:
      color = LED_WHITE_MEDIUM;
      break;

    default:
      color = LED_WHITE_HIGH;
      break;
    }
    return color;
  }

struct tm myTime;

void setup() {
    
    Serial.begin(115200);
    Serial2.begin(SERIAL2_BAUDS, SERIAL_8N1, RXD2, TXD2);

    uint16_t numMatrix = MATRIX_WIDTH*MATRIX_HEIGHT;
    FastLED.addLeds<NEOPIXEL,MATRIX_DATA_PIN>(leds, numMatrix).setCorrection(TypicalLEDStrip);
 
    Serial.printf("COLORS\ngreen_low:%d blue_low:%d blue_medium:%d blue_high:%d\n red_low:%d red_medium:%d red_high:%d\n",
    LED_GREEN_LOW,LED_BLUE_LOW,LED_BLUE_MEDIUM,LED_BLUE_HIGH,LED_RED_LOW,LED_RED_MEDIUM,LED_RED_HIGH);
    rtc.init(); // initialize the RTC that's found (amongst 3 supported) on the default I2C pins for this target board

#if SET_CLOCK 
    myTime.tm_hour = hr;
    myTime.tm_min = m;
    myTime.tm_year = year;
    myTime.tm_mon = month;
    myTime.tm_mday = day;
    rtc.setTime(&myTime);
#endif

    matrix->begin();
    matrix->setTextWrap(true);
    matrix->setBrightness(MATRIX_BRIGHTNESS);
    matrix->setTextColor(LED_WHITE_LOW);
    matrix->setCursor(4, 10);
    matrix->setFont(&Font1);
    matrix->print("HOLA");
    matrix->show();
    delay(1000);
}

void loop() {
  matrix->clear();
  char timestr[12];
  rtc.getTime(&myTime); // Read the current time from the RTC into our time structure
  sprintf(timestr, "%02d:%02d", myTime.tm_hour, myTime.tm_min);
  matrix->setCursor(4, 14);
  matrix->setFont(&Font1);
  matrix->print(timestr);
  matrix->show();
  delay(60000);
}
