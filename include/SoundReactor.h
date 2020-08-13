#ifndef __SOUNDREACTOR_H__
#define __SOUNDREACTOR_H__

#include "global.h"

#include <FastLED.h>

// struct Color
// {
//     byte red;
//     byte green;
//     byte blue;
// };

struct SoundReactorParams
{
    CRGB        *leds;
    int         leds_count;
    uint8_t     data_pin;
    bool        reactor_initialized{false};
    Color       color;
    uint16_t    sound_thresh;
};


void  init_reactor(SoundReactorParams& params);
void  react_sound(SoundReactorParams& params);  
void  deinit_reactor(SoundReactorParams& params);

#endif // __SOUNDREACTOR_H__