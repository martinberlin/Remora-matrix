#include <SPI.h>                // Adafruit asks for this also we don't use SPI at all
#include <Adafruit_GFX.h>
#include <Adafruit_NeoMatrix.h>
#include <Adafruit_NeoPixel.h>
#include "WiFi.h"
#include "freertos/FreeRTOS.h"
#include "freertos/timers.h"
#include "AsyncUDP.h"
// Note all this define's are shared between demos so is defined in central platformio.ini

#define debugMode true

// Message transport protocol
AsyncUDP udp;
// Define your Matrix following Adafruit_NeoMatrix Guides
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
#define LED_BLUE_LOW 		7
#define LED_BLUE_MEDIUM 	15
#define LED_BLUE_HIGH 		31

TimerHandle_t wifiReconnectTimer;

void connectToWifi() {
  Serial.println("Connecting to Wi-Fi...");
  WiFi.begin(WIFI_SSID, WIFI_PASS);
}

void print(String message, bool newline = true)
{
  if (debugMode) {
    if (newline) {
      Serial.println(message);
    } else {
      Serial.print(message);
    }
   }
}

void WiFiEvent(WiFiEvent_t event) {
    Serial.printf("[WiFi-event] event: %d\n", event);
    switch(event) {
    case SYSTEM_EVENT_STA_GOT_IP:
        print("WiFi connected");

        // Start UDP
    if(udp.listen(UDP_PORT)) {
        print("UDP Listening on IP: ");
        Serial.println(WiFi.localIP().toString()+":"+String(UDP_PORT));

    // Callback that gets fired every time an UDP Message arrives
    udp.onPacket([](AsyncUDPPacket packet) {
      String command;
    
      for ( int i = 0; i < packet.length(); i++ ) {
          command += (char)packet.data()[i];
      }
      matrix->clear();
      matrix->setCursor(1,6);
      matrix->print(command);
      matrix->show();
      
      
      if(debugMode) {
            Serial.write(packet.data(), packet.length());Serial.println();
        }

    });
        break;
    case SYSTEM_EVENT_STA_DISCONNECTED:
        Serial.println("WiFi lost connection");
        // TODO: Ensure we don't start UDP while reconnecting to Wi-Fi (low prio)
	      xTimerStart(wifiReconnectTimer, 0);
        break;
    }
    
    }
}


void setup() {
    Serial.begin(115200);
    connectToWifi();
    WiFi.onEvent(WiFiEvent);

    // Set up automatic reconnect timer
    wifiReconnectTimer = xTimerCreate("wifiTimer", pdMS_TO_TICKS(2000), pdFALSE, (void *)0, reinterpret_cast<TimerCallbackFunction_t>(connectToWifi));


    matrix->begin();
    matrix->setTextWrap(true);
    matrix->setBrightness(MATRIX_BRIGHTNESS);

    matrix->setTextColor(LED_BLUE_LOW);
    matrix->print("50x20");
    matrix->show();
    /* delay(4000);
    matrix->clear();
    matrix->show(); */
}

void loop() {
}