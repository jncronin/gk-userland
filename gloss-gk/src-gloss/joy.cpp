#include "_gk_memaddrs.h"
#include "syscalls.h"
#include "deferred.h"
#include <cstdint>

#if __GAMEKID__ >= 4
extern "C" int GK_GetJoystickAxes(int *x, int *y)
{
    if(x) *x = (int)*(volatile int16_t *)(GK_JOYSTICK_ADDRESS);
    if(y) *y = (int)*(volatile int16_t *)(GK_JOYSTICK_ADDRESS + 4);
    return 0;
}

extern "C" int GK_GetJoystickBAxes(int *x, int *y)
{
    if(x) *x = (int)*(volatile int16_t *)(GK_JOYSTICKB_ADDRESS);
    if(y) *y = (int)*(volatile int16_t *)(GK_JOYSTICKB_ADDRESS + 4);
    return 0;
}

extern "C" int GK_GetTiltAxes(int *x, int *y)
{
    if(x) *x = (int)*(volatile int *)(GK_TILT_ADDRESS);
    if(y) *y = (int)*(volatile int *)(GK_TILT_ADDRESS + 4);
    return 0;
}

extern "C" int GK_GetJoystickAxesEx(unsigned int axis_pair, int *x, int *y, int is_raw)
{
    if(axis_pair >= 3 || (is_raw && axis_pair >= 2))
    {
        return -1;
    }

    uintptr_t pair_start = 0;
    switch(axis_pair)
    {
        case 0:
            if(is_raw)
            {
                pair_start = GK_JOYSTICK_RAW_ADDRESS;
            }
            else
            {
                pair_start = GK_JOYSTICK_ADDRESS;
            }
            break;
        case 1:
            if(is_raw)
            {
                pair_start = GK_JOYSTICKB_RAW_ADDRESS;
            }
            else
            {
                pair_start = GK_JOYSTICKB_ADDRESS;
            }
            break;
        case 2:
            pair_start = GK_TILT_ADDRESS;
            break;
    }

    if(x) *x = (int)*(volatile int16_t *)(pair_start);
    if(y) *y = (int)*(volatile int16_t *)(pair_start + 4);

    return 0;
}

extern "C" int GK_SetJoystickCalibration(unsigned int axis_pair,
    int left, int right, int top, int bottom, int middle_x, int middle_y)
{
    if(axis_pair >= 2)
    {
        return -1;
    }

    struct __syscall_joystick_calib_params p
    {
        .axis_pair = axis_pair,
        .left = left,
        .right = right,
        .top = top,
        .bottom = bottom,
        .middle_x = middle_x,
        .middle_y = middle_y
    };
    return deferred_call(__syscall_joystick_calib, &p);
}
#else
extern "C" int GK_GetJoystickAxes(int *x, int *y)
{
    /* axes are 0-65535 from gk, reintepret as signed int for SDL2 */
    if(x) *x = *(volatile int *)(GK_JOYSTICK_ADDRESS) - 32768;
    if(y) *y = *(volatile int *)(GK_JOYSTICK_ADDRESS + 4) - 32768;
    return 0;
}

extern "C" int GK_GetTiltAxes(int *x, int *y)
{
    /* axes are -32768 to 32767 so report directly */
    if(x) *x = *(volatile int *)(GK_TILT_ADDRESS);
    if(y) *y = *(volatile int *)(GK_TILT_ADDRESS + 4);
    return 0;
}
#endif
