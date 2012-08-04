#define I_WANT_INTERNAL_AUDIO_STUFF
#include "audio.h"

#include <SDL.h>

SDL_AudioSpec basespec;
SDL_AudioSpec usedspec;

s16 audio_load_buf[AUDIO_RB_SIZE][2];

// TODO work out how to cleanly deal to this duplication issue --GM
int sfp_sdl_error_audio(char *ref)
{
	fprintf(stderr, "%s: %s\n", ref, SDL_GetError());
	return 1;
}

void audio_cb_sdl(void *userdata, Uint8 *stream, int len)
{
	int slen = audio_stealchunk(len/4, (s16 *)stream);
	return;
}

int sfp_audio_audio_init()
{
	if(SDL_InitSubSystem(SDL_INIT_AUDIO))
		return sfp_sdl_error_audio("SDL_InitSubSystem(SDL_INIT_AUDIO)");
	
	basespec.freq = 44100;
	basespec.format = AUDIO_S16SYS;
	basespec.channels = 2;
	basespec.samples = 4096;
	basespec.callback = audio_cb_sdl;
	basespec.userdata = NULL;
	
	if(SDL_OpenAudio(&basespec, &usedspec))
		return sfp_sdl_error_audio("SDL_OpenAudio");
	
	fprintf(stderr, "SDL AUDIO: f=%i, fmt=%i, ch=%i, smp=%i\n",
		usedspec.freq,
		usedspec.format,
		usedspec.channels,
		usedspec.samples);
	
	SDL_PauseAudio(0);
	
	return 0;
}
