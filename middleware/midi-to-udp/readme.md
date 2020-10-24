## Nodejs midi middleware

This code will run in a loop using Nodejs and it will receive midi input, convert it to short commands, and redirect it per UDP to an IP Address.
The ESP32 will be hearing on this IP address:port this commands and your laptop will do the job to receive the Midi signals and redirect them to WiFi board that controls the LED Matrix. 
This reduces the hardware gear to use since you don't need an special [Midi shield](https://www.sparkfun.com/products/12898) but will also increase latency between the played note and the signal arrival to the ESP32.

```
 ________________       ____                  ____________________
|MIDI instrument |     | PC |                |o Led matrix ESP32 o|
|________________| --->|____| --WiFi UDP --> |__o__o__o__o__o__o__| 
                        Both PC and ESP32 are in same WiFi network
```

### Running 

After successful installation using npm install, run first:

```
nodejs midi.js

  -p, --port_id  ID: Port name
                 0: Midi Through:Midi Through Port-0 14:0
                 1: USB MIDI Interface:USB MIDI Interface MIDI 1 24:0
                                                             [number] [required]
  -u, --udp_ip                                               [string] [required]
```

This will list the USB midi porst. Just select the one that works for you and then tell the midi script what is the UDP IP where the notes will be redirected:

```
nodejs midi.js -p 1 -u 192.168.0.11
```

### Instructions to install 

**Running:**

npm install

I got an error about gyp==0.1 distribution was not found
I solved it using pyenv to switch my Python version to last 2.x version.

After this you can install that requirement using pip with following command:

pip install git+https://chromium.googlesource.com/external/gyp

Thanks to this [question in stackoverflow](https://stackoverflow.com/questions/40025591/the-gyp-0-1-distribution-was-not-found)

[node-midi library](https://github.com/justinlatimer/node-midi)

[Midi messages format PDF](https://www.nyu.edu/classes/bello/FMT_files/9_MIDI_code.pdf)
