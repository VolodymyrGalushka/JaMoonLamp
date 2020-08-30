#include "SoundReactor.h"
#include <arduinoFFT.h>

#ifdef __AVR__
  #include <avr/power.h>
#endif

namespace 
{
    struct RegisterState
    {
        uint8_t adcsra;
        uint8_t admux;
        uint8_t didr0;
    };
}

RegisterState r_state;

const uint16_t samples = 64;
// const uint16_t num_pixels = 60;
const uint8_t loop_num = 1;
const uint16_t sound_thresh = 10;

arduinoFFT FFT = arduinoFFT();
double vReal[samples];
double vImag[samples];


void react_sound(SoundReactorParams& params) 
{
    // collects the samples from the ADC and computes the average
  int freq_avg = 0;
  int max_avg = 0;

  // we sample and compute the FFT a few times to reduce noise
  for (int loop_count = 0; loop_count < loop_num; loop_count++) 
  {
    double average = 0;
    for (int i = 0; i < samples; i++) 
    {
      while (!(ADCSRA & 0x10)); // wait on ADIF bit
      ADCSRA = 0b11110101; // clear ADIF bit
      vReal[i] = (double)ADC; average += ADC;
      vImag[i] = 0;
    }

    average = average / samples;
    for (int i = 0; i < samples; i++) 
    {
      vReal[i] = vReal[i] - average;
    }

    // windows the data and computes the FFT
    FFT.Windowing(vReal, samples, FFT_WIN_TYP_HAMMING, FFT_FORWARD);
    FFT.Compute(vReal, vImag, samples, FFT_FORWARD);
    FFT.ComplexToMagnitude(vReal, vImag, samples);

    // finds largest frequency component, and computes overall volume
    double max_val = 0;
    uint16_t frequency = 0;
    for (int i = 0; i < samples/2; i++) 
    {
      if (vReal[i] > max_val) 
      {
        max_val = vReal[i];
        frequency = (20000 / (samples/2)) * i;
      }
    }

    max_avg += max_val;
    freq_avg += frequency;
  }

  max_avg /= loop_num;
  freq_avg /= loop_num;
  
  const int range_max = 8000;
  if (freq_avg > range_max) 
    freq_avg = range_max;
  int x = map(freq_avg, 0, range_max, 0, 255);
  
  for (int i = 0; i < params.leds_count; i++) 
  {
    // int red = 255 - abs(2*(x-255));
    // int green = 255 - abs(2*x);
    // int blue = 255 - abs(2*(x-128));

    int red = 255 - abs(2*(x-params.color.red));
    int green = 255 - abs(2*(x-params.color.green));
    int blue = 255- abs(2*(x-params.color.blue));
      
    if (red < 0) red = 0;
    if (green < 0) green = 0;
    if (blue < 0) blue = 0;

    if (max_avg < sound_thresh)
    {
        //fill_solid( &(params.leds[i]), 1 /*number of leds*/, CRGB( 255, 68, 221) );
        //strip.setPixelColor(i, strip.Color(10, 10, 10, 255));
        params.leds[i].r = 10;
        params.leds[i].g = 10;
        params.leds[i].b = 10;
    }
    else
    {
        params.leds[i].r = red;
        params.leds[i].g = green;
        params.leds[i].b = blue;
        //fill_solid( &(params.leds[i]), 1 /*number of leds*/, CRGB( red, green, blue) );
        //strip.setPixelColor(i, CRGB(red, green, blue, 255));
    }
  }

  FastLED.show();

}

void deinit_reactor(SoundReactorParams& params) 
{
    //pop state
    ADCSRA = r_state.adcsra;
    ADMUX = r_state.admux;
    DIDR0 = r_state.didr0;
    params.reactor_initialized = false;
}

void init_reactor(SoundReactorParams& params) 
{
  //push registers
  r_state.adcsra = ADCSRA;
  r_state.admux = ADMUX;
  r_state.didr0 = DIDR0;   

//TIMSK0 = 0; // turn off timer0 for lower jitter
  ADCSRA = 0xe5; // set the adc to free running mode
  ADMUX = 0x40; // use adc0
  DIDR0 = 0x01; // turn off the digital input for adc0
  pinMode(params.data_pin, INPUT);
  params.reactor_initialized = true;
}
