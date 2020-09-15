![Remora Logo](./assets/remora-matrix.svg)

# Remora-matrix

Remora listens to UDP short commands or MIDI to trigger LED Matrix Animations. Receives ; commands from [ORCΛ](https://github.com/hundredrabbits/Orca)

## Launching matrix effects

Our previous [Firmware Remora](https://github.com/martinberlin/Remora) was intended to receive short commands from ORCΛ and make very simple Neopixels  animations on addressable LEDs stripes (WS2812B like)

## Ideas to develop

* Use midi as a direct source to launch animations. No middlewares means less latency.
* Have an alternative [Nodejs midi middleware](https://github.com/martinberlin/Remora-midi/tree/master/middleware) as an alternative so you don't need midi special gear to use this Firmware.
* Use [Adafruit Neomatrix GFX](https://learn.adafruit.com/adafruit-neopixel-uberguide/neomatrix-library) as a graphic library for Neopixel matrix so you can treat it is as a display. That means full geometric functions and also font support.
* Build a new midi language that has full chord support, state (on/off) and velocity.

The language should be kept simple and also short so it can fly fast via UDP. First iteration looks like this:

```
Chord HEXA | note on/off (1 char) | velocity HEXA
48140 C on 40
48030 C off
```

* Build some simple animation effects based on the Chord / Velocity and keep the effect running till the note is released
* Also build a mode that is not animated but instead launches GFX's (Squares, Circles, you name it) with different locations using Chord and sizes based on velocity.
So a C pressed lightly will make a small circle, a D pressed strong will make a bigger one some pixels ahead.

## Demos organization in this repository

Since this will be a code repository with more than one example we will use the src_dir parameter to choose what to compile.
Just edit platformio.ini and uncomment only one of the **src_dir** folders to select what example to run.


## Companion applications

* [Remora](https://github.com/martinberlin/Remora) just for short Led stripe animations
* [ORCΛ Sequencer](https://github.com/hundredrabbits/Orca)