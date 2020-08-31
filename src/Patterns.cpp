#include "Patterns.h"


Patterns::Patterns(PatternParams pp) : m_patternParams{pp}
{
    m_heat = new byte[m_patternParams.led_count];
}


Patterns::~Patterns() 
{
    delete [] m_heat;   
}

//----------------------------------------------------------------------------------------------------------
void Patterns::fadeInOut(Color c)
{
  float r, g, b;
     
  for(int k = 0; k < 256; k=k+1) 
  {
    r = (k/256.0)*c.red;
    g = (k/256.0)*c.green;
    b = (k/256.0)*c.blue;
    this->setAll(Color{r,g,b});
    this->showStrip();
  }
     
  for(int k = 255; k >= 0; k=k-2) 
  {
    r = (k/256.0)*c.red;
    g = (k/256.0)*c.green;
    b = (k/256.0)*c.blue;
    this->setAll(Color{r,g,b});
    this->showStrip();
  }
}

//----------------------------------------------------------------------------------------------------------
void Patterns::snowSparkle(Color c, int SparkleDelay, int SpeedDelay) 
{
  this->setAll(c);
 
  int Pixel = random(this->m_patternParams.led_count);
  this->setPixel(Pixel, Color{0xff,0xff,0xff});
  this->showStrip();
  delay(SparkleDelay);
  this->setPixel(Pixel, c);
  this->showStrip();
  delay(SpeedDelay);
}


//----------------------------------------------------------------------------------------------------------
void Patterns::meteorRain(Color c, byte meteorSize, byte meteorTrailDecay, bool meteorRandomDecay, int SpeedDelay) 
{  
  this->setAll(Color{0,0,0});
 
  for(int i = 0; i < 2*this->m_patternParams.led_count; i++) 
  {
   
   
    // fade brightness all LEDs one step
    for(int j=0; j<this->m_patternParams.led_count; j++) 
    {
      if( (!meteorRandomDecay) || (random(10)>5) ) 
      {
        this->fadeToBlack(j, meteorTrailDecay );        
      }
    }
   
    // draw meteor
    for(int j = 0; j < meteorSize; j++) 
    {
      if( ( i-j <this->m_patternParams.led_count) && (i-j>=0) ) 
      {
        this->setPixel(i-j, c);
      }
    }
   
    this->showStrip();
    delay(SpeedDelay);
  }
}


//----------------------------------------------------------------------------------------------------------
void Patterns::fadeToBlack(int ledNo, byte fadeValue) 
{
 #ifdef ADAFRUIT_NEOPIXEL_H
    // NeoPixel
    uint32_t oldColor;
    uint8_t r, g, b;
    int value;
   
    oldColor = strip.getPixelColor(ledNo);
    r = (oldColor & 0x00ff0000UL) >> 16;
    g = (oldColor & 0x0000ff00UL) >> 8;
    b = (oldColor & 0x000000ffUL);

    r=(r<=10)? 0 : (int) r-(r*fadeValue/256);
    g=(g<=10)? 0 : (int) g-(g*fadeValue/256);
    b=(b<=10)? 0 : (int) b-(b*fadeValue/256);
   
    strip.setPixelColor(ledNo, r,g,b);
 #endif
 #ifndef ADAFRUIT_NEOPIXEL_H
   // FastLED
   this->m_patternParams.leds[ledNo].fadeToBlackBy( fadeValue );
 #endif  
}

//----------------------------------------------------------------------------------------------------------
void Patterns::fire(int Cooling, int Sparking, int SpeedDelay) 
{
  //static byte heat[this->m_patternParams.led_count];
  int cooldown;
  
  // Step 1.  Cool down every cell a little
  for( int i = 0; i < this->m_patternParams.led_count; i++) 
  {
    cooldown = random(0, ((Cooling * 10) / this->m_patternParams.led_count) + 2);
    
    if(cooldown > m_heat[i]) 
    {
      m_heat[i]=0;
    } else 
    {
      m_heat[i]=m_heat[i]-cooldown;
    }
  }
  
  // Step 2.  Heat from each cell drifts 'up' and diffuses a little
  for( int k= this->m_patternParams.led_count - 1; k >= 2; k--) 
  {
    m_heat[k] = (m_heat[k - 1] + m_heat[k - 2] + m_heat[k - 2]) / 3;
  }
    
  // Step 3.  Randomly ignite new 'sparks' near the bottom
  if( random(255) < Sparking ) 
  {
    int y = random(7);
    m_heat[y] = m_heat[y] + random(160,255);
    //heat[y] = random(160,255);
  }

  // Step 4.  Convert heat to LED colors
  for( int j = 0; j < this->m_patternParams.led_count; j++) 
  {
    this->setPixelHeatColor(j, m_heat[j] );
  }

  this->showStrip();
  delay(SpeedDelay);
}


//----------------------------------------------------------------------------------------------------------
void Patterns::setPixelHeatColor (int Pixel, byte temperature) 
{
  // Scale 'heat' down from 0-255 to 0-191
  byte t192 = round((temperature/255.0)*191);
 
  // calculate ramp up from
  byte heatramp = t192 & 0x3F; // 0..63
  heatramp <<= 2; // scale up to 0..252
 
  // figure out which third of the spectrum we're in:
  if( t192 > 0x80) 
  {                     // hottest
    this->setPixel(Pixel, Color{255, 255, heatramp});
  } 
  else if( t192 > 0x40 ) 
  {             // middle
    this->setPixel(Pixel, Color{heatramp, 255 , 0});
  } 
  else 
  {                               // coolest
    this->setPixel(Pixel, Color{0, heatramp, 0});
  }
}

//----------------------------------------------------------------------------------------------------------
void Patterns::showStrip() 
{
 #ifdef ADAFRUIT_NEOPIXEL_H 
   // NeoPixel
   strip.show();
 #endif
 #ifndef ADAFRUIT_NEOPIXEL_H
   // FastLED
   FastLED.show();
 #endif
}


//----------------------------------------------------------------------------------------------------------
void Patterns::setPixel(int Pixel, Color c) 
{
 #ifdef ADAFRUIT_NEOPIXEL_H 
   // NeoPixel
   strip.setPixelColor(Pixel, strip.Color(red, green, blue));
 #endif
 #ifndef ADAFRUIT_NEOPIXEL_H 
   // FastLED
   this->m_patternParams.leds[Pixel].r = c.red;
   this->m_patternParams.leds[Pixel].g = c.green;
   this->m_patternParams.leds[Pixel].b = c.blue;
 #endif
}


//----------------------------------------------------------------------------------------------------------
void Patterns::setAll(Color c) 
{
  for(int i = 0; i < this->m_patternParams.led_count; i++ ) 
  {
    this->setPixel(i, c); 
  }
  this->showStrip();
}
