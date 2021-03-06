#!/usr/bin/env node
/**
 * MIDI-to-UDP middleware. Written by Martin Fasani https://github.com/martinberlin
 */
const yargs = require("yargs");
const midi = require('midi');
const dgram = require('dgram');

// This should match the note status on:
const noteStatusOn = 144;
var udpclient = dgram.createSocket('udp4');

var DEBUG = true;
var PORT = 49161;       // Default Orca UDP output port
// Set up a new input.
const input = new midi.Input();
// Print port names
let udp_ip = "";
let ports = "ID: Port name\n";
for (let n=0; n<input.getPortCount(); n++){
  ports += n.toString() +': '+input.getPortName(n)+"\n";
}

let options = yargs
 .usage("Usage: -p <MIDI_PORT_ID>")
 .option("p", { alias: "port_id", describe: ports, type: "number", demandOption: true })
 .option("u", { alias: "udp_ip", describe: udp_ip, type: "string", demandOption: true })
 .argv;

console.log("Listening to: "+input.getPortName(options.port_id)+ ' and forwarding to '+options.udp_ip+':'+PORT);

let logNotes = true;
// Configure a callback.
input.on('message', (deltaTime, message) => {
  
  // The message is an array of numbers corresponding to the MIDI bytes: https://www.cs.cf.ac.uk/Dave/Multimedia/node158.html
  if (Array.isArray(message) && message[0]!==248) {
    // The line that solved the not being called issue:
    setImmediate(() => {})
    noteVelocity = '00';
    //console.log("STATUS:"+message[0]);
    noteStatus = (message[0]>=noteStatusOn) ? '1' : '0';
    if (typeof message[1]!=='number') {
      return;
    }

    noteChord  = (message[1].toString(16).length==1) ?'0'+message[1].toString(16).toUpperCase():message[1].toString(16).toUpperCase();  // FF 0 01
    
    if (typeof message[2]==='number') {
      ucVelocity = message[2].toString(16).toUpperCase();
      noteVelocity  = (message[2].toString(16).length==1) ?'0'+ucVelocity:ucVelocity;
    }
    let noteInfo = noteChord+noteStatus+noteVelocity;

    if (logNotes) {
      console.log(noteInfo);
      logNotes = false;
    }

    // When called inside the midi callback it simply does not work
    sendPerUdp(noteInfo);
  }
});

// Open the port received per command line -p parameter
input.openPort(options.port_id);
input.ignoreTypes(false, false, false);


function sendPerUdp(noteInfo) {
  var inBuffer = Buffer.from(noteInfo, 'utf8');
  udpclient.send(inBuffer,PORT,options.udp_ip, function(err, bytes) {
    if (err) {
      console.log(err);
      throw err;
    }
    
    if (DEBUG) console.log(inBuffer.toString());
  });
}
