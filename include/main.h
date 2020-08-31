#ifndef __MAIN_H__
#define __MAIN_H__

#include <FastLED.h>

struct ValueSet
{
    int v1;
    int v2;
    int v3;
};

enum class LightMode
{
    White = 1,
    Color,
    UV,
    Fire,
    Sparkle,
    MeteorRain,
    SoundReactive,
    FadeInOut
};


void switch_detect();
void switch_isr();
void read_adjustings();
void uv_toggle(bool on);

void set_light(CRGB color);
void light_off();

void save_mode();
void load_mode();

void save_user_light();
void load_user_light();


#endif // __MAIN_H__