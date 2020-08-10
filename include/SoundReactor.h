#ifndef __SOUNDREACTOR_H__
#define __SOUNDREACTOR_H__

#include <FastLED.h>

struct SoundReactorParams
{
    CRGB        *leds;
    int         leds_count;
    uint8_t     data_pin;
    bool        reactor_initialized{false};
};


void  init_reactor(SoundReactorParams& params);
void  react_sound(SoundReactorParams& params);  
void  deinit_reactor(SoundReactorParams& params);

#endif // __SOUNDREACTOR_H__