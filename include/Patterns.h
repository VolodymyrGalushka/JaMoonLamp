#ifndef __PATTERNS_H__
#define __PATTERNS_H__


#include "global.h"

#include <Arduino.h>
#include <FastLED.h>


struct PatternParams
{
    CRGB        *leds;
    int         led_count;
    uint8_t     led_pin;
};


class Patterns
{
public:
    Patterns(PatternParams pp);
    ~Patterns();

    void    fadeInOut(Color c);
    void    snowSparkle(Color c, int SparkleDelay, int SpeedDelay);
    void    meteorRain(Color c, byte meteorSize, byte meteorTrailDecay, bool meteorRandomDecay, int SpeedDelay);
    void    fire(int Cooling, int Sparking, int SpeedDelay);

private:
    void    setPixel(int Pixel, Color c);
    void    setAll(Color c);
    void    showStrip();
    void    setPixelHeatColor (int Pixel, byte temperature);
    void    fadeToBlack(int ledNo, byte fadeValue); 

    PatternParams   m_patternParams;
    byte*           m_heat;
};



#endif // __PATTERNS_H__