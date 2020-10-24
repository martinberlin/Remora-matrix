#include <SPI.h>                // Adafruit asks for this also we don't use SPI at all
#include <Adafruit_GFX.h>
#include <Adafruit_NeoMatrix.h>
#include <Adafruit_NeoPixel.h>
#include "WiFi.h"
#include "freertos/FreeRTOS.h"
#include "freertos/timers.h"
#include "AsyncUDP.h"
// Note all this define's are shared between demos so is defined in central platformio.ini

// Message transport protocol
AsyncUDP udp;
// Define your Matrix following Adafruit_NeoMatrix Guide: https://learn.adafruit.com/adafruit-neopixel-uberguide/neomatrix-library
Adafruit_NeoMatrix *matrix = new Adafruit_NeoMatrix(MATRIX_WIDTH, MATRIX_HEIGHT, MATRIX_DATA_PIN,
  NEO_MATRIX_TOP     + NEO_MATRIX_LEFT +
  NEO_MATRIX_ROWS    + NEO_MATRIX_ZIGZAG,
  NEO_GRB           + NEO_KHZ800);

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
double cX = 0;
double cRadius = 0;
uint16_t yAxisCenter = MATRIX_HEIGHT/2;
bool firstNote = true;

void connectToWifi() {
  Serial.println("Connecting to Wi-Fi...");
  WiFi.begin(WIFI_SSID, WIFI_PASS);
}

uint8_t StrToHex(char str[])
{
  return (uint8_t) strtol(str, 0, 16);
}

void matrixShow() {
  portDISABLE_INTERRUPTS();
  matrix->show();
  portENABLE_INTERRUPTS();
}


void WiFiEvent(WiFiEvent_t event) {
    Serial.printf("[WiFi-event] event: %d\n", event);
    switch(event) {
    case SYSTEM_EVENT_STA_GOT_IP:
        Serial.println("WiFi connected");
        matrix->print(WiFi.localIP().toString());
        matrixShow();

        // Start UDP
    if(udp.listen(UDP_PORT)) {
        Serial.println("UDP Listening on IP: ");
        Serial.println(WiFi.localIP().toString()+":"+String(UDP_PORT));

    
    // Callback that gets fired every time an UDP Message arrives
    udp.onPacket([](AsyncUDPPacket packet) {
      if (firstNote) {
        matrix->fillScreen(matrix->Color(0,0,0));
        firstNote = false;
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

      uint8_t absNote = (note-53<1)?1:(note-53)*2;
      cX = absNote;
      cRadius = velocity/7;
      esp_random();
      uint8_t randomColor = random(5);

      uint16_t color = LED_GREEN_LOW;
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
      
      if (status != 48) { // status 0: 48 in ASCII table is '0'
         matrix->printf("%c%c",note1,note2);
         matrix->fillCircle(cX, yAxisCenter, cRadius, color);

         #if defined(DEBUGMODE) && DEBUGMODE==1
            //Serial.printf("Note HEX:%c%c st:%c S:%d vel:%c%c\n",note1,note2,status,s,velocity1,velocity2);
            Serial.printf("N:%d V:%d cX:%.1f cR:%.1f col:%d\n", note, velocity,cX,cRadius,color);
         #endif
         
      } else { 
        // status 1: Turn this note off
        Serial.printf("cX:%d R:%lf\n",cX,cRadius);
        matrix->fillCircle(cX, yAxisCenter, cRadius, matrix->Color(0,0,0));
      }
      matrixShow();

    });
        break;

    case SYSTEM_EVENT_STA_DISCONNECTED:
        Serial.println("WiFi lost connection");
	      xTimerStart(wifiReconnectTimer, 0);
        matrix->print("No Wifi");
        matrixShow();
        break;
    }
    
    }
}

void setup() {
    
    Serial.begin(115200);
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

    //matrix->setTextColor(LED_BLUE_LOW);
    //matrix->printf("%dx%d",MATRIX_WIDTH,MATRIX_HEIGHT);
    // Demo to check if colors are well mapped:
/*  matrix->setTextColor(LED_GREEN_LOW);
    matrix->print("Green ");
    matrix->setTextColor(LED_RED_LOW);
    matrix->print("Red"); */
    matrixShow();
}

void loop() {
}