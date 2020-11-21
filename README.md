![Remora Logo](./assets/remora-matrix.svg)

# Remora-matrix

Remora listens to UDP short commands or MIDI to trigger LED Matrix Animations. Receives ; commands from [ORCΛ](https://github.com/hundredrabbits/Orca)

## Launching matrix effects using MIDI as a trigger

Our previous [Firmware Remora](https://github.com/martinberlin/Remora) was intended to receive short commands from ORCΛ and make very simple Neopixels  animations on addressable LEDs stripes (WS2812B like)

**PLAN A**

To use this for the moment we convert incoming MIDI input to UDP using a **nodejs middleware**. To start it just go to the directory:

    cd middleware/midi-to-udp
    nodejs midi.js

There you will see what ports you have as incoming MIDI. You need a midi input source either an instrument or something you send from the computer itself. In my case I use a combination of Timidity and Rosegarden (Kind of Cakewalk for Linux). Once you see the port listed, just add there the -p (port) and -u (udp) IP address. UDP port is fixed to 49161 both in the middleware and also on the ESP32 Firmware. As an example:

    $ nodejs midi.js -p 3 -u 192.168.12.109
    Listening to: rosegarden:out 1 - General MIDI Device 130:3 and forwarding to 192.168.12.109:49161

This will redirect the MIDI notes to UDP. And the ESP32 will receive this UDP short messages and draw things in your LED Matrix.

**PLAN B**

On next updates we will also worrk in a module to receive directly the notes in the ESP32. Thhat will require an Sparkfun Midi-Arduino module to enable Midi thru and do the signal to serial conversion. The design to connect the ESP32 is still on the works and it will take some weeks more to see the light.
If that works as expected, then there is no more middleware needed, and no more WiFi latence delay. So this should be actually the real thing if you want to be independant of WiFi. 

![Sparkfun MIDI](./assets/midi-arduino.jpg)

## Our custom internal messages to unify both NodeJS and Midi SerialIN

In order to unify both different takes of Firmware and in the future make a common class that will listen to same commands we invented a very short internal message that uses 6 characters. We use Hexadecimal to keep it always in 2 chars length:

    2 chars (HEXA) representing Note played
    1 boolean      representing Status (1 note on, 0 note off)
    2 chars (HEXA) representing Velocity

**NNSVV**  Note, Status, Velocity

In the **udp-midi-matrix** version the middleware script should midi.js should be running in the background in order to convert MIDI signals into short UDP messages. 

In the **midi-in-matrix** version the midi is converted to UART using Sparkfun midi HAT and received directly in HardwareSerial port (Serial2) in the ESP32. So this version is WiFi independant and should have less latency. But has the downside that you need to use Midi IN cables.

**Example:**
Playing DO in octave 3 that is 36 in decimal, velocity 60, Note ON message would be:

    2413B
    When the same note is released it could be:
    24000

Note that some synths also send the release Velocity so the last 2 chars hexa is not always 0.
Channel filtering would be implemented on the future. Letting you choose between all channels or 2 channels configurable using the platformio.ini constants:

    -D MIDI_LISTEN_CHANNEL1=0
    -D MIDI_LISTEN_CHANNEL2=0

On 0 it will allow all. If you use 1 and 2 for each constant then it will listen only to those channels. Channel filtering will be only available in Serial version since I still didn't discovered where it comes in the nodejs MIDI library. Feel free to explore the readme on [middleware/midi-to-udp](https://github.com/martinberlin/Remora-matrix/tree/master/middleware/midi-to-udp)

## Ideas to develop

* Have a [Nodejs midi middleware](https://github.com/martinberlin/Remora-midi/tree/master/middleware) as an alternative so you don't need midi special gear to use this Firmware. DONE
* Plan B: Use midi as a direct source to launch animations. ON THE WORKS (midi-in-matrix)
* Use [Adafruit Neomatrix GFX](https://learn.adafruit.com/adafruit-neopixel-uberguide/neomatrix-library) as a graphic library for Neopixel matrix so you can treat it is as a display. That means full geometric functions and also font support. DONE using awesome FastLED_NeoMatrix (credits: Marc Merlin)
* Build a new midi language that has full chord support, state (on/off) and velocity. DONE

The language should be kept simple and also short so it can fly fast via UDP. First plan A iteration looks like this:

```
Chord HEXA | note on/off (1 char) | velocity HEXA
48140 C on 40
48030 C off
```

* Build some simple animation effects based on the Chord / Velocity and keep the effect running till the note is released
* Also build a mode that is not animated but instead launches GFX's (Squares, Circles, you name it) with different locations using Chord and sizes based on velocity.
So a C pressed lightly will make a small circle, a D pressed strong will make a bigger one some pixels ahead.

## Demos organization in this repository

Please make sure to edit platformio.ini and uncomment only one of the **src_dir** folders to select what example to run.
For example if you want to run the UDP to MIDI firmware just uncomment this line:

```
#Uncomment only one of the folders to select what example to run:
#src_dir = firmware/adafruit-matrix-test
src_dir = firmware/udp-midi-matrix
```
Then compiling will just pick this directory and flash it to your ESP32.

Requirements are in **platformio.ini** file. Check it out and adapt it to your board and needs.
This works super fast thanks of the work of amazing Marc Merlin's fork for [FastLED_NeoMatrix](https://github.com/marcmerlin/FastLED_NeoMatrix) that is what sends the data to your addressable LEDs data PIN.


## Companion applications

* [Remora](https://github.com/martinberlin/Remora) just for short Led stripe animations
* [ORCΛ Sequencer](https://github.com/hundredrabbits/Orca)
