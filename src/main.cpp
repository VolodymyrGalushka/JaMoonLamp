#include "main.h"
#include "SoundReactor.h"
#include "Patterns.h"

#include <Arduino.h>
#include <FastLED.h>
#include <EEPROM.h>

#define GLOBAL_DELAY 20

constexpr uint8_t led_pin = 7;
constexpr uint8_t switch_pin = 3;
constexpr uint8_t transistor_pin = 10;
constexpr uint8_t v1_pin = A2;
constexpr uint8_t v2_pin = A4;
constexpr uint8_t v3_pin = A6;


constexpr int led_count = 15;
constexpr int modes_count = 8;

constexpr int light_mode_addr{0x00};

SoundReactorParams    s_params;
LightMode             current_light_mode{LightMode::White};
CRGB                  leds[led_count];
bool                  light_on{false};
Patterns*             patterns{nullptr};
Color                 user_light;
ValueSet              adjust_values{0,0,0};

bool                  switch_mode{false};
uint64_t              last_switch_time{0};
uint64_t              save_mode_delay{1000*30};
bool                  mode_saved{true};

int                   last_switch_state = HIGH;
int                   switch_state;
uint64_t              lastDebounceTime = 0;  // the last time the output pin was toggled
uint64_t              debounceDelay = 100;    // the debounce time; increase if the output flickers


void setup() 
{
    pinMode(switch_pin, INPUT);
    // attachInterrupt(digitalPinToInterrupt(switch_pin), switch_isr, FALLING);

    pinMode(v1_pin, INPUT);
    pinMode(v2_pin, INPUT);
    pinMode(v3_pin, INPUT);

    pinMode(transistor_pin, OUTPUT);

    s_params.data_pin = A0;
    s_params.leds = leds;
    s_params.leds_count = led_count;

    PatternParams pp;
    pp.led_count = led_count;
    pp.led_pin = led_pin;
    pp.leds = leds;

    patterns = new Patterns(pp);

    FastLED.addLeds<WS2812B, led_pin>(leds, led_count);

    load_mode();

    Serial.begin(9600);
}

void loop() 
{

  switch_detect();

  if(!s_params.reactor_initialized)
  {
    read_adjustings();
  }
 

  Serial.println("Current mode: ");
  Serial.println((int)current_light_mode);

  if(switch_mode)
  {
    current_light_mode = static_cast<LightMode>( (int)current_light_mode % modes_count + 1);
    last_switch_time = millis();
    mode_saved = false;
    switch_mode = false;
    Serial.print("Current mode: ");
    Serial.print(int(current_light_mode));
  }

  if(!mode_saved)
  {
      if((millis() - last_switch_time) > save_mode_delay)
      {
          save_mode();
      }
  }

  switch (current_light_mode)
  {
      case LightMode::White:
      {
        //de-init other modes
        if(s_params.reactor_initialized)
          deinit_reactor(s_params);
        uv_toggle(false);
        //warm white
        CRGB c;
        c.green = 244; // red
        c.red = 164; // green
        c.blue = 96;
        set_light(c);
      }
      break;
      
      case LightMode::Color:
      {
        if(s_params.reactor_initialized)
          deinit_reactor(s_params);
        uv_toggle(false);

        user_light.red = map(adjust_values.v1, 0, 1024, 0, 255);
        user_light.green = map(adjust_values.v2, 0, 1024, 0, 255);
        user_light.blue = map(adjust_values.v3, 0, 1024, 0, 255);
        CRGB c;
        c.red = user_light.red;
        c.green = user_light.green;
        c.blue = user_light.blue;
        set_light(c);
      }
      break;

      case LightMode::Fire:
      {
        if(s_params.reactor_initialized)
            deinit_reactor(s_params);
        uv_toggle(false);
        patterns->fire(adjust_values.v1, adjust_values.v2, adjust_values.v3);
      }
      break;

      case LightMode::FadeInOut:
      {
      if(s_params.reactor_initialized)
            deinit_reactor(s_params);
      uv_toggle(false);
      patterns->fadeInOut(user_light);
      }
      break;

      case LightMode::MeteorRain:
      {
        if(s_params.reactor_initialized)
            deinit_reactor(s_params);
        uv_toggle(false);
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
        uv_toggle(false);
        patterns->snowSparkle(user_light, adjust_values.v1, adjust_values.v2);
      }
      break;

      case LightMode::UV:
      {
          if(s_params.reactor_initialized)
            deinit_reactor(s_params);
          light_off();
          uv_toggle(true);
      }
      break;

    
      case LightMode::SoundReactive:
      {
        s_params.color = user_light;
        if(!s_params.reactor_initialized)
          init_reactor(s_params);
        uv_toggle(false);
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
    static CRGB last_c;

    if(!light_on || (last_c.red != color.red) || last_c.blue != color.blue || last_c.green != color.green)
    {
      for(int i = 0; i < led_count; ++i)
      {
        leds[i] = color;
      } 
      FastLED.show();
      light_on = true;
      last_c = color;
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

void save_mode() 
{
    EEPROM.put(light_mode_addr, current_light_mode);
    mode_saved = true;
}

void load_mode() 
{
    LightMode mode{LightMode::White};
    EEPROM.get(light_mode_addr, mode);
    auto mode_val = static_cast<int>(mode);
    if(mode_val >= 1 && mode_val <= 8)
      current_light_mode = mode;
}


void switch_detect() 
{
  auto reading = digitalRead(switch_pin);

  // Serial.print("Reading: " );
  // Serial.print(reading);
  // Serial.println();

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
      if (switch_state == LOW) 
      {
        switch_mode = true;
      }
    }
  }

   last_switch_state = reading;
}


void switch_isr()
{
  auto reading = digitalRead(switch_pin);

  Serial.print("Reading: " );
  Serial.print(reading);
  Serial.println();
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

void uv_toggle(bool on) 
{
  if(on)
  {
    digitalWrite(transistor_pin, HIGH);
  }
  else
  {
    digitalWrite(transistor_pin, LOW);
  }
}
