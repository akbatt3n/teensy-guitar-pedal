This project is no longer under development.
This was a senior design project, so I'm keeping this repo as an archive while I develop the [Teensy Pedal v2.](https://github.com/akbatt3n/teensy-pedal-v2)

# Teensy Guitar Pedal
Designed for Teensy 3.6 with Teensy Audio Adaptor

https://www.pjrc.com/store/teensy36.html <br>
https://www.pjrc.com/store/teensy3_audio.html

The only circuitry involved is controls and an LCD, have a look at the code for pin assignments. The audio in/out connects to the audio adaptor.

The stomp switches are 3P2T/3PDT, either pulling their Teensy pin high or low. Buttons for cycling through effects simply connect their pin to GND, or don't. The pins are configured with a pullup resistor. This is done with pinMode(pin#, INPUT_PULLUP); or by adding a pullup resistor physically.

The current code is not fully functional. Of the 3 effects (bitcrusher, synth, and overdrive), bitcrusher is fully functional, synth works at higher frequencies (for a bass guitar) but with a significant delay before matching the note being played, and overdrive doesn't work at all.
