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
#include "../../SDL_internal.h"

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

static int GKAUDIO_OpenDevice(_THIS, const char *devname)
{
    int bytes_per_sample = 0;
    this->hidden = (struct SDL_PrivateAudioData *)SDL_calloc(1, sizeof(*this->hidden));

    if(this->hidden == NULL)
    {
        return SDL_OutOfMemory();
    }

    // cap to stereo
    if(this->spec.channels > 2)
    {
        this->spec.channels = 2;
    }

    // only allow supported formats
    if(this->spec.format == AUDIO_U8)
    {
        this->spec.format = AUDIO_S8;
    }
    if(this->spec.format == AUDIO_U16)
    {
        this->spec.format = AUDIO_S16;
    }
    switch(this->spec.format)
    {
        case AUDIO_S8:
            bytes_per_sample = 1;
            break;
        case AUDIO_S16:
            bytes_per_sample = 2;
            break;
        case AUDIO_S32:
            bytes_per_sample = 4;
            break;
        default:
            this->spec.format = AUDIO_S16;
            bytes_per_sample = 2;
            break;
    }

    // only allow supported audio frequencies
    if(this->spec.freq < 4000)
        this->spec.freq = 4000;
    if(this->spec.freq > 48000)
        this->spec.freq = 48000;

    // cap nsamps if required - hard limit of 32 kiB/2 = 16 kiB buffer in GK
    if((bytes_per_sample * this->spec.channels * this->spec.samples) > 16*1024)
    {
        this->spec.samples = 16 * 1024 / bytes_per_sample / this->spec.channels;
    }

    /* Update the fragment size as size in bytes */
    SDL_CalculateAudioSpec(&this->spec);
    
    /* Check this is okay with GK */
    if(GK_AudioSetMode(this->spec.channels, bytes_per_sample * 8,
        this->spec.freq, this->spec.size) != 0)
    {
        SDL_free(this->hidden);
        return -1;
    }

    /* Get the first buffer */
    GK_AudioQueueBuffer(NULL, (void **)&this->hidden->mixbuf);
    GK_AudioEnable(1);

    SDL_memset(this->hidden->mixbuf, this->spec.silence, this->spec.size);

    return 0;
}

static int GKAUDIO_CaptureFromDevice(_THIS, void *buffer, int buflen)
{
    /* Delay to make this sort of simulate real audio input. */
    SDL_Delay((this->spec.samples * 1000) / this->spec.freq);

    /* always return a full buffer of silence. */
    SDL_memset(buffer, this->spec.silence, buflen);
    return buflen;
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

static void GKAUDIO_ThreadInit(_THIS)
{
    // Can set thread priority etc here
}

static SDL_bool GKAUDIO_Init(SDL_AudioDriverImpl *impl)
{
    /* Set the function pointers */
    impl->OpenDevice = GKAUDIO_OpenDevice;
    impl->PlayDevice = GKAUDIO_PlayDevice;
    impl->WaitDevice = GKAUDIO_WaitDevice;
    impl->GetDeviceBuf = GKAUDIO_GetDeviceBuf;
    impl->CloseDevice = GKAUDIO_CloseDevice;
    impl->ThreadInit = GKAUDIO_ThreadInit;
    impl->LockDevice = GKAUDIO_LockAudio;
    impl->UnlockDevice = GKAUDIO_UnlockAudio;
    impl->OnlyHasDefaultOutputDevice = SDL_TRUE;

    /* Should be possible, but micInit would fail */
    impl->HasCaptureSupport = SDL_FALSE;
    impl->CaptureFromDevice = GKAUDIO_CaptureFromDevice;

    return SDL_TRUE; /* this audio target is available. */
}

AudioBootStrap GKAUDIO_bootstrap = {
    GKAUDIO_DRIVER_NAME,
    "GK audio driver",
    GKAUDIO_Init,
    0
};


#endif
