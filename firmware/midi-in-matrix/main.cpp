#include <SPI.h> 
// Super proud of amazing Marc: https://github.com/marcmerlin All credits for this go to him!
#include <Adafruit_GFX.h>
#include <FastLED_NeoMatrix.h>
#include <FastLED.h>
#include <LEDMatrix.h>

#include "WiFi.h"
#include "freertos/FreeRTOS.h"
#include "freertos/timers.h"
#include "AsyncUDP.h"
#include <vector>

// Try out Hardware serial: https://forum.arduino.cc/index.php?topic=603775.0
//#include <HardwareSerial.h>
// serial(1) = pin27=RX green, pin26=TX white

// Note all this define's are shared between demos so is defined in central platformio.ini

// Create a vector containing incoming Notes
std::vector<uint8_t> vNote;
// Enabling this converts Note 71 that is electribe's potentiometer into the velocity divisor (Makes smaller/bigger shapes)
#define POTENCIOMETER_DIVISOR_KORG 0
// If the figures are too small just reduce this. If too big, just increase. SIZE / velocity_division
uint8_t velocity_division = 20;
// Message transport protocol
AsyncUDP udp;
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

TimerHandle_t wifiReconnectTimer;
uint8_t offCount = 0;
// Matrix pointers
// cX, yAxisCenter, cRadius : Test to save this globally
double cRadius = 0;
uint16_t yAxisCenter = MATRIX_HEIGHT/2;
bool firstNote = true;

void connectToWifi() {
  Serial.println("Connecting to Wi-Fi...");
  WiFi.begin(WIFI_SSID, WIFI_PASS);
}

uint8_t StrToHex(char str[]) {
  return (uint8_t) strtol(str, 0, 16);
}

uint16_t rndColorLow(uint8_t randomColor) {
  uint16_t color = LED_RED_LOW;
switch (randomColor)
    {
    case 1:
      color = LED_RED_VERYLOW;
      break;
    case 2:
      color = LED_GREEN_VERYLOW;
      break;
    case 3:
      color = LED_BLUE_VERYLOW;
      break;
    case 4:
      color = LED_PURPLE_VERYLOW;
      break;
    case 5:
      color = LED_CYAN_VERYLOW;
      break;
    }
    return color;
}

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
    default:
      color = LED_ORANGE_HIGH;
      break;
    }
    return color;
    }

uint16_t colorSampler1(uint8_t velocity) {
  uint16_t color = LED_GREEN_LOW;
  esp_random();
  uint8_t randomColor = random(5);
  if (velocity>29 && velocity<36) {
      color = LED_BLUE_LOW;
  }
  if (velocity>35 && velocity<41) {
      color = LED_BLUE_MEDIUM;
  }
  if (velocity>40 && velocity<46) {
      color = LED_BLUE_HIGH;
  }
  if (velocity>45 && velocity<51) {
      color = LED_GREEN_LOW;
  }
  if (velocity>50 && velocity<56) {
      color = LED_GREEN_MEDIUM;
  }
  if (velocity>55 && velocity<61) {
      color = LED_ORANGE_MEDIUM;
  }
  if (velocity>50 && velocity<63) {
      color = LED_RED_MEDIUM;
  }
  if (velocity>63) {
    color = rndColor(randomColor);
  }
  return color;
}

// Simple shape selector
void shapeDrawing1(uint8_t note, double radius, uint8_t velocity, uint16_t color) {
  double absNote = (note-53<1)?1:(note-53)*2;
  uint8_t randomX = random(MATRIX_WIDTH);
  uint8_t randomY = random(MATRIX_HEIGHT);
  uint8_t randomX2 = random(MATRIX_WIDTH);
  // If the note is pair then
  if (note%2 == 0) {
     matrix->fillCircle(absNote, yAxisCenter, radius, color);
     return;
  }
  if (note%3 == 0) {
     matrix->fillTriangle(absNote-(velocity/10), yAxisCenter, absNote, yAxisCenter-radius,absNote+radius, yAxisCenter, rndColorLow(random(5)));
     return;
  }
  if (note%5 == 0) {
     matrix->fillTriangle(yAxisCenter, absNote-(velocity/10), absNote, yAxisCenter-radius,absNote+radius, yAxisCenter+(velocity/10), rndColorLow(random(5)));
     return;
  }
  matrix->fillRect(absNote, yAxisCenter, radius, radius, color);
}

// Draw Piano keys selector
void shapePianoKeys(uint8_t note, double radius, uint8_t velocity, uint16_t color) {
  double absNote = (note-53<1)?4:(note-53)*2;
  // If the note is pair then
  if (note%2 == 0) {
     matrix->fillRect(absNote, yAxisCenter, radius, radius*2, (color==0)?0:LED_WHITE_MEDIUM);
     return;
  }
  if (note%3 == 0) {
     matrix->fillRect(absNote, yAxisCenter, radius, radius*2, color);
     return;
  }
  matrix->fillCircle(absNote, yAxisCenter, radius, color);
}

/**
 * Jumper function, uncomment here or add more shapes that will trigger with the Notes played
 */
void shapeSelector(uint8_t note, double radius, uint8_t velocity, uint16_t color) {
  //shapeDrawing1(note, cRadius, velocity, color);
  shapePianoKeys(note, cRadius, velocity, color);
}


// Serial Callback 
void serialIn(uint8_t in[5]) {
      if (firstNote) {
        matrix->fillRect(0,0,MATRIX_WIDTH,MATRIX_HEIGHT,matrix->Color(0,0,0));
        matrix->show();
        firstNote = false;
      }

      // Extract from Serial 
      // NN1VV  Note (2) status (bool) Velocity (2) 
      char note1 = in[0];
      char note2 = in[1];
      char noteArray[2] = {note1,note2};
      uint8_t note = StrToHex(noteArray);

      uint8_t status = in[2];
      char velocity1 = in[3];
      char velocity2 = in[4];
      char velArray[2] = {velocity1,velocity2};
      uint8_t velocity = StrToHex(velArray);          
      
      if (status != 48) { // status 0: 48 in ASCII table is '0'
        // Add the note to the vector
        vNote.push_back(note);
        // Crude test to use the pan Potentiometer to regulate shape size
      #if defined(POTENCIOMETER_DIVISOR_KORG) && POTENCIOMETER_DIVISOR_KORG==1
        if (note==72 && velocity<21) {
          velocity_division = velocity;
          printf("divisor:%d\n",velocity_division);
          matrix->fillRect(0,0,MATRIX_WIDTH,MATRIX_HEIGHT,matrix->Color(0,0,0));
        }
      #endif

        cRadius = velocity/velocity_division;
        if (cRadius<2) cRadius=2;
        shapeSelector(note, cRadius, velocity, colorSampler1(velocity));

         #if defined(DEBUGMODE) && DEBUGMODE==1
            Serial.printf("note:%d \n",note);
            //Serial.printf("Note HEX:%c%c st:%c S:%d vel:%c%c\n",note1,note2,status,s,velocity1,velocity2);
            Serial.printf("N:%d V:%d cX:%.1f cR:%.1f col:%d\n", note, velocity,cX,cRadius,color);
         #endif
         
      } else { 
        // Iterate and delete released notes from Vector

        std::vector<uint8_t>::iterator it = vNote.begin();
        for ( ; it != vNote.end(); ) {
          if (*it==note) {
            it = vNote.erase(it);
            //printf("delNote:%d ",note);
            // status 1: Turn this note off drawing same shape in Black
            // Missing to store here the other elemements of the note like cRadius
            shapeSelector(note, cRadius, velocity, matrix->Color(0,0,0));
          } else {
            ++it;
          }
        }
      }
      matrix->show();
}

void setup() {
    
    Serial.begin(115200);
    Serial2.begin(SERIAL2_BAUDS, SERIAL_8N1, RXD2, TXD2);
    delay(100);

    uint16_t numMatrix = MATRIX_WIDTH*MATRIX_HEIGHT;
    FastLED.addLeds<NEOPIXEL,MATRIX_DATA_PIN>(leds, numMatrix).setCorrection(TypicalLEDStrip);

    Serial.printf("COLORS\ngreen_low:%d blue_low:%d blue_medium:%d blue_high:%d\n red_low:%d red_medium:%d red_high:%d\n",
    LED_GREEN_LOW,LED_BLUE_LOW,LED_BLUE_MEDIUM,LED_BLUE_HIGH,LED_RED_LOW,LED_RED_MEDIUM,LED_RED_HIGH);


    matrix->begin();
    matrix->setTextWrap(true);
    matrix->setBrightness(MATRIX_BRIGHTNESS);
    matrix->setTextColor(LED_ORANGE_MEDIUM);
    matrix->print("Mini IN");
    matrix->show();
}

void loop() {

  while (Serial2.available() > 0)
  {
    uint8_t midi_in = Serial2.read();

    Serial.printf("%d ",midi_in);
    if (midi_in == 7) {
      Serial.println();
    }
    delay(600);
  }

}