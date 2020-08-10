#ifndef __MAIN_H__
#define __MAIN_H__

#include <FastLED.h>

struct ClapArgs
{
    bool    double_clap{false};
    bool    single_clap{false};
    int     clap_count{0};
    int     data_pin{};
    int     clap_threshold{50};
    int     doubleclap_threshold{400};
};

enum class LightMode
{
    White = 1,
    Color,
    SoundReactive,
    UV,
    Fire,
    FadeInOut,
    Sparkle,
    MeteorRain
};


void clap_detect(ClapArgs& args);


void set_light(CRGB color);
void light_off();


#endif // __MAIN_H__