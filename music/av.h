
#ifndef av_h
#define av_h

#include <inttypes.h>

#define HERTZ(n) ((uint16_t)((n)*65536.0/15625.0))


typedef struct _AV_VOICE
{
  uint8_t volume;
  uint8_t currentvolume;
  uint8_t* waveform;  // point to program space
  uint8_t duty;  // for rectangle only
  uint16_t frequency;
  uint16_t phase;
} _AV_VOICE;

typedef void (*renderfunction_t)(uint8_t line);


void av_init(renderfunction_t renderfunction);
void av_waitForBlanking();
extern _AV_VOICE voiceA;
extern _AV_VOICE voiceB;
extern _AV_VOICE voiceC;
extern _AV_VOICE voiceD;

#endif
