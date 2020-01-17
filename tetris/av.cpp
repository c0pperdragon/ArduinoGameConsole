// designed for a Atmega328p
#include "av.h"
#include <avr/interrupt.h>
#include <avr/pgmspace.h>
#include "pins_arduino.h"


_AV_VOICE voiceA;
_AV_VOICE voiceB;
_AV_VOICE voiceC;
_AV_VOICE voiceD;

uint8_t videoMatrix[320];
uint8_t *av_tileset;

uint8_t av_level;             // current output voltage level
uint8_t vcounter;
volatile bool isblanking;

         
inline void initVoice(_AV_VOICE &v, uint8_t *waveform)
{
    v.volume = 0;
    v.currentvolume = 0;
    v.waveform = waveform;
    v.duty = 128;
    v.frequency = HERTZ(440);
    v.phase = 0;
    v.volume = 0;
}

void av_init(uint8_t *tileset, uint8_t *defaultwaveform) 
{   
    cli();
    
    uint16_t i;
   
    av_tileset = tileset;
    for (i=0; i<320; i++) { videoMatrix[i]=0; }
    
    // start values of individual signal generation
    initVoice(voiceA,defaultwaveform);
    initVoice(voiceA,defaultwaveform);
    initVoice(voiceA,defaultwaveform);
    initVoice(voiceA,defaultwaveform);

    av_level = 128;
    isblanking = true;
    vcounter = 0;

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

    // set timers to well-defined initial values to force timers to run in harmonic operation from now on
    TCNT1H = 0;    // prepare high-byte for timer1 conter
    TCNT1L = 0;    // clear timer1 counter
    GTCCR = 0x01;  // clear timer 0 prescaler
    TCNT0 = 22;    // set timer0 counter to specified start value
       
    // enable timer 1 overflow interrupt at the next line start
    TIMSK1 = 0x01;  // B00000001;     
    // disable all other interrupts
    TIMSK0 = 0x00;
    TIMSK2 = 0x00;

    // configure output pin directions
    DDRB |= 0x07;  // B00000111    enable timer 1 output pins and debug signal
    DDRD |= 0xA8;  // B10101000;   enable video output pins 

    sei();
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

void av_setTileSet(uint8_t *tileset)
{
    av_tileset = tileset;
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

inline void renderLine()
{
    uint8_t line_x8 = (vcounter & 0xf0) >> 1;
    
    __asm__ __volatile__ 
    (
        "ld r30,x+           ; fetch next tile index                   \n"
        "swap r30            ; prepare index                           \n"
        "mov r31,r30         ;                                         \n"         
        "andi r30,0xf0       ; multiply index by 16: step 1            \n" 
        "andi r31,0x0f       ; multiply index by 16: step 2            \n"
        "add r30,r28         ; compute pixel address                   \n"     
        "adc r31,r29         ; compute pixel address  -> z             \n"

#define RENDER_ONE_TILE \
        "lpm __tmp_reg__,z+          ; 0   fetch pixel data            \n" \
        "                            ; 1                               \n" \
        "                            ; 2                               \n" \
        "out %[port],__tmp_reg__     ; 3   write pixel to port         \n" \
        "ld r25,x+                   ; 4   fetch next tile index       \n" \
        "                            ; 5                               \n" \
        "lsl __tmp_reg__             ; 6                               \n" \ 
        "out %[port],__tmp_reg__     ; 7   write pixel to port         \n" \
        "swap r25                    ; 8   prepare tile index          \n" \
        "nop                         ; 9                               \n" \
        "lsl __tmp_reg__             ; 10                              \n" \ 
        "out %[port],__tmp_reg__     ; 11  write pixel to port         \n" \
        "nop                         ; 12                              \n" \
        "nop                         ; 13                              \n" \
        "lsl __tmp_reg__             ; 14                              \n" \ 
        "out %[port],__tmp_reg__     ; 15  write pixel to port         \n" \
        "lpm __tmp_reg__,z+          ; 16  fetch pixel data            \n" \
        "                            ; 17                              \n" \
        "                            ; 18                              \n" \
        "out %[port],__tmp_reg__     ; 19  write pixel to port         \n" \
        "mov r31,r25                 ; 20                              \n" \
        "mov r30,r25                 ; 21                              \n" \
        "lsl __tmp_reg__             ; 22                              \n" \ 
        "out %[port],__tmp_reg__     ; 23  write pixel to port         \n" \
        "andi r30,0xf0               ; 24  multiply index step 1       \n" \
        "andi r31,0x0f               ; 25  multiply index step 2       \n" \
        "lsl __tmp_reg__             ; 26                              \n" \ 
        "out %[port],__tmp_reg__     ; 27  write pixel to port         \n" \
        "add r30,r28                 ; 28  compute pixel address       \n" \   
        "adc r31,r29                 ; 29  compute pixel address  -> z \n" \
        "lsl __tmp_reg__             ; 30                              \n" \ 
        "out %[port],__tmp_reg__     ; 31  write pixel to port         \n" \

        RENDER_ONE_TILE
        RENDER_ONE_TILE
        RENDER_ONE_TILE
        RENDER_ONE_TILE
        RENDER_ONE_TILE
        RENDER_ONE_TILE
        RENDER_ONE_TILE
        RENDER_ONE_TILE
        RENDER_ONE_TILE
        RENDER_ONE_TILE
        RENDER_ONE_TILE
        RENDER_ONE_TILE
        RENDER_ONE_TILE
        RENDER_ONE_TILE
        RENDER_ONE_TILE
        RENDER_ONE_TILE
        RENDER_ONE_TILE
        RENDER_ONE_TILE
        RENDER_ONE_TILE
        RENDER_ONE_TILE
        
        "nop                                                           \n"
        "nop                                                           \n"
        "eor __tmp_reg__,__tmp_reg__                                   \n"
        "out %[port],__tmp_reg__                                       \n"        
        :  
        :  [port] "I" (_SFR_IO_ADDR(PORTD)), 
                  "x" ( ((uint16_t*)videoMatrix) + (line_x8+(line_x8>>2))),
                  "y" (av_tileset + (vcounter&0x0e) )
        :  "r25", "r30", "r31"
    );
}

inline void processVideo()
{
      // progress line counter and trigger appropriate render or sync adjustment action
    if (!isblanking)
    {
        renderLine();
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
    __asm__ __volatile__ 
    (
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
