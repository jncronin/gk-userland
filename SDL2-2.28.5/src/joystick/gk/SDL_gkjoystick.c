#include "../../SDL_internal.h"

#ifdef SDL_JOYSTICK_GK

#include <gk.h>

#include "../SDL_sysjoystick.h"
#include "SDL_events.h"
#include "_gk_memaddrs.h"

static int GK_JoystickInit(void)
{
    return 0;
}

static int GK_JoystickGetCount(void)
{
    return 1;
}

static void GK_JoystickDetect(void)
{
}

static const char *GK_JoystickGetDeviceName(int device_index)
{
    return "GK";
}

static const char *GK_JoystickGetDevicePath(int device_index)
{
    return NULL;
}

static int GK_JoystickGetDevicePlayerIndex(int device_index)
{
    return -1;
}

static void GK_JoystickSetDevicePlayerIndex(int device_index, int player_index)
{
}

static SDL_JoystickGUID GK_JoystickGetDeviceGUID(int device_index)
{
    SDL_JoystickGUID guid = SDL_CreateJoystickGUIDForName(GK_JoystickGetDeviceName(device_index));
    return guid;
}

static SDL_JoystickID GK_JoystickGetDeviceInstanceID(int device_index)
{
    return device_index;
}

static int GK_JoystickOpen(SDL_Joystick *joystick, int device_index)
{
#if __GAMEKID__ >= 4
    joystick->nbuttons = GK_KERNEL_INFO->joystick_nbuttons;
    joystick->naxes = GK_KERNEL_INFO->joystick_naxes;
#else
    joystick->nbuttons = 0;
    joystick->naxes = 4;
#endif
    joystick->nhats = 0;
    joystick->instance_id = device_index;

    return 0;
}

static int GK_JoystickRumble(SDL_Joystick *joystick, Uint16 low_frequency_rumble, Uint16 high_frequency_rumble)
{
    return SDL_Unsupported();
}

static int GK_JoystickRumbleTriggers(SDL_Joystick *joystick, Uint16 left_rumble, Uint16 right_rumble)
{
    return SDL_Unsupported();
}

static Uint32 GK_JoystickGetCapabilities(SDL_Joystick *joystick)
{
    return SDL_JOYCAP_LED;
}

static int GK_JoystickSetLED(SDL_Joystick *joystick, Uint8 red, Uint8 green, Uint8 blue)
{
    uint32_t argb = (((uint32_t)red) << 16) |
        (((uint32_t)green) << 8) |
        (uint32_t)blue;
    GK_SetLED(0, argb);
    return 0;
}

static int GK_JoystickSendEffect(SDL_Joystick *joystick, const void *data, int size)
{
    return SDL_Unsupported();
}

static int GK_JoystickSetSensorsEnabled(SDL_Joystick *joystick, SDL_bool enabled)
{
    return SDL_Unsupported();
}

static void GK_JoystickUpdate(SDL_Joystick *joystick)
{
#if __GAMEKID__ >= 4
    static unsigned long last_ticks = 0;
    static int16_t old_axes[8] = { 0 };
    static unsigned int oldbuttons = 0;
    const int16_t delta = 50;
    const int delta_sq = (int)delta * (int)delta;
    uint64_t buttons;
    unsigned int naxes = (joystick->naxes < 8) ? joystick->naxes : 8;
    unsigned int nbuttons = (joystick->nbuttons < 64) ? joystick->nbuttons : 64;
    unsigned long now = GK_GetCurUs();

    if(now < (last_ticks + 5000UL))
    {
        return;
    }

    /* Only report if the stick moves by more than delta in a 2D direction */
    for(unsigned int axis_pair = 0; axis_pair < naxes / 2; axis_pair++)
    {
        int16_t *valp_x = GK_KERNEL_INFO->joystick_axes[axis_pair * 2];
        int16_t curval_x = valp_x ? *valp_x : 0;
        int16_t *valp_y = GK_KERNEL_INFO->joystick_axes[axis_pair * 2 + 1];
        int16_t curval_y = valp_y ? *valp_y : 0;

        int x_diff = (int)curval_x - (int)old_axes[axis_pair * 2];
        int y_diff = (int)curval_y - (int)old_axes[axis_pair * 2 + 1];

        int diff_sq = x_diff * x_diff + y_diff * y_diff;

        if(diff_sq >= delta_sq)
        {
            SDL_PrivateJoystickAxis(joystick, axis_pair * 2, curval_x);
            SDL_PrivateJoystickAxis(joystick, axis_pair * 2 + 1, curval_y);
            old_axes[axis_pair * 2] = curval_x;
            old_axes[axis_pair * 2 + 1] = curval_y;
            last_ticks = now;
        }
    }

    buttons = GK_KERNEL_INFO->joystick_buttons;
    for(unsigned int button = 0; button < nbuttons; button++)
    {
        uint64_t mask = (1ULL << button);

        if((buttons & mask) && !(oldbuttons & mask))
        {
            SDL_PrivateJoystickButton(joystick, button, SDL_PRESSED);
            last_ticks = now;
        }
        else if(!(buttons & mask) && (oldbuttons & mask))
        {
            SDL_PrivateJoystickButton(joystick, button, SDL_RELEASED);
            last_ticks = now;
        }
    }
    oldbuttons = buttons;
#else
    int jx, jy, tx, ty;
    GK_GetJoystickAxes(&jx, &jy);
    GK_GetTiltAxes(&tx, &ty);

    SDL_PrivateJoystickAxis(joystick, 0, jx);
    SDL_PrivateJoystickAxis(joystick, 1, jy);
    SDL_PrivateJoystickAxis(joystick, 2, tx);
    SDL_PrivateJoystickAxis(joystick, 3, ty);
#endif
}

static void GK_JoystickClose(SDL_Joystick *joystick)
{
}

static void GK_JoystickQuit()
{
}

static SDL_bool GK_JoystickGetGamepadMapping(int device_index, SDL_GamepadMapping *out)
{
    *out = (SDL_GamepadMapping) {
        .a = { EMappingKind_None, 255 },
        .b = { EMappingKind_None, 255 },
        .x = { EMappingKind_None, 255 },
        .y = { EMappingKind_None, 255 },
        .back = { EMappingKind_None, 255 },
        .guide = { EMappingKind_None, 255 },
        .start = { EMappingKind_None, 255 },
        .leftstick = { EMappingKind_None, 255 },    // push left stick
        .rightstick = { EMappingKind_None, 255 },
        .leftshoulder = { EMappingKind_None, 255 },
        .rightshoulder = { EMappingKind_None, 255 },
        .dpup = { EMappingKind_None, 255 },
        .dpdown = { EMappingKind_None, 255 },
        .dpleft = { EMappingKind_None, 255 },
        .dpright = { EMappingKind_None, 255 },
        .misc1 = { EMappingKind_None, 255 },
        .paddle1 = { EMappingKind_None, 255 },
        .paddle2 = { EMappingKind_None, 255 },
        .paddle3 = { EMappingKind_None, 255 },
        .paddle4 = { EMappingKind_None, 255 },
        .leftx = { EMappingKind_Axis, 0 },
        .lefty = { EMappingKind_Axis, 1 },
        .rightx = { EMappingKind_Axis, 2 },
        .righty = { EMappingKind_Axis, 3 },
        .lefttrigger = { EMappingKind_None, 255 },
        .righttrigger = { EMappingKind_None, 255 },
    };
    return SDL_TRUE;
}

SDL_JoystickDriver SDL_GK_JoystickDriver = {
    .Init = GK_JoystickInit,
    .GetCount = GK_JoystickGetCount,
    .Detect = GK_JoystickDetect,
    .GetDeviceName = GK_JoystickGetDeviceName,
    .GetDevicePath = GK_JoystickGetDevicePath,
    .GetDevicePlayerIndex = GK_JoystickGetDevicePlayerIndex,
    .SetDevicePlayerIndex = GK_JoystickSetDevicePlayerIndex,
    .GetDeviceGUID = GK_JoystickGetDeviceGUID,
    .GetDeviceInstanceID = GK_JoystickGetDeviceInstanceID,
    .Open = GK_JoystickOpen,
    .Rumble = GK_JoystickRumble,
    .RumbleTriggers = GK_JoystickRumbleTriggers,
    .GetCapabilities = GK_JoystickGetCapabilities,
    .SetLED = GK_JoystickSetLED,
    .SendEffect = GK_JoystickSendEffect,
    .SetSensorsEnabled = GK_JoystickSetSensorsEnabled,
    .Update = GK_JoystickUpdate,
    .Close = GK_JoystickClose,
    .Quit = GK_JoystickQuit,
    .GetGamepadMapping = GK_JoystickGetGamepadMapping
};

#endif

