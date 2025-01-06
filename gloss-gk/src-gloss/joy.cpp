#include "_gk_memaddrs.h"

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
