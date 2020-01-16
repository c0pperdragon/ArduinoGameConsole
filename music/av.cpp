#include "av.h"
#include <avr/interrupt.h>
#include <avr/pgmspace.h>
#include "pins_arduino.h"

// designed for a ATMEGA328

_AV_VOICE voiceA;
_AV_VOICE voiceB;
_AV_VOICE voiceC;
_AV_VOICE voiceD;

renderfunction_t av_renderfunction;

uint8_t av_level;             // current output voltage level
uint8_t vcounter;
volatile bool isblanking;



static const uint8_t PROGMEM waveDummy[] = 
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

         
inline void initVoice(_AV_VOICE &v)
{
    v.volume = 0;
    v.currentvolume = 0;
    v.waveform = waveDummy;
    v.duty = 128;
    v.frequency = HERTZ(440);
    v.phase = 0;
    v.volume = 0;
}
void av_init(renderfunction_t renderfunction) 
{   
    av_renderfunction = renderfunction;
    
    av_level = 128;
    
    isblanking = true;
    vcounter = 0;

    // start values of individual signal generation
    initVoice(voiceA);
    initVoice(voiceB);
    initVoice(voiceC);
    initVoice(voiceD);

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

    // -- TIMER0 register setup
    // set to mode 7 (fast PWM counts from 0 to OCRA)
    TCCR0A = 
      0x03    // B00000011    // WGM1:0=3
    | 0x00    // B00000000    // COM2A : not used
    | 0x30;   // B00110000 ;  // COM2B : clear on bottom, set at match
    TCCR0B = 
      0x08    // B00001000    // WGM3=1
    | 0x02;   // B00000010 ;  // clock source = 1/8 prescaler
      
    OCR0A = 127; // count sequence 0 to 127 
    OCR0B = 8;   // set output high again after 4.5 us 

    // set timers to well-defined initial values to force timer to run in harmonic operation from now on
    GTCCR = 0x02;  // clear timer 2 prescaler
    TCNT1H = 0;    // prepare high-byte for timer1 conter
    TCNT1L = 0;    // clear timer1 counter
    TCNT0 = 25;    // set timer0 counter to specified start value
       
    // enable timer 1 overflow interrupt at the next line start
    TIMSK1 = 0x01;  // B00000001;     
    // disable all other interrupts
    TIMSK0 = 0x00;
    TIMSK2 = 0x00;

    // configure output pin directions
    DDRB |= 0x07;  // B00000111    enable timer 1 output pins and debug signal
    DDRD |= 0xA8;  // B10101000;   enable video output pins 
}  

void av_waitForBlanking()
{
  while (isblanking) { } 
  while (!isblanking) { } 
}

#define PROCESSVOICE(v)                                          \
{                                                                \
    v.phase += v.frequency;                                      \
                                                                 \
    uint8_t vol = v.currentvolume;                               \    
    if (vol<v.volume) { vol++; }                                 \
    else if (vol>v.volume) { vol--; }                            \
    v.currentvolume = vol;                                       \
                                                                 \
    uint8_t phi = (uint8_t) (v.phase >> 8);                      \
    int8_t s = ((int8_t) pgm_read_byte(v.waveform+phi)) - 128;   \
    total += (int8_t) ((s*(int16_t) vol) >> 8);                  \
}


inline void processAudio()
{
    // process individual voices 
    uint8_t total = 128;
    PROCESSVOICE(voiceA)
    PROCESSVOICE(voiceB)
    PROCESSVOICE(voiceC)
    PROCESSVOICE(voiceD)

    // need to boost up the level
    if (total>av_level)
    {
        uint8_t delta = total - av_level;
        if (delta>63) {delta=63;}
        av_level += delta;

        uint16_t val = 1023 - 16 * (uint16_t) delta;
        OCR1AH = 3;             // idle pin A
        OCR1AL = 255;           
        OCR1BH = (uint8_t)(val>>8);  // prepare high byte 
        OCR1BL = (uint8_t)(val);     // write to 16-bit register 
    }
    // need to boost down the level
    else if (total<av_level)
    {
        uint8_t delta = av_level - total;
        if (delta>63) {delta=63;}
        av_level -= delta;

        uint16_t val = 1023 - 16 * (uint16_t) delta;
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

inline void adjustSync(uint8_t vcounter)
{
    const uint8_t vtrigger = 13;
    
    // switch from normal horizontal sync to 32 us pulse intervals
    if (vcounter==vtrigger-1) 
    {   OCR0A = 63; // count sequence 0 to 63 
                    // do not change pulse duration
    }
    // switch to short pulses
    else if (vcounter==vtrigger+0)
    {
        OCR0B = 3;   // set output high again after 2 us 
    }
    // switch to vsync pulses 
    else if (vcounter==vtrigger+2)
    {
        // delay until counter is beyond the next reset point
        uint8_t c = TCNT0;
        while (TCNT0 >= c) {} 
        OCR0B = 60;   // set output high again after 30 us 
    }
    // switch to short pulses 
    else if (vcounter==vtrigger+5)
    {
        OCR0B = 3;   // set output high again after 2 us 
    }
    // switch to normal hsync pulses 
    else if (vcounter==vtrigger+7)
    {
        // delay until counter is beyond the next reset point
        uint8_t c = TCNT0;
        while (TCNT0 >= c) {} 
        OCR0A = 127; // count sequence 0 to 127
        OCR0B = 8;   // set output high again after 4.5 us 
    }  
}

inline void processVideo()
{
      // progress line counter and trigger appropriate render or sync adjustment action
    if (!isblanking)
    {
        av_renderfunction(vcounter);
        if (vcounter<255)
        {   vcounter++;
        }
        else
        {   vcounter = 0;
            isblanking = true;
        }
    }
    else
    {
        adjustSync(vcounter);
        if (vcounter<55)
        {   vcounter++;
        }
        else
        {
            vcounter = 0;
            isblanking = false;
        }
    }
}


// interrupt routine every 64 microseconds (15.625 kHz)
ISR(TIMER1_OVF_vect)
{
    // debug flag to test how long the interrupt routine runs
    PORTB = 0x01;
    
    // even out delayed interrupt trigger (due to 1 or 2 cycle instructions)
    __asm__ __volatile__ (
        "lds __tmp_reg__,%[counter]\n\t"   
        "asr __tmp_reg__\n\t"       // test for bit 0 of the counter
        "brcc afterbranch\n\t"      // this takes either 1 or 2 cycles 
      "afterbranch:\n\t"
        :
        :  [counter] "i" (_SFR_ADDR(TCNT1L))
    );
      
    processVideo();
    processAudio();

    PORTB = 0;
}
