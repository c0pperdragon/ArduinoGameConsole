
#ifndef Synth_h
#define Synth_h

#include <inttypes.h>

#define MAXVOICES 4

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

typedef void (*voidfunction_t)();

class Synth 
{
public:
  static void init(uint8_t numvoices);
  static void setNotify(voidfunction_t n);
 
  static volatile _SYNTH_VOICE voice[MAXVOICES];
  
//private:
  static uint8_t numvoices;         // number of supported voices
  static uint8_t level;             // current output voltage level
  static uint16_t boosttable[256];  // range from 0 to 1023
  static voidfunction_t notificationHandler;   // notification in frequency of sample
    
  static inline void processAudio();
  static inline int8_t processVoice(volatile _SYNTH_VOICE &v);
};

#endif
