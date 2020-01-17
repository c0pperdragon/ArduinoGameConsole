
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


void av_init(uint8_t *tileset, uint8_t *defaultwaveform);
void av_waitForBlanking();
void av_setTileSet(uint8_t *tilesset);

extern uint8_t videoMatrix[320];
extern _AV_VOICE voiceA;
extern _AV_VOICE voiceB;
extern _AV_VOICE voiceC;
extern _AV_VOICE voiceD;

#endif
