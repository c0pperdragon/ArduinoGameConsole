#include "synth.h"
#include <avr/interrupt.h>
#include <avr/pgmspace.h>
#include "pins_arduino.h"

// designed for a ATMEGA328

// static data for the Synth library
volatile _SYNTH_VOICE Synth::voice[MAXVOICES];
uint8_t Synth::numvoices;  
uint8_t Synth::level;
uint16_t Synth::boosttable[256]; 
voidfunction_t Synth::notificationHandler;

static const uint8_t PROGMEM sine_table[] = 
{
    127,130,133,136,139,143,146,149,152,155,158,161,164,167,170,173,
    176,179,182,184,187,190,193,195,198,200,203,205,208,210,213,215,
    217,219,221,224,226,228,229,231,233,235,236,238,239,241,242,244,
    245,246,247,248,249,250,251,251,252,253,253,254,254,254,254,254,
    255,254,254,254,254,254,253,253,252,251,251,250,249,248,247,246,
    245,244,242,241,239,238,236,235,233,231,229,228,226,224,221,219,
    217,215,213,210,208,205,203,200,198,195,193,190,187,184,182,179,
    176,173,170,167,164,161,158,155,152,149,146,143,139,136,133,130,
    127,124,121,118,115,111,108,105,102, 99, 96, 93, 90, 87, 84, 81,
     78, 75, 72, 70, 67, 64, 61, 59, 56, 54, 51, 49, 46, 44, 41, 39,
     37, 35, 33, 30, 28, 26, 25, 23, 21, 19, 18, 16, 15, 13, 12, 10,
      9,  8,  7,  6,  5,  4,  3,  3,  2,  1,  1,  0,  0,  0,  0,  0,
      0,  0,  0,  0,  0,  0,  1,  1,  2,  3,  3,  4,  5,  6,  7,  8,
      9, 10, 12, 13, 15, 16, 18, 19, 21, 23, 25, 26, 28, 30, 33, 35,
     37, 39, 41, 44, 46, 49, 51, 54, 56, 59, 61, 64, 67, 70, 72, 75,
     78, 81, 84, 87, 90, 93, 96, 99,102,105,108,111,115,118,121,124
};

void donothinghandler()
{}

void Synth::init(uint8_t numvoices) 
{   
 
    uint16_t i;

    Synth::numvoices = numvoices;
    level = 128;
    notificationHandler = &donothinghandler;
    
    // start values of individual signal generation
    for (i=0; i<numvoices; i++)
    {   
        voice[i].volume = 0;
        voice[i].currentvolume = 0;
        voice[i].waveform = RECTANGLE;
        voice[i].duty = 128;
        voice[i].frequency = HERTZ(440);
        voice[i].phase = 0;
    }

    // compute boost levels
    for (i=0; i<256; i++)
    {
//        boosttable[x] = 262911 / (512-x);   // too slow
//        boosttable[x] = 199485 / (450-x);   // still too slow
//        boosttable[x] = 148335 / (400-x);     // too fast
        boosttable[i] = 179025 / (430-i);     // just right ;-)
    }  

    // -- TIMER 1 register setup
    // set to mode 7 for 10-bit counter (fast PWM)
    // will count 0 to 1023 for a 64 us period
    TCCR1A = 
      0x03    // B00000011    // WGM1:0=3
    | 0x80    // B10000000    // COM1A : set on bottom, clear at match
    | 0x30;   // B00110000 ;  // COM1B : clear on bottom, set at match
    TCCR1B = 
      0x08    // B00001000    // WGM3=1
    | 0x01;   // B00000001 ;  // clock source = full speed

    // initially turn off pulses
    OCR1AH = 3;    // prepare high byte 
    OCR1AL = 255;    // write to 16-bit register  
    OCR1BH = 3;    // prepare high byte 
    OCR1BL = 255;   // write to 16-bit register 

    // enable timer 1 overflow interrupt to start up the engine
    TIMSK1 = 0x01;  // B00000001;  

    // configure output pin directions
    DDRB |= 0x06;  // B00000110    enable timer 1 output pins
    DDRD |= 0x04;  // B00000100;   enable debug output pin   
}  

void Synth::setNotify(voidfunction_t n)
{
    notificationHandler = n;  
}


inline void Synth::processAudio()
{
    uint8_t i;

    // process individual voices 
    int8_t total = 0;
    for (i=0; i<numvoices; i++)
    {
        total += processVoice(voice[i]);
    }

    // need to boost up the level
    uint8_t newlevel = 128 + total;
    if (newlevel>level)
    {
        uint8_t delta = newlevel - level;
        if (delta>64) {delta=64;}
        uint16_t boost = boosttable[level];
        boost = (boost * delta) >> 6;
        level += delta;

        uint16_t val = 1023 - boost;
        OCR1AH = 3;             // idle pin A
        OCR1AL = 255;           
        OCR1BH = (uint8_t)(val>>8);  // prepare high byte 
        OCR1BL = (uint8_t)(val);     // write to 16-bit register 
    }
    // need to boost down the level
    else if (newlevel<level)
    {
        uint8_t delta = level - newlevel;
        if (delta>64) {delta=64;}
        uint16_t boost = boosttable[255-level];
        boost = (boost * delta) >> 6;
        level -= delta;

        uint16_t val = 1023 - boost;
        OCR1AH = (uint8_t)(val>>8);  // prepare high byte 
        OCR1AL = (uint8_t)(val);     // write to 16-bit register 
        OCR1BH = 3; 
        OCR1BL = 255;             // idle pin B        
    }
    // no change
    else
    {
        OCR1AH = 3;    // idle pin A
        OCR1AL = 255;    
        OCR1BH = 3; 
        OCR1BL = 255;  // idle pin B              
    }
}

inline int8_t Synth::processVoice(volatile _SYNTH_VOICE &v)
{
    v.phase += v.frequency;

    uint8_t vol = v.currentvolume;
    
    // slowly adjust current volume
    if (vol<v.volume) { vol++; }
    else if (vol>v.volume) { vol--; }
    v.currentvolume = vol;
  
    if (vol<=0) { return 0; }  
    
    uint8_t phi = (uint8_t) (v.phase >> 8);  // bring to range 0 - 255

    switch (v.waveform)
    {
        case RECTANGLE:  
        {   if (phi<v.duty) { return (vol>>1); }
            else { return -(vol>>1); }
        }
        case SINE:
        {   int8_t s = ((int8_t) pgm_read_byte(sine_table+phi)) - 128;
            return (int8_t) ((s*(int16_t) vol) >> 8);
        }   
        case TRIANGLE:
        {   int8_t s;
            if (phi<128) { s = 2*phi - 127; }  
            else         { s = 2*(255-phi) - 127; }
            return (int8_t) ((s*(int16_t) vol) >> 8);
        }    
        case SAWTOOTH:   
        {   int8_t s = phi-127;
            return (int8_t) ((s*(int16_t) vol) >> 8);
        }    
        default:       // silence
        {   return 0; 
        }
    }
}

// interrupt routine every 64 microseconds (15.625 kHz)
ISR(TIMER1_OVF_vect)
{
    PORTD = 0xfb;  // B11111011;  // debug output for time measurement

    Synth::notificationHandler();
    if (Synth::numvoices>0)
    {   Synth::processAudio();
    }
    
    PORTD = 0xff;  // B11111111;    // debug output for time measurement
}
