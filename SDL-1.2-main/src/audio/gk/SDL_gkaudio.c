/*
  Simple DirectMedia Layer
  Copyright (C) 1997-2023 Sam Lantinga <slouken@libsdl.org>

  This software is provided 'as-is', without any express or implied
  warranty.  In no event will the authors be held liable for any damages
  arising from the use of this software.

  Permission is granted to anyone to use this software for any purpose,
  including commercial applications, and to alter it and redistribute it
  freely, subject to the following restrictions:

  1. The origin of this software must not be misrepresented; you must not
     claim that you wrote the original software. If you use this software
     in a product, an acknowledgment in the product documentation would be
     appreciated but is not required.
  2. Altered source versions must be plainly marked as such, and must not be
     misrepresented as being the original software.
  3. This notice may not be removed or altered from any source distribution.
*/
#include "SDL_config.h"

/* Output audio to nowhere... */

//#include "SDL_rwops.h"
#include "SDL_timer.h"
#include "SDL_audio.h"
#include "../SDL_audiomem.h"
#include "../SDL_audio_c.h"
#include "../SDL_audiodev_c.h"

#ifdef SDL_AUDIO_DRIVER_GK

#include "SDL_audio.h"

/* GK Audio driver */

#include "../SDL_sysaudio.h"
#include "SDL_timer.h"

#include "gk.h"

#define GKAUDIO_DRIVER_NAME "gk"

#define _THIS SDL_AudioDevice *this

struct SDL_PrivateAudioData
{
    /* Need some buffer for SDL to write to */
    Uint8 *mixbuf;
};

static void GKAUDIO_LockAudio(_THIS)
{

}

static void GKAUDIO_UnlockAudio(_THIS)
{

}

static int GKAUDIO_OpenDevice(_THIS, SDL_AudioSpec *spec)
{
    int bytes_per_sample = 0;
    this->hidden = (struct SDL_PrivateAudioData *)SDL_calloc(1, sizeof(*this->hidden));

    if(this->hidden == NULL)
    {
        SDL_OutOfMemory();
        return -1;
    }

    // cap to stereo
    if(spec->channels > 2)
    {
        spec->channels = 2;
    }

    // only allow supported formats
    if(spec->format == AUDIO_U8)
    {
        spec->format = AUDIO_S8;
    }
    if(spec->format == AUDIO_U16)
    {
        spec->format = AUDIO_S16;
    }
    switch(spec->format)
    {
        case AUDIO_S8:
            bytes_per_sample = 1;
            break;
        case AUDIO_S16:
            bytes_per_sample = 2;
            break;
        default:
            spec->format = AUDIO_S16;
            bytes_per_sample = 2;
            break;
    }

    // only allow supported audio frequencies
    if(spec->freq < 11025)
        spec->freq = 8000;
    else if(spec->freq < 12000)
        spec->freq = 11025;
    else if(spec->freq < 16000)
        spec->freq = 12000;
    else if(spec->freq < 22050)
        spec->freq = 16000;
    else if(spec->freq < 24000)
        spec->freq = 22050;
    else if(spec->freq < 32000)
        spec->freq = 24000;
    else if(spec->freq < 44100)
        spec->freq = 32000;
    else if(spec->freq < 48000)
        spec->freq = 44100;
    else
        spec->freq = 48000;

    // cap nsamps if required - hard limit of 32 kiB/2 = 16 kiB buffer in GK
    if((bytes_per_sample * spec->channels * spec->samples) > 16*1024)
    {
        spec->samples = 16 * 1024 / bytes_per_sample / spec->channels;
    }

    /* Update the fragment size as size in bytes */
    SDL_CalculateAudioSpec(&this->spec);
    
    /* Check this is okay with GK */
    if(GK_AudioSetMode(spec->channels, bytes_per_sample * 8,
        spec->freq, spec->size) != 0)
    {
        SDL_free(this->hidden);
        return -1;
    }

    /* Get the first buffer */
    GK_AudioQueueBuffer(NULL, (void **)&this->hidden->mixbuf);
    GK_AudioEnable(1);

    SDL_memset(this->hidden->mixbuf, spec->silence, spec->size);

    return 0;
}

static void GKAUDIO_PlayDevice(_THIS)
{
    //GK_AudioEnable(1);
    GK_AudioQueueBuffer(this->hidden->mixbuf, (void**)&this->hidden->mixbuf);
}

static void GKAUDIO_WaitDevice(_THIS)
{
    GK_AudioWaitFree();
}

static Uint8 *GKAUDIO_GetDeviceBuf(_THIS)
{
    return this->hidden->mixbuf;
}

static void GKAUDIO_CloseDevice(_THIS)
{
    // TODO
}

static void GKAUDIO_Free(SDL_AudioDevice *device)
{
    SDL_free(device->hidden);
    SDL_free(device);
}

static void GKAUDIO_ThreadInit(_THIS)
{
    // Can set thread priority etc here
}

static int GKAUDIO_Available(void)
{
    return 1;
}

static SDL_AudioDevice * GKAUDIO_Init(int devindex)
{
    SDL_AudioDevice *impl;

	/* Initialize all variables that we clean on shutdown */
	impl = (SDL_AudioDevice *)SDL_malloc(sizeof(SDL_AudioDevice));
	if ( impl ) {
		SDL_memset(impl, 0, (sizeof *impl));
		impl->hidden = (struct SDL_PrivateAudioData *)
				SDL_malloc((sizeof *impl->hidden));
	}
	if ( (impl == NULL) || (impl->hidden == NULL) ) {
		SDL_OutOfMemory();
		if ( impl ) {
			SDL_free(impl);
		}
		return(0);
	}
	SDL_memset(impl->hidden, 0, (sizeof *impl->hidden));

    /* Set the function pointers */
    impl->OpenAudio = GKAUDIO_OpenDevice;
    impl->PlayAudio = GKAUDIO_PlayDevice;
    impl->WaitAudio = GKAUDIO_WaitDevice;
    impl->GetAudioBuf = GKAUDIO_GetDeviceBuf;
    impl->CloseAudio = GKAUDIO_CloseDevice;
    impl->ThreadInit = GKAUDIO_ThreadInit;
    impl->LockAudio = GKAUDIO_LockAudio;
    impl->UnlockAudio = GKAUDIO_UnlockAudio;
    //impl->OnlyHasDefaultOutputDevice = SDL_TRUE;

    /* Should be possible, but micInit would fail */
    //impl->HasCaptureSupport = SDL_FALSE;
    //impl->CaptureFromDevice = GKAUDIO_CaptureFromDevice;
    impl->free = GKAUDIO_Free;

    return impl; /* this audio target is available. */
}

AudioBootStrap GKAUDIO_bootstrap = {
    GKAUDIO_DRIVER_NAME,
    "GK audio driver",
    GKAUDIO_Available,
    GKAUDIO_Init
};


#endif
