
#ifndef Musictrack_h
#define Musictrack_h

#include <inttypes.h>
#include "av.h"

class __FlashStringHelper;

typedef class Musictrack Musictrack;
class Musictrack 
{
public:
    Musictrack(_AV_VOICE &voice, uint8_t volume, uint8_t* waveform, uint8_t ticks_per_note, const __FlashStringHelper* notes);
    void start();
    bool tick();
    void silence();

private:
    void adjustvolume();
 
    _AV_VOICE *voice;
    uint8_t volume;
    uint8_t* waveform; // points to program space!
    uint8_t ticks_per_note;
    uint8_t* notes;   // points to program space!
    
    uint8_t* cursor;  // points to program space!
    uint8_t targetvolume; 
    uint8_t tickcounter;  
};

typedef class Song Song;
class Song
{
    public:
        inline Song(uint8_t tracks, Musictrack* part0)
        {
            numtracks = tracks;
            numparts=1;
            part[0] = part0;
        }
        inline Song(uint8_t tracks, Musictrack* part0, Musictrack* part1)
        {
            numtracks = tracks;
            numparts=2;
            part[0] = part0;
            part[1] = part1;
        }
        inline Song(uint8_t tracks, Musictrack* part0, Musictrack* part1, Musictrack* part2) 
        {
            numtracks = tracks;
            numparts=3;
            part[0] = part0;
            part[1] = part1;
            part[2] = part2;
        }
        inline Song(uint8_t tracks, Musictrack* part0, Musictrack* part1, Musictrack* part2, Musictrack* part3)
        {
            numtracks = tracks;
            numparts=4;
            part[0] = part0;
            part[1] = part1;
            part[2] = part2;          
            part[3] = part3;          
        }

        void start();
        void tick();
        void silence();

    private:
        uint8_t numtracks;
        uint8_t numparts;
        Musictrack* part[10];
        uint8_t current;
};

#endif
