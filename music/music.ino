// running on an atmega328

#include "synth.h"
#include "musictrack.h"


void setup() 
{
    noInterrupts();
    Synth::init(simplepattern);
    interrupts();
} 

void simplepattern(uint8_t vcounter) 
{
    PORTD = 0xC0;
    PORTD = 0xC0;
    PORTD = 0xC0;
    PORTD = 0xC0;
    PORTD = 0xC0;
    PORTD = 0xC0;
    PORTD = 0xC0;
    PORTD = 0xC0;
    PORTD = 0xC0;
    PORTD = 0xC0;
    PORTD = 0xC0;
    PORTD = 0xC0;
    PORTD = 0x00;
    PORTD = 0x00;
    PORTD = 0x00;
    PORTD = 0x00;
    
    register uint8_t x = (vcounter<<5) & 0xC0;

    PORTD=x; PORTD=x; PORTD=x; x += 64;
    PORTD=x; PORTD=x; PORTD=x; x += 64;
    PORTD=x; PORTD=x; PORTD=x; x += 64;
    PORTD=x; PORTD=x; PORTD=x; x += 64;
    PORTD=x; PORTD=x; PORTD=x; x += 64;
    PORTD=x; PORTD=x; PORTD=x; x += 64;
    PORTD=x; PORTD=x; PORTD=x; x += 64;
    PORTD=x; PORTD=x; PORTD=x; x += 64;
    PORTD=x; PORTD=x; PORTD=x; x += 64;
    PORTD=x; PORTD=x; PORTD=x; x += 64;
    PORTD=x; PORTD=x; PORTD=x; x += 64;
    PORTD=x; PORTD=x; PORTD=x; x += 64;
    PORTD=x; PORTD=x; PORTD=x; x += 64;
    PORTD=x; PORTD=x; PORTD=x; x += 64;
    PORTD=x; PORTD=x; PORTD=x; x += 64;
    PORTD=x; PORTD=x; PORTD=x; x += 64;

    PORTD = 0;
}

void loop()
{
    Musictrack part1[4] = {
        //                                 |               |               |               |               |               |               |               |               |
        Musictrack(0, 90,  TRIANGLE, 5, F("q---l-m-o-qom-l-j---j-m-q---o-m-l---llm-o---q---m---j---j-------o---o-r-v---t-r-q---q-m-q---o-m-l---llm-o---q---m---j---j------ ")),
        Musictrack(1, 0,   TRIANGLE, 5, F("                                                                                                                                ")),
        Musictrack(2, 80,  SINE,     5, F("Q-- Q-- Q-- Q-- V-- V-- V-- V-- U-- U-- Q-- Q-- V-- V-- V-- a-- O-- O-- O-- O-- M-- M-- M-- M-- H-- H-- Q-- Q-- V-- V-- V------ ")),
        Musictrack(3, 70,  SINE,     5, F("  e-- e-- e-- e-- j-- j-- j-- j-- j-- j-- e-- e-- j-- j-- X-- c-- c-- c-- c-- c-- a-- a-- a-- a-- T-- T-- e-- e-- j-- j-- j--   "))
    };
    Musictrack part2[4] = {
        //                                 |               |               |               |               |               |               |               |               | 
        Musictrack(0, 70, TRIANGLE,  5, F("m-------j-------l-------i-------j-------e-------e-------i-------m-------j-------l-------i-------j---m---q---q---q-------------- ")),
        Musictrack(1, 70, TRIANGLE,  5, F("q-------m-------o-------l-------m-------j-------i-------l-------q-------m-------o-------l-------m---q---v---v---u-------------- ")),
        Musictrack(2, 60, SINE,      5, F("V-- V-- V-- V-- X-- X-- X-- X-- V-- V-- V-- V-- U-- U-- U-- U-- V-- V-- V-- V-- X-- X-- X-- X-- V-- V-- V-- V-- U-- U-- U-- U-- ")),
        Musictrack(3, 60, SINE,      5, F("  e-- e-- e-- e-- c-- c-- c-- c-- e-- e-- e-- e-- e-- e-- e-- e-- e-- e-- e-- e-- c-- c-- c-- c-- e-- e-- e-- e-- e-- e--       "))
    };
    Song song(4, part1, part1, part2, part1);

    Synth::waitForBlanking();
    song.start();
    while (song.tick()) { Synth::waitForBlanking(); }

    for (;;) {}
}
