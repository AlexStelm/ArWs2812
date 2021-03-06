#include <Arduino.h>
#include "ExtNeoPixel.h"
#include "color.h"
#include "palette.h"
#include "anim.h"
#include "brightness.h"

Anim::Anim() 
{


    pixels.begin();
    pixels.show(); // turn of all LEDs

    nextms = millis();
}

void Anim::setPeriod(byte period) {
    this->period = period;
}

void Anim::setPalette(Palette * pal) {
    this->palette = pal;
    if (setUpOnPalChange) {
        setUp();
    }
}

void Anim::run()
{    
    if ( millis()<=nextms) {
        digitalWrite(LED_BUILTIN, HIGH);
        return;
    }
    digitalWrite(LED_BUILTIN, LOW);
    nextms=millis() + period;
    
    if (runImpl != NULL)
    {
        (this->*runImpl)();
    }

    //transition coef, if within 0..1 - transition is active
    //changes from 1 to 0 during transition, so we interpolate from current color to previous
    float transc = (float)((long)transms - (long)millis()) / TRANSITION_MS;
    Color * leds_prev = (leds == leds1) ? leds2 : leds1;
    
    if (transc > 0) {
        for(int i=0; i<LEDS; i++) {
            //transition is in progress
            Color c = leds[i].interpolate(leds_prev[i], transc);
            //pixels.setPixelColor(i, pixels.Color(c.r, c.g, c.b));
            byte r = pgm_read_byte_near(BRI + c.r);
            byte g = pgm_read_byte_near(BRI + c.g);
            byte b = pgm_read_byte_near(BRI + c.b);
            pixels.setPixelColor(i, pixels.Color(r, g, b));
        }
    } else {
        for(int i=0; i<LEDS; i++) {
            //regular operation
            //pixels.setPixelColor(i, pixels.Color(leds[i].r, leds[i].g, leds[i].b));
            byte r = pgm_read_byte_near(BRI + leds[i].r);
            byte g = pgm_read_byte_near(BRI + leds[i].g);
            byte b = pgm_read_byte_near(BRI + leds[i].b);
            pixels.setPixelColor(i, pixels.Color(r, g, b));
        }
    }
  
    pixels.show();
    digitalWrite(LED_BUILTIN, HIGH);
    
}

void Anim::setUp()
{
    pinMode(LED_BUILTIN, OUTPUT);  
    transms = millis() + TRANSITION_MS;

    //switch operation buffers (for transition to operate)
    
    if (leds == leds1) {
        leds = leds2;
    } else {
        leds = leds1;
    }

    if (setUpImpl != NULL) {
        (this->*setUpImpl)();
    }
}

void Anim::doSetUp()
{
    if (!setUpOnPalChange) {
        setUp();
    }
}

void Anim::setAnim(byte animInd)
{
    switch (animInd) {
        case 0: 
            setUpImpl = &Anim::animRun_SetUp;
            runImpl = &Anim::animRun_Run;
            setUpOnPalChange = true;
        break;
        case 1: 
            setUpImpl = &Anim::animPixieDust_SetUp;
            runImpl = &Anim::animPixieDust_Run;
            setUpOnPalChange = true;
        break;        
        case 2: 
            setUpImpl = &Anim::animSparkr_SetUp;
            runImpl = &Anim::animSparkr_Run;
            setUpOnPalChange = true;
        break;        
        case 3: 
            setUpImpl = &Anim::animRandCyc_SetUp;
            runImpl = &Anim::animRandCyc_Run;
            setUpOnPalChange = true;
        break;   
        case 4: 
            setUpImpl = &Anim::animStars_SetUp;
            runImpl = &Anim::animStars_Run;
            setUpOnPalChange = false;
        break;    
        case 5: 
            setUpImpl = &Anim::animSpread_SetUp;
            runImpl = &Anim::animSpread_Run;
            setUpOnPalChange = false;
        break;     
        case 6: 
            setUpImpl = &Anim::animFly_SetUp;
            runImpl = &Anim::animFly_Run;
            setUpOnPalChange = false;
        break;                                
        default:
            setUpImpl = &Anim::animStart_SetUp;
            runImpl = &Anim::animStart_Run;
            setUpOnPalChange = true;
        break;
    }
}



unsigned int rng() {
    static unsigned int y = 0;
    y += micros(); // seeded with changing number
    y ^= y << 2; y ^= y >> 7; y ^= y << 7;
    return (y);
}

byte rngb() {
    return (byte)rng();
}


Adafruit_NeoPixel Anim::pixels = Adafruit_NeoPixel(LEDS, PIN, NEO_GRB + NEO_KHZ800); 
Color Anim::leds1[LEDS];
Color Anim::leds2[LEDS];
Color Anim::ledstmp[LEDS];
byte Anim::seq[LEDS];
