
#ifndef Synth_h
#define Synth_h

#include <inttypes.h>

#define NUMVOICES 4

#define RECTANGLE   0
#define SINE        1
#define TRIANGLE    2
#define SAWTOOTH    3

#define HERTZ(n) ((uint16_t)((n)*65536.0/15625.0))


typedef struct _SYNTH_VOICE
{
  uint8_t volume;
  uint8_t currentvolume;
  uint8_t waveform;
  uint8_t duty;  // for rectangle only
  uint16_t frequency;
  uint16_t phase;
} _SYNTH_VOICE;

typedef void (*renderfunction_t)(uint8_t line);

class Synth 
{
public:
  static void init(renderfunction_t renderfunction);
  static void waitForBlanking();
  static _SYNTH_VOICE voice[NUMVOICES];

//private:
  static uint8_t level;             // current output voltage level
  static uint16_t boosttable[256];  // range from 0 to 1023
  static renderfunction_t renderfunction;
    
  static inline void processAudio();
  static inline int8_t processVoice(_SYNTH_VOICE &v);
  static inline void adjustSync(uint8_t vcounter);
};

#endif
