## Nodejs midi middleware

This code will run in a loop using Nodejs and it will receive midi input, convert it to short commands, and redirect it per UDP to an IP Address.
The ESP32 will be hearing on this IP address:port this commands and your laptop will do the job to receive the Midi signals and redirect them to WiFi board that controls the LED Matrix. This reduces the hardware gear to use since you don't need an special [Midi shield](https://www.sparkfun.com/products/12898) but will also increase latency between the played note and the signal arrival to the ESP32.

```
|MIDI instrument |     | PC |                |oö Led matrix ESP32 öo|
|________________| --->|____| --WiFi UDP --> |_______________________ 

```

### Instructions to install 

Running:

npm install

I got an error about gyp==0.1 distribution was not found
I solved it using pyenv to switch my Python version to last 2.x version.

After this you can install that requirement using pip with following command:

pip install git+https://chromium.googlesource.com/external/gyp


Thanks to this [question in stackoverflow](https://stackoverflow.com/questions/40025591/the-gyp-0-1-distribution-was-not-found)

[node-midi library](https://github.com/justinlatimer/node-midi)

[Midi messages format PDF](https://www.nyu.edu/classes/bello/FMT_files/9_MIDI_code.pdf)
