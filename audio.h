#ifndef _SFP_AUDIO_H_
#define _SFP_AUDIO_H_

#include "common.h"

int sfp_audio_init();

#ifdef I_WANT_INTERNAL_AUDIO_STUFF
int sfp_audio_audio_init();

#endif /* I_WANT_INTERNAL_AUDIO_STUFF */
#endif /* _SFP_AUDIO_H_ */
