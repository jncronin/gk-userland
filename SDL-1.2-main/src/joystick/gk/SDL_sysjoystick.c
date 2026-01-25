/*
    SDL - Simple DirectMedia Layer
    Copyright (C) 1997-2012 Sam Lantinga

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Lesser General Public
    License as published by the Free Software Foundation; either
    version 2.1 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Lesser General Public License for more details.

    You should have received a copy of the GNU Lesser General Public
    License along with this library; if not, write to the Free Software
    Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA

    Sam Lantinga
    slouken@libsdl.org
*/
#include "SDL_config.h"

/* This is the system specific header for the SDL joystick API */

#include "SDL_joystick.h"
#include "../SDL_sysjoystick.h"
#include "../SDL_joystick_c.h"
#include "SDL_events.h"

#include "gk.h"

static uint32_t oldbuttons = 0;
static int old_axes[6] = { 0 };

/* Function to scan the system for joysticks.
 * This function should set SDL_numjoysticks to the number of available
 * joysticks.  Joystick 0 should be the system default joystick.
 * It should return 0, or -1 on an unrecoverable fatal error.
 */
int SDL_SYS_JoystickInit(void)
{
	SDL_numjoysticks = 1;
	return(1);
}

/* Function to get the device-dependent name of a joystick */
const char *SDL_SYS_JoystickName(int index)
{
	return "GK_Joystick";
}

/* Function to open a joystick for use.
   The joystick to open is specified by the index field of the joystick.
   This should fill the nbuttons and naxes fields of the joystick structure.
   It returns 0, or -1 if there is an error.
 */
int SDL_SYS_JoystickOpen(SDL_Joystick *joystick)
{
#if __GAMEKID__ >= 4
    //joystick->naxes = 6;
    joystick->naxes = 2;
    joystick->nhats = 0;
    joystick->nballs = 0;
    //joystick->nbuttons = GK_NUMKEYS;
    joystick->nbuttons = 0;
#endif
	return 0;
}

/* Function to update the state of a joystick - called as a device poll.
 * This function shouldn't update the joystick structure directly,
 * but instead should call SDL_PrivateJoystick*() to deliver events
 * and update joystick device state.
 */
void SDL_SYS_JoystickUpdate(SDL_Joystick *joystick)
{
#if __GAMEKID__ >= 4
    const int delta = 50;
    int cur_axes[6];
    uint32_t buttons;

    GK_GetJoystickAxes(&cur_axes[0], &cur_axes[1]);
    GK_GetJoystickBAxes(&cur_axes[2], &cur_axes[3]);
    GK_GetTiltAxes(&cur_axes[4], &cur_axes[5]);

    for(unsigned int axis = 0; axis < joystick->naxes; axis++)
    {
        if((cur_axes[axis] < old_axes[axis] - delta) ||
            (cur_axes[axis] > old_axes[axis] + delta))
        {
            SDL_PrivateJoystickAxis(joystick, axis, cur_axes[axis]);
            old_axes[axis] = cur_axes[axis];
        }
    }

    buttons = *(uint32_t *)GK_KEYSTATE_ADDRESS;
    for(unsigned int i = 0; i < joystick->nbuttons; i++)
    {
        uint32_t mask = 1UL << i;
        if((buttons & mask) && !(oldbuttons & mask))
        {
            SDL_PrivateJoystickButton(joystick, i, SDL_PRESSED);
        }
        else if(!(buttons & mask) && (oldbuttons & mask))
        {
            SDL_PrivateJoystickButton(joystick, i, SDL_RELEASED);
        }
    }
    oldbuttons = buttons;
#endif

	return;
}

/* Function to close a joystick after use */
void SDL_SYS_JoystickClose(SDL_Joystick *joystick)
{
	return;
}

/* Function to perform any system-specific joystick related cleanup */
void SDL_SYS_JoystickQuit(void)
{
	return;
}

