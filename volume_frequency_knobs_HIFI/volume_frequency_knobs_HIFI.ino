/*  Example playing a sinewave at a set frequency,
    using Mozzi sonification library.

    Demonstrates the use of Oscil to play a wavetable.

    This sketch using HIFI mode is not for Teensy 3.1.

    IMPORTANT: this sketch requires Mozzi/mozzi_config.h to be
    be changed from STANDARD mode to HIFI.
    In Mozz/mozzi_config.h, change
    #define AUDIO_MODE STANDARD
    //#define AUDIO_MODE HIFI
    to
    //#define AUDIO_MODE STANDARD
    #define AUDIO_MODE HIFI

    Circuit: Audio output on digital pin 9 and 10 (on a Uno or similar),
    Check the Mozzi core module documentation for others and more detail

                     3.9k
     pin 9  ---WWWW-----|-----output
                    499k           |
     pin 10 ---WWWW---- |
                                       |
                             4.7n  ==
                                       |
                                   ground

    Resistors are ±0.5%  Measure and choose the most precise
    from a batch of whatever you can get.  Use two 1M resistors
    in parallel if you can't find 499k.
    Alternatively using 39 ohm, 4.99k and 470nF components will
    work directly with headphones.

    Mozzi documentation/API
		https://sensorium.github.io/Mozzi/doc/html/index.html

		Mozzi help/discussion/announcements:
    https://groups.google.com/forum/#!forum/mozzi-users

    Tim Barrass 2012-13, CC by-nc-sa.
*/

#include <MozziGuts.h>
#include <Oscil.h> // oscillator template
#include <tables/sin2048_int8.h> // sine table for oscillator

const char KNOB_PIN = 5; // set the input for the knob to analog pin 0
const char LDR_PIN = 4; // set the input for the LDR to analog pin 1

// use: Oscil <table_size, update_rate> oscilName (wavetable), look in .h file of table #included above
Oscil <SIN2048_NUM_CELLS, AUDIO_RATE> aSin(SIN2048_DATA);

byte volume;


void setup(){
  startMozzi(); // uses the default control rate of 64, defined in mozzi_config.h
}


void updateControl(){
    // read the potentiometer
  int knob_value = mozziAnalogRead(KNOB_PIN); // value is 0-1023

  // map it to an 8 bit volume range for efficient calculations in updateAudio
  volume = knob_value >> 2;  // 10 bits (0->1023) shifted right by 2 bits to give 8 bits (0->255)

  // read the light dependent resistor
  int light_level = mozziAnalogRead(LDR_PIN); // value is 0-1023

  // set the frequency
  aSin.setFreq(light_level);

  }


int updateAudio(){
  // https://groups.google.com/g/mozzi-users/c/PxRTWmLySnc
  return ((int)aSin.next() * volume) >> 2; // 8 bits scaled up to 14 bits

}


void loop(){
  audioHook(); // required here
}
