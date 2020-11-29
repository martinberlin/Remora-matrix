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

// Globals to keep track of loop() iterations
uint8_t midi_channel = 1;
uint8_t midi_index = 0;
uint8_t midi_status = 0;
uint8_t midi_note = 0;
uint8_t midi_velocity = 0;
uint8_t last_velocity = 127;
double note_x_factor = (MIDI_TOP_OCTAVE*12)/MATRIX_WIDTH;
// If the figures are too small just reduce this. If too big, just increase. SIZE / velocity_division
uint8_t velocity_division = 12;

uint8_t erase_counter=0;
char parser[8];
// Stores the message that triggers a shape. NNSVV Note, Status, Velocity
char midi_msg[6];
// Hardware serial2 pins: Defined in platformio.ini

// Create a vector containing incoming Notes
std::vector<uint8_t> vNote;
uint8_t note_last_velocity[127];
// Enabling this converts Note 71 that is electribe's potentiometer into the velocity divisor (Makes smaller/bigger shapes)
#define POTENCIOMETER_DIVISOR_KORG 0

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

uint8_t offCount = 0;
// Matrix pointers
// cX, yAxisCenter, shapeSize : Test to save this globally
double shapeSize = 0;
// Feel free to modify this to adapt to your display. MIDDLE - Yoffset (Negative brings it up/down depending on your display config)
uint16_t yAxisCenter = MATRIX_HEIGHT/2-5;
bool firstNote = true;

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
      color = LED_RED_LOW;
  }
  if (velocity>62 && velocity<81) {
      color = LED_RED_MEDIUM;
  }
  if (velocity>80 && velocity<97) {
      color = LED_RED_HIGH;
  }
  if (velocity>96) {
    color = rndColor(randomColor);
  }
  return color;
}

bool isSemitone(uint8_t note) {
  bool semitone = false;
  uint8_t chord = note%12;
  if ((chord<4 && chord%2!=0) || (chord>5 && chord%2==0)) {
    semitone = true;
  }
  #if defined(DEBUGMODE) && DEBUGMODE==1
    uint8_t octave = note/12;
    //Serial.printf("Ch:%d N:%d Oct:%d Chord:%d #:%d\n", midi_channel, midi_note, octave, chord, semitone);
  #endif
  return semitone;
}

// Feel free to adapt and expand this shape selectors!
void shapeCircle(uint8_t x, double radius, uint8_t velocity, uint16_t color) {  
  matrix->fillCircle(x, yAxisCenter, radius/3, color);
  return;
  }

void shapeTriangle(uint8_t x, double radius, uint8_t velocity, uint16_t color) {
  matrix->fillTriangle(x, yAxisCenter, x, yAxisCenter-radius,x+radius, yAxisCenter, (color==0)?0:colorSampler1(velocity));
  return;
  }
void shapeTriangle2(uint8_t x, double radius, uint8_t velocity, uint16_t color) {
  matrix->fillTriangle(x, yAxisCenter, x, yAxisCenter+radius,x, yAxisCenter, (color==0)?0:LED_WHITE_MEDIUM);
  return;
  }
void shapeTriangle3(uint8_t x, double radius, uint8_t velocity, uint16_t color) {
  matrix->fillTriangle(x, yAxisCenter, x, yAxisCenter+radius, x-radius, yAxisCenter, (color==0)?0:velocity);
  return;
  }
void shapeLineH(uint8_t x, double radius, uint8_t velocity, uint16_t color) {
  uint8_t semiYOffset = (isSemitone(midi_note)) ? 3 : 0;
  matrix->drawFastHLine(x, yAxisCenter+semiYOffset,velocity/velocity_division,color);
  matrix->drawFastHLine(x, yAxisCenter+semiYOffset+1,velocity/velocity_division,color);
  return;
  }
void shapeLineV(uint8_t x, double radius, uint8_t velocity, uint16_t color) {
  uint8_t semiYOffset = (isSemitone(midi_note)) ? 3 : 0;
  matrix->drawFastVLine(x, yAxisCenter+semiYOffset,velocity/velocity_division,color);
  matrix->drawFastVLine(x+1, yAxisCenter+semiYOffset,velocity/velocity_division,color);
  return;
  }
void shapeRectangle(uint8_t x, double radius, uint8_t velocity, uint16_t color) { 
  matrix->fillRoundRect(x, yAxisCenter+4, radius, velocity/4, 4, rndColor(midi_note%10)); 
  return;
}

// Draw Piano keys selector
void shapePianoKeys(uint8_t x, double radius, uint8_t velocity, uint16_t color) { 
  uint8_t semiYOffset = (isSemitone(midi_note)) ? 4 : 0;
  //Serial.printf("Ch:%d N:%d x:%d Vel:%d\n",midi_channel,midi_note,x,velocity);
  matrix->fillRect(x, yAxisCenter-semiYOffset, radius, velocity/3, (color==0)?0:LED_WHITE_MEDIUM); // LED_WHITE_MEDIUM
  return;
}
void shapePianoKeys2(uint8_t x, double radius, uint8_t velocity, uint16_t color) {
  uint8_t semiYOffset = (isSemitone(midi_note)) ? 3 : 0;
  matrix->fillRect(x, yAxisCenter-semiYOffset, radius, velocity/3, (color==0)?0:matrix->Color(100,0,100)); // red+blue= violet
  return;
}
void shapePianoKeys3(uint8_t x, double radius, uint8_t velocity, uint16_t color) {
  uint8_t semiYOffset = (isSemitone(midi_note)) ? 2 : -2;
  matrix->fillRect(x, yAxisCenter-semiYOffset, radius, velocity/3, (color==0)?0:matrix->Color(100,100,0)); // yellow
  return;
}

/**
 * Jumper function to select a shape. You can expand this to add more per each channel.
 * Ex. Print a shape per instrument (1 is mostly Piano)
 */
void shapeSelector(uint8_t note, double shapeSize, uint8_t velocity, uint16_t color) {
  uint8_t x = (note-(MIDI_BASE_OCTAVE*12))+MIDI_X_OFFSET_PIXELS;
  switch (midi_channel)
        {
        case 1:
          shapePianoKeys(x, shapeSize, velocity, color);
          break;
        case 2:
          shapeTriangle(x, shapeSize, velocity, color);
          break;
        case 3:
          shapeRectangle(x, shapeSize, velocity, color);
          break;
        case 4:
          shapePianoKeys(x, shapeSize, velocity, color);
          break;
        case 5:
          shapePianoKeys3(x, shapeSize, velocity, color);
          break;
        case 6:
          shapeTriangle2(x, shapeSize, velocity, color);
          break;
        case 7:
          shapeTriangle3(x, shapeSize, velocity, color);
        case 8:
        case 9:
        case 10:
          shapeLineH(x, shapeSize, velocity, color);
        break;
        case 11:
        case 12:
        case 13:
          shapeLineV(x, shapeSize, velocity, color);
        break;
        default:
          shapeCircle(x, shapeSize, velocity, color);
          break;
        }
  return;
}

/** Converts the message into a shape
 * Note as with the Serial input we receive also the MIDI channel on this version we will use this information#
 * to trigger different shapes, keeping the same message format
 */
void messageToShape(char in[6]) {
  #if defined(MIDI_CLEAN_SCREEN_EVERY) && MIDI_CLEAN_SCREEN_EVERY!=0
  erase_counter++;
  if (erase_counter%MIDI_CLEAN_SCREEN_EVERY) {
    matrix->fillRect(0,0,MATRIX_WIDTH,MATRIX_HEIGHT,matrix->Color(0,0,0));
  }
  #endif

  if (firstNote) {
    matrix->fillRect(0,0,MATRIX_WIDTH,MATRIX_HEIGHT,matrix->Color(0,0,0));
    matrix->show();
    firstNote = false;
  }

  // Extract from Serial: NN1VV  Note (2) status (bool) Velocity (2) 
  char note1 = in[0];
  char note2 = in[1];
  char noteArray[2] = {note1,note2};
  uint8_t note = StrToHex(noteArray);

  uint8_t status = in[2];
  #if defined(MIDI_FIXED_VELOCITY) && MIDI_FIXED_VELOCITY==0
    char velocity1 = in[3];
    char velocity2 = in[4];
    char velArray[2] = {velocity1,velocity2};
    uint8_t velocity = StrToHex(velArray);
  #else    
    uint8_t velocity = MIDI_FIXED_VELOCITY;
  #endif
  
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

    shapeSize = velocity/velocity_division;
    if (shapeSize<2) shapeSize=2;

    shapeSelector(note, shapeSize, velocity, colorSampler1(velocity));
    //Serial.printf("ON :%d Size:%f V:%d\n", note, shapeSize, velocity);

    note_last_velocity[note] = velocity;
  } else { 
    // Iterate and delete released notes from Vector

    std::vector<uint8_t>::iterator it = vNote.begin();
    for ( ; it != vNote.end(); ) {
      if (*it==note) {
        it = vNote.erase(it);
        //Turn this note off drawing same shape in Black
        //Serial.printf("OFF: %d size %f Vel %d\n", note, shapeSize, note_last_velocity[note]);
        shapeSelector(note, shapeSize, note_last_velocity[note], matrix->Color(0,0,0));
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

    uint16_t numMatrix = MATRIX_WIDTH*MATRIX_HEIGHT;
    FastLED.addLeds<NEOPIXEL,MATRIX_DATA_PIN>(leds, numMatrix).setCorrection(TypicalLEDStrip);
    Serial.printf("Remora-Matrix\nNote X factor:%.2f\n", note_x_factor);

    Serial.printf("COLORS\ngreen_low:%d blue_low:%d blue_medium:%d blue_high:%d\n red_low:%d red_medium:%d red_high:%d\n",
    LED_GREEN_LOW,LED_BLUE_LOW,LED_BLUE_MEDIUM,LED_BLUE_HIGH,LED_RED_LOW,LED_RED_MEDIUM,LED_RED_HIGH);

    matrix->begin();
    matrix->setTextWrap(true);
    matrix->setBrightness(MATRIX_BRIGHTNESS);
    matrix->setTextColor(LED_BLUE_MEDIUM);
    matrix->print("Midi IN");
    matrix->show();
    matrix->setTextColor(LED_BLUE_LOW);
}

bool midi_control_ignore = false;

void loop() {
  // Call MIDI.read the fastest you can for real-time performance: Receiving in TXD2 pin
  while (Serial2.available() > 0)
  {
    if (midi_index>2) {
      midi_index=0;
    }
    uint8_t midi_in = Serial2.read();
    
    // Add here midi_in messages that should be ignored
    // Serial.printf("%d ", midi_in); // uncomment for debug
    // RealTime messages discarded 0xF8
    if (midi_in > 176) break;

    switch (midi_index) {
        case 0:
        // Korg change Pattern 176: Identify and flag this message as midi_control_ignore so it's not sent as a shape
        if (midi_in == 176) {
          midi_control_ignore = true;
        }
        // channel is in low order bits and comes in byte 0
        midi_channel = (midi_in & 0x0F) + 1;
          
        if (midi_in & 0x80) {
              // Decode midi message that comes in first byte
          switch ((midi_in >> 4) & 0x07) {
            case 0:
                midi_status = 0;
                break;
            case 1:
                midi_status = 1;
                break;
            default:
                //printf("C %d ",(midi_in >> 4) & 0x07); // Sometimes comes 3
                return;

                // Fill with more cases if you want. For this example we are interested only on Note ON/OFF event
                // There is additional messages like Program change, Pitch wheel, Control change and many more!
                // More info: https://www.gammon.com.au/forum/?id=12746
            }
            
        }

        case 1:
        midi_note = midi_in;
        break;

        case 2:
        midi_velocity = midi_in;
        // Control signal ignore. Don't send shape to matrix, just a number flash
        if (midi_control_ignore) {
          // A practical way to restart this from Synth if it hanged, turning know all to left+1
          if (midi_in==1) {
            ESP.restart();
          }
          #if defined(MIDI_DRAW_CONTROL_SIGNALS) && MIDI_DRAW_CONTROL_SIGNALS==1
            matrix->setCursor(2,10);
            matrix->printf("%d", midi_in);
            matrix->show();
          #endif
          midi_control_ignore = false;
          delayMicroseconds(100);
          break;
        }
        // Last byte received. Call our function to render the event
        // Note[2] HEX Status[1] BOOL Velocity[2] HEX
        // Make nice this fucking mess, is only to force HEX to be 2 chars always
        if (midi_note<16 && midi_velocity>15) {
          sprintf(parser, "%s", "0%x%d%x");
        } else if(midi_note>15 && midi_velocity<16) {
          sprintf(parser, "%s", "%x%d0%x");
        } else if(midi_note<16 && midi_velocity<16) {
          sprintf(parser, "%s", "0%x%d0%x");
        } else {
          sprintf(parser, "%s", "%x%d%x");
        }
        sprintf(midi_msg, parser, midi_note, midi_status, midi_velocity);
        // Channels defined as global constants in platformio.ini
        // Listen all or only two channels. Expand this for more:
        if (MIDI_LISTEN_CHANNEL1==0) {
          messageToShape(midi_msg);
        } else if(MIDI_LISTEN_CHANNEL1==midi_channel || MIDI_LISTEN_CHANNEL2==midi_channel || MIDI_LISTEN_CHANNEL3==midi_channel) {
          messageToShape(midi_msg);
        }
        delayMicroseconds(10);
        //Serial.printf("Ch%d Note:%d\n M:%s",midi_channel, midi_note,midi_msg);
        #if defined(DEBUGMODE) && DEBUGMODE==1
         Serial.printf("Ch%d N:%d S:%d V:%d M:%s\n", midi_channel, midi_note, midi_status, midi_velocity, midi_msg);
        #endif
        break;
    }
  
    midi_index++;
  }
}
