
#include <MozziGuts.h>
#include <Oscil.h> // oscillator template
#include <tables/sin2048_int8.h> // sine table for oscillator
#include <IntMap.h>

#define CONTROL_RATE 128

//MIXER
const char LEVEL_PIN = 5; 
byte volume;

//VCO
const char VCO_FREQ_PIN = 4; 
Oscil <SIN2048_NUM_CELLS, AUDIO_RATE> aSin(SIN2048_DATA);
float VCO_fm_cv;
const char VCO_fm_cv_pin = 2;
IntMap VCO_fm_cv_map(0, 1023, 0, 1000);

//LFO
const char LFO_FREQ_PIN = 3; 
Oscil <SIN2048_NUM_CELLS, CONTROL_RATE> kLFO(SIN2048_DATA);
IntMap LFO_map(0, 1023, 0, 2000);



void setup(){
  startMozzi(CONTROL_RATE); // uses the default control rate of 64, defined in mozzi_config.h
}


void updateControl(){
    // read the potentiometer
  int level_value = mozziAnalogRead(LEVEL_PIN); // value is 0-1023
  // map it to an 8 bit volume range for efficient calculations in updateAudio
  volume = level_value >> 2;  // 10 bits (0->1023) shifted right by 2 bits to give 8 bits (0->255)

  // read the LFO pot
  float LFO_freq_value = LFO_map(mozziAnalogRead(LFO_FREQ_PIN))/100.0;
  // read the VCO FM CV pot
  VCO_fm_cv = VCO_fm_cv_map(mozziAnalogRead(VCO_fm_cv_pin))/100.0;
  //set the frequency kLFO
  kLFO.setFreq(LFO_freq_value);
  float vibrato = VCO_fm_cv * kLFO.next();
  
  // read the light dependent resistor
  int VCO_frequency = mozziAnalogRead(VCO_FREQ_PIN); // value is 0-1023
  // set the frequency
  aSin.setFreq(VCO_frequency + vibrato);

  }


int updateAudio(){
  // https://groups.google.com/g/mozzi-users/c/PxRTWmLySnc
  return ((int)aSin.next() * volume) >> 2; // 8 bits scaled up to 14 bits

}


void loop(){
  audioHook(); // required here
}
