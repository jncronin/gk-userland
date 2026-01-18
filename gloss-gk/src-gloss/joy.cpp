#include "_gk_memaddrs.h"

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
