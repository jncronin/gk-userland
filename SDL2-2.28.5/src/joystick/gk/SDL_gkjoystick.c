#include "../../SDL_internal.h"

#ifdef SDL_JOYSTICK_GK

#include <gk.h>

#include "../SDL_sysjoystick.h"
#include "SDL_events.h"

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
    joystick->nbuttons = 0;
    joystick->naxes = 4;
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
    int jx, jy, tx, ty;
    GK_GetJoystickAxes(&jx, &jy);
    GK_GetTiltAxes(&tx, &ty);

    SDL_PrivateJoystickAxis(joystick, 0, jx);
    SDL_PrivateJoystickAxis(joystick, 1, jy);
    SDL_PrivateJoystickAxis(joystick, 2, tx);
    SDL_PrivateJoystickAxis(joystick, 3, ty);
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
        .rightshoulder = { EMappingKind_None, 255 },
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

