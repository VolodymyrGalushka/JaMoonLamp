#include "main.h"
#include "SoundReactor.h"
#include "Patterns.h"

#include <Arduino.h>
#include <FastLED.h>

#define GLOBAL_DELAY 20

constexpr uint8_t led_pin = 7;
constexpr uint8_t switch_pin = 3;
constexpr uint8_t v1_pin = A5;
constexpr uint8_t v2_pin = A6;
constexpr uint8_t v3_pin = A7;

constexpr int led_count = 15;
constexpr int modes_count = 8;


SoundReactorParams s_params;
LightMode current_light_mode{LightMode::White};
CRGB leds[led_count];
bool light_on{false};
Patterns *patterns{nullptr};
Color user_light;
ValueSet adjust_values{0,0,0};
bool switch_mode{false};
uint64_t lastDebounceTime = 0;  // the last time the output pin was toggled
uint64_t debounceDelay = 50;    // the debounce time; increase if the output flickers

void setup() 
{
    pinMode(switch_pin, INPUT);
    attachInterrupt(digitalPinToInterrupt(switch_pin), switch_isr, CHANGE);

    pinMode(v1_pin, INPUT);
    pinMode(v2_pin, INPUT);
    pinMode(v3_pin, INPUT);

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

  // switch_detect();

  if(!s_params.reactor_initialized)
  {
    read_adjustings();
  }
 


  //Serial.println("Current mode: ");
  //Serial.println((int)current_light_mode);

  if(switch_mode)
  {
    current_light_mode = static_cast<LightMode>( (int)current_light_mode % modes_count + 1);
    //TODO: save into EEPROM
    switch_mode = false;
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
      
      case LightMode::Color:
      {
        if(s_params.reactor_initialized)
          deinit_reactor(s_params);
        user_light.red = map(adjust_values.v1, 0, 1024, 0, 255);
        user_light.green = map(adjust_values.v2, 0, 1024, 0, 255);
        user_light.blue = map(adjust_values.v3, 0, 1024, 0, 255);
        CRGB c;
        c.red = user_light.red;
        c.green = user_light.green;
        c.blue = user_light.blue;
        set_light(c);
      }

      case LightMode::Fire:
      {
        if(s_params.reactor_initialized)
            deinit_reactor(s_params);
        patterns->fire(adjust_values.v1, adjust_values.v2, adjust_values.v3);
      }
      break;

      case LightMode::FadeInOut:
      {
      if(s_params.reactor_initialized)
            deinit_reactor(s_params);
      patterns->fadeInOut(user_light);
      }
      break;

      case LightMode::MeteorRain:
      {
        if(s_params.reactor_initialized)
            deinit_reactor(s_params);
        auto m_size = map(adjust_values.v1, 0, 1024, 0, 255);
        auto m_trail = map(adjust_values.v2, 0, 1024, 0, 255);
        auto speed_delay = map(adjust_values.v3, 0, 1024, 0, 255);
        patterns->meteorRain(user_light, m_size, m_trail, true, speed_delay);
      }
      break;

      case LightMode::Sparkle:
      {
        if(s_params.reactor_initialized)
            deinit_reactor(s_params);
      patterns->snowSparkle(user_light, adjust_values.v1, adjust_values.v2);
      }
      break;

      case LightMode::UV:
      {

      }
      break;

    
      case LightMode::SoundReactive:
      {
        if(!s_params.reactor_initialized)
          init_reactor(s_params);
        react_sound(s_params);
      }
      break;

      default:
        break;
  }

  delay(GLOBAL_DELAY);
  
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

//TODO: add debounce
void switch_detect() 
{
   static int last_switch_state = LOW;
   static int switch_state;
   auto reading = digitalRead(switch_pin);

  // If the switch changed, due to noise or pressing:
  if (reading != last_switch_state) 
  {
    // reset the debouncing timer
    lastDebounceTime = millis();
  }

  if ((millis() - lastDebounceTime) > debounceDelay) 
  {
      if (reading != switch_state) 
      {
        switch_state = reading;

      // only toggle the LED if the new button state is HIGH
      if (switch_state == HIGH) 
      {
        switch_mode = true;
      }
    }
  }

   last_switch_state = reading;
}


void switch_isr()
{
  if ((millis() - lastDebounceTime) > debounceDelay) 
    switch_mode = true;
  lastDebounceTime = millis();
}

void read_adjustings() 
{
    adjust_values.v1 = analogRead(v1_pin);
    adjust_values.v2 = analogRead(v2_pin);
    adjust_values.v3 = analogRead(v3_pin);
}
