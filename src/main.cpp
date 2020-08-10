#include "main.h"
#include "SoundReactor.h"
#include "Patterns.h"

#include <Arduino.h>
#include <FastLED.h>

#define GLOBAL_DELAY 20

constexpr uint8_t mic_pin = 4;
constexpr uint8_t led_pin = 7;
constexpr int led_count = 15;


SoundReactorParams s_params;
LightMode current_light_mode{LightMode::Sparkle};
CRGB leds[led_count];
bool light_on{false};
Patterns *patterns{nullptr};

void setup() 
{
    pinMode(mic_pin, INPUT);

    s_params.data_pin = A0;
    s_params.leds = leds;
    s_params.leds_count = led_count;

    PatternParams pp;
    pp.led_count = led_count;
    pp.led_pin = led_pin;
    pp.leds = leds;

    patterns = new Patterns(pp);

    FastLED.addLeds<WS2812B, led_pin>(leds, led_count);

    Serial.begin(9600);
}

void loop() 
{
  
  //clap_detect(c_args);

  //lamp is turned off
  // if(!c_args.double_clap)
  // {
  //   light_off();
  //   delay(GLOBAL_DELAY);
  //   return;
  // }

  if(false)
  {
    current_light_mode = static_cast<LightMode>( (int)current_light_mode % 5 + 1);
  }  

  switch (current_light_mode)
  {
      case LightMode::White:
      {
        //de-init other modes
        if(s_params.reactor_initialized)
          deinit_reactor(s_params);
        set_light(CRGB::White);
      }
      break;
      
      case LightMode::SoundReactive:
      {
        if(!s_params.reactor_initialized)
          init_reactor(s_params);
        react_sound(s_params);
      }
      break;

      case LightMode::Fire:
      {
        if(s_params.reactor_initialized)
            deinit_reactor(s_params);
        patterns->fire(25, 50, 45);
      }
      break;

      case LightMode::FadeInOut:
      {
      if(s_params.reactor_initialized)
            deinit_reactor(s_params);
      patterns->fadeInOut(Color{233,14,56});
      }
      break;

      case LightMode::MeteorRain:
      {
        if(s_params.reactor_initialized)
            deinit_reactor(s_params);
      patterns->meteorRain(Color{233,14,56}, 10, 20, true, 15);
      }
      break;

      case LightMode::Sparkle:
      {
        if(s_params.reactor_initialized)
            deinit_reactor(s_params);
      patterns->snowSparkle(Color{50, 50, 50}, 40, 200);
      }
      break;

      default:
        break;
  }

  delay(GLOBAL_DELAY);
  
}



void clap_detect(ClapArgs& c_args)
{
  long detection_range_start = 0;
  long detection_range = 0;
  int status_sensor = digitalRead(c_args.data_pin);
  Serial.println(status_sensor);
  if (status_sensor == 0)
  {
    if (c_args.clap_count == 0)
    {
      detection_range_start = detection_range = millis();
      c_args.clap_count++;
    }
    else if (c_args.clap_count > 0 && millis() - detection_range >= c_args.clap_threshold)
    {
      detection_range = millis();
      c_args.clap_count++;
    }
  }

  if (millis() - detection_range_start >= c_args.doubleclap_threshold)
  {
    if (c_args.clap_count == 2)
    {
      c_args.double_clap = !c_args.double_clap;
    }
    c_args.clap_count = 0;
  }
}

void set_light(CRGB color) 
{
    if(!light_on)
    {
      for(int i = 0; i < led_count; ++i)
      {
        leds[i] = color;
      } 
      FastLED.show();
      light_on = true;
    }
}

void light_off() 
{
    if(light_on)
    {
      for(int i = 0; i < led_count; ++i)
      {
        leds[i] = CRGB::Black;
      } 
      FastLED.show();
      light_on = false;
    }
}