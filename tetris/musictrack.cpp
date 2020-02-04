#include "musictrack.h"
#include <avr/pgmspace.h>
#include "av.h"

// designed for a ATMEGA328

static const uint16_t PROGMEM frequency_table[] = 
{
    // 1. octave
    HERTZ(36.71),     // 0 = D
    HERTZ(38.89),     // 1 = D#
    HERTZ(41.20),     // 2 = E
    HERTZ(43.65),     // 3 = F
    HERTZ(46.25),     // 4 = F#    
    HERTZ(49.00),     // 5 = G
    HERTZ(51.91),     // 6 = G#   
    HERTZ(55.00),     // 7 = A  
    HERTZ(58.27),     // 8 = A#
    HERTZ(61.74),     // 9 = B
    0,0,0,0,0,0,0,
    // 2. octave 
    HERTZ(65.41),     // A = C
    HERTZ(69.30),     // B = C#
    HERTZ(73.42),     // C = D
    HERTZ(77.78),     // D = D#
    HERTZ(82.41),     // E = E
    HERTZ(87.31),     // F = F
    HERTZ(92.50),     // G = F#    
    HERTZ(98.00),     // H = G
    HERTZ(103.83),    // I = G#   
    HERTZ(110.00),    // J = A  
    HERTZ(116.54),    // K = A#
    HERTZ(123.47),    // L = B
    // 3. octave
    HERTZ(130.81),    // M = C
    HERTZ(138.59),    // N = C#
    HERTZ(146.83),    // O = D
    HERTZ(155.56),    // P = D#
    HERTZ(164.81),    // Q = E
    HERTZ(174.61),    // R = F
    HERTZ(185.00),    // S = F#    
    HERTZ(196.00),    // T = G
    HERTZ(207.65),    // U = G#   
    HERTZ(220.00),    // V = A  
    HERTZ(233.08),    // W = A# 
    HERTZ(246.94),    // X = B
    0,0,0,0,0,0,0,0,
    // 4. octave
    HERTZ(261.63),    // a = C
    HERTZ(277.18),    // b = C#
    HERTZ(293.66),    // c = D
    HERTZ(311.13),    // d = D#
    HERTZ(329.63),    // e = E
    HERTZ(349.23),    // f = F
    HERTZ(369.99),    // g = F#    
    HERTZ(392.00),    // h = G
    HERTZ(415.30),    // i = G#   
    HERTZ(440.00),    // j = A
    HERTZ(466.16),    // k = A# 
    HERTZ(493.88),    // l = B
    // 5. octave
    HERTZ(523.25),    // m = C
    HERTZ(554.37),    // n = C#
    HERTZ(587.33),    // o = D
    HERTZ(622.25),    // p = D#
    HERTZ(659.25),    // q = E
    HERTZ(698.46),    // r = F
    HERTZ(739.99),    // s = F#    
    HERTZ(783.99),    // t = G
    HERTZ(830.61),    // u = G#   
    HERTZ(880.00),    // v = A  
    HERTZ(932.33),    // w = A# 
    HERTZ(987.77),    // x = B
    // 6. octave    
    HERTZ(1046.50),   // y = C
    HERTZ(1108.73)    // z = C#
};

Musictrack::Musictrack(_AV_VOICE &voice, uint8_t volume, uint8_t* waveform, uint8_t ticks_per_note, const __FlashStringHelper* notes) 
{   
    this->voice = &voice;
    this->volume = volume;
    this->waveform = waveform;
    this->ticks_per_note = ticks_per_note;
    this->notes = (uint8_t*) notes;
    start();
}

void Musictrack::start()
{
    this->cursor = notes;
    this->tickcounter = 0;
    this->targetvolume = 0;
    
    voice->volume = 0;
    voice->waveform = waveform;
}

bool Musictrack::tick()
{
    if (tickcounter>0)
    {
        tickcounter--;    
        adjustvolume();    
        return true;
    }

    tickcounter = ticks_per_note-1;
    
    uint8_t x = pgm_read_byte(cursor);
    cursor++;
    
    if ((x>='0') && (x<='z'))
    {   uint8_t index = x-'0';
        uint16_t f = pgm_read_word(frequency_table+index);
        voice->frequency = f;
        voice->volume = volume;
        targetvolume = (volume>>2) + (volume>>1);        
    }
    else if (x!='-') 
    {
       targetvolume = 0;        
       adjustvolume();
    }

    return x!=0;
}

void Musictrack::silence()
{
  voice->volume = 0;
  targetvolume = 0;
}

void Musictrack::adjustvolume()
{
    int16_t v = voice->volume;
    if (v > targetvolume)
    {
        uint8_t d = 20;
        if (v-d > targetvolume) v-=d;
        else v=targetvolume;
    } 
   
    voice->volume = (uint8_t) v;
}


void Song::start()
{
    uint8_t i;
    
    current = 0;
    for (i=0; i<numtracks; i++) { part[0][i].start(); }
}

void Song::tick()
{
    uint8_t i;

    bool over = false;
    for (i=0; i<numtracks; i++) 
    {   
        if (!part[current][i].tick()) { over=true; }
    }

    if (over)
    {
        current++;
        if (current>=numparts) { current=0; }
        
        for (i=0; i<numtracks; i++) 
        {   part[current][i].start(); 
            part[current][i].tick();
        }
    }
}

void Song::silence()
{
    uint8_t i;
    for (i=0; i<numtracks; i++) 
    {   
        part[current][i].silence();
    }
}
