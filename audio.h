#ifndef _SFP_AUDIO_H_
#define _SFP_AUDIO_H_

#include "common.h"

#define AUDIO_RB_SIZE 65536

extern u16 audio_rb[][2];
extern int audio_rb_start;
extern int audio_rb_end;

int sfp_audio_init();

#ifdef I_WANT_INTERNAL_AUDIO_STUFF
int audio_stealchunk(int len, s16 *buf);

int sfp_audio_audio_init();

#endif /* I_WANT_INTERNAL_AUDIO_STUFF */
#endif /* _SFP_AUDIO_H_ */
