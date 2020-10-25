#include <SPI.h>                // Adafruit asks for this also we don't use SPI at all
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
// Note all this define's are shared between demos so is defined in central platformio.ini

// Create a vector containing incoming Notes
std::vector<uint8_t> vNote;
uint8_t velocity_division = 13;
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


uint16_t colorSampler1(uint8_t velocity) {
  uint16_t color = LED_GREEN_LOW;
  esp_random();
  uint8_t randomColor = random(5);
  if (velocity>29 && velocity<40) {
      color = LED_BLUE_LOW;
  }
  if (velocity>39 && velocity<50) {
      color = LED_BLUE_MEDIUM;
  }
  if (velocity>49 && velocity<56) {
      color = LED_BLUE_HIGH;
  }
  if (velocity>55 && velocity<58) {
      color = LED_RED_LOW;
  }
  if (velocity>57 && velocity<63) {
      color = LED_RED_MEDIUM;
  }
  if (velocity>63) {
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
  }
  return color;
}

// Simple shape selector
void shapeDrawing1(uint8_t note, double radius, uint8_t velocity, uint16_t color) {
  double absNote = (note-53<1)?1:(note-53)*2;
  // If the note is pair then
  if (note%2 == 0) {
     matrix->fillCircle(absNote, yAxisCenter, radius, color);
     return;
  }
  if (note%3 == 0) {
     matrix->fillTriangle(absNote-(velocity/velocity_division), yAxisCenter, absNote, yAxisCenter-radius,absNote+radius, yAxisCenter, color);
     return;
  }
  matrix->fillRect(absNote, yAxisCenter, radius, radius, color);
}

void WiFiEvent(WiFiEvent_t event) {
    Serial.printf("[WiFi-event] event: %d\n", event);
    switch(event) {
    case SYSTEM_EVENT_STA_GOT_IP:
        Serial.println("WiFi connected");
        matrix->print(WiFi.localIP().toString());
        matrix->show();

    // Start UDP
    if(udp.listen(UDP_PORT)) {
        Serial.println("UDP Listening on IP: ");
        Serial.println(WiFi.localIP().toString()+":"+String(UDP_PORT));

    
    // Callback that gets fired every time an UDP Message arrives
    udp.onPacket([](AsyncUDPPacket packet) {
      if (firstNote) {
        matrix->fillRect(0,0,MATRIX_WIDTH,MATRIX_HEIGHT,matrix->Color(0,0,0));
        matrix->show();
      }
      char note1 = packet.data()[0];
      char note2 = packet.data()[1];
      char noteArray[2] = {note1,note2};
      uint8_t note = StrToHex(noteArray);

      uint8_t status = packet.data()[2];
      char velocity1 = packet.data()[3];
      char velocity2 = packet.data()[4];
      char velArray[2] = {velocity1,velocity2};
      uint8_t velocity = StrToHex(velArray);          
      
      if (status != 48) { // status 0: 48 in ASCII table is '0'
        // Add the note to the vector
        vNote.push_back(note);
        cRadius = velocity/velocity_division;
        if (cRadius<2) cRadius=2;
        shapeDrawing1(note, cRadius, velocity, colorSampler1(velocity));

         #if defined(DEBUGMODE) && DEBUGMODE==1
            Serial.printf("note:%d \n",note);
            //Serial.printf("Note HEX:%c%c st:%c S:%d vel:%c%c\n",note1,note2,status,s,velocity1,velocity2);
            Serial.printf("N:%d V:%d cX:%.1f cR:%.1f col:%d\n", note, velocity,cX,cRadius,color);
         #endif
         
      } else { 
        // Iterate and delete release notes from Vector
        uint8_t vIdx = 0;
        for (auto n:vNote) {
          //if (vIdx==0) printf("-\n");
          //printf("%d\n",n);
          if (note == n) {
            // Hangs all if using +vIdx:
            vNote.erase(vNote.begin());
            
            // status 1: Turn this note off drawing same shape in Black
            // Missing to store here the other elemements of the note like cRadius
            shapeDrawing1(note, cRadius, velocity, matrix->Color(0,0,0));
          }
          ++vIdx;
        }
        //printf("E%d\n",vIdx);
      }
      matrix->show();

    });
        break;

    case SYSTEM_EVENT_STA_DISCONNECTED:
        Serial.println("WiFi lost connection");
	      xTimerStart(wifiReconnectTimer, 0);
        matrix->print("No Wifi");
        matrix->show();
        break;
    }
    
    }
}

void setup() {
    
    Serial.begin(115200);
    uint16_t numMatrix = MATRIX_WIDTH*MATRIX_HEIGHT;
    FastLED.addLeds<NEOPIXEL,MATRIX_DATA_PIN>(leds, numMatrix).setCorrection(TypicalLEDStrip);

    Serial.printf("COLORS\ngreen_low:%d blue_low:%d blue_medium:%d blue_high:%d\n red_low:%d red_medium:%d red_high:%d\n",
    LED_GREEN_LOW,LED_BLUE_LOW,LED_BLUE_MEDIUM,LED_BLUE_HIGH,LED_RED_LOW,LED_RED_MEDIUM,LED_RED_HIGH);

    connectToWifi();

    WiFi.onEvent(WiFiEvent);

    // Set up automatic reconnect timer
    wifiReconnectTimer = xTimerCreate("wifiTimer", pdMS_TO_TICKS(2000), pdFALSE, (void *)0, reinterpret_cast<TimerCallbackFunction_t>(connectToWifi));


    matrix->begin();
    matrix->setTextWrap(true);
    matrix->setBrightness(MATRIX_BRIGHTNESS);
    matrix->setTextColor(LED_ORANGE_MEDIUM);

    matrix->show();
}

void loop() {
}