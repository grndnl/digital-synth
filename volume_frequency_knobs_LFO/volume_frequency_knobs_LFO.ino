// TODO: 
// - implement v/oct on the VCO
// - knobs: root, octave, scale, range, clock
// - button: random


#include <MozziGuts.h>
#include <Oscil.h> // oscillator template
#include <tables/sin2048_int8.h> // sine table for oscillator
#include <tables/square_no_alias_2048_int8.h>
#include <tables/saw2048_int8.h>
#include <tables/triangle2048_int8.h>
#include <IntMap.h>
#include <AutoMap.h>
#include <LowPassFilter.h>

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
const char VCO_fm_pin = 3; 
const char VCO_waveform_pin = 1;
byte VCO_waveform;
const int quantize_pin = 7;
bool quantize;
IntMap tone_index_map(0, 1023, 0, 59);



//LFO
//Oscil <SIN2048_NUM_CELLS, CONTROL_RATE> kLFO(SIN2048_DATA);
Oscil <2048, CONTROL_RATE> kLFO;
IntMap LFO_map(0, 1023, 0, 2000);
const byte LFO_waveform_pin = A0;
byte waveform = 3;
AutoMap wavetable_choice_map(0, 1023, 0, 3);

// LPF
LowPassFilter LPF;
uint8_t resonance = 255;


void set_LFO_wavetable(byte n) {
  if (n==0) {kLFO.setTable(SIN2048_DATA);}
  else if (n==1) {kLFO.setTable(TRIANGLE2048_DATA);}
  else if (n==2) {kLFO.setTable(SAW2048_DATA);}
  else {kLFO.setTable(SQUARE_NO_ALIAS_2048_DATA);}
}

void set_VCO_wavetable(byte n) {
  if (n==0) {aSin.setTable(SIN2048_DATA);}
  else if (n==1) {aSin.setTable(TRIANGLE2048_DATA);}
  else if (n==2) {aSin.setTable(SAW2048_DATA);}
  else {aSin.setTable(SQUARE_NO_ALIAS_2048_DATA);}
}

//Quantized frequencies array from A1 to A7
float note_freq[60] = {55.00, 58.27, 61.74, 65.41, 69.30, 73.42, 77.78, 82.41, 87.31, 92.50, 98.00, 103.83, 
                       110.00, 116.54, 123.47, 130.81, 138.59, 146.83, 155.56, 164.81, 174.61, 185.00, 196.00, 207.65, 
                       220.00, 233.08, 246.94, 261.63, 277.18, 293.66, 311.13, 329.63, 349.23, 369.99, 392.00, 415.30, 
                       440.00, 466.16, 493.88, 523.25, 554.37, 587.33, 622.25, 659.25, 698.46, 739.99, 783.99, 830.61, 
                       880.00, 932.33, 987.77, 1046.50, 1108.73, 1174.66, 1244.51, 1318.51, 1396.91, 1479.98, 1567.98, 1661.22};



void setup(){
  startMozzi(CONTROL_RATE); // uses the default control rate of 64, defined in mozzi_config.h
  LPF.setResonance(resonance);
  pinMode(quantize_pin, INPUT);
}


void updateControl(){
    // set the volume
  int level_value = mozziAnalogRead(LEVEL_PIN); // value is 0-1023
  // map it to an 8 bit volume range for efficient calculations in updateAudio
  volume = level_value >> 2;  // 10 bits (0->1023) shifted right by 2 bits to give 8 bits (0->255)

  // set the LFO
  float LFO_freq_value = LFO_map(mozziAnalogRead(VCO_fm_pin))/100.0;
  // read the VCO FM CV pot
  VCO_fm_cv = VCO_fm_cv_map(mozziAnalogRead(VCO_fm_cv_pin))/100.0;
  //set the frequency kLFO
  kLFO.setFreq(LFO_freq_value);
  waveform = wavetable_choice_map(mozziAnalogRead(LFO_waveform_pin));
  set_LFO_wavetable(waveform);
  float vibrato = VCO_fm_cv * kLFO.next();
  
  // set the VCO
  int VCO_frequency = mozziAnalogRead(VCO_FREQ_PIN); // value is 0-1023
  quantize = digitalRead(quantize_pin);
  if (quantize) { aSin.setFreq(note_freq[tone_index_map(VCO_frequency+ vibrato)] ); }
  else { aSin.setFreq(VCO_frequency + vibrato); }
  VCO_waveform = wavetable_choice_map(mozziAnalogRead(VCO_waveform_pin));
  set_VCO_wavetable(VCO_waveform);


  // LPF
  LPF.setCutoffFreq(250);

  }


int updateAudio(){
  int audio;
  if (VCO_waveform !=3 or VCO_waveform != 4) { audio = (int)(aSin.next() * volume) >> 2; } // 8 bits scaled up to 14 bits // https://groups.google.com/g/mozzi-users/c/PxRTWmLySnc
  else {(int)(aSin.next() * volume) >> 4; }
//  return LPF.next(audio);
  return audio;

}


void loop(){
  audioHook(); // required here
}
