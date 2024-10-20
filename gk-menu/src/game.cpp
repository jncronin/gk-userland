#include "game.h"
#include <gk.h>
#include <cstring>
#include <sys/wait.h>

void Game::Load() const
{
    proccreate_t pcinfo;

    memset(&pcinfo, 0, sizeof(proccreate_t));

    pcinfo.cwd = cwd.c_str();
    pcinfo.argc = args.size();
    pcinfo.argv = (const char **)malloc(pcinfo.argc * sizeof(const char *));
    for(int i = 0; i < pcinfo.argc; i++)
    {
        pcinfo.argv[i] = args[i].c_str();
    }
    pcinfo.heap_size = heap_size;
    pcinfo.stack_size = stack_size;
    pcinfo.screen_w = screen_w;
    pcinfo.screen_h = screen_h;
    pcinfo.screen_ignore_vsync = screen_ignore_vsync;
    pcinfo.pixel_format = screen_pf;
    pcinfo.with_focus = 1;
    pcinfo.keymap = keymap;

    pid_t cpid;
    if(GK_CreateProcess(fname.c_str(), &pcinfo, &cpid) < 0)
    {
        printf("process creation failed %d\n", errno);
    }
    else
    {
        int retno;
        waitpid(cpid, &retno, 0);
        printf("process returned %d\n", retno);
    }
}

Game::Game()
{
    /* keymap defaults */
    keymap.gamepad_is_joystick = 0;
    keymap.gamepad_is_keyboard = 1;
    keymap.gamepad_is_mouse = 0;

    /* default to mednafen NES */
    keymap.gamepad_to_scancode[GK_KEYLEFT] = GK_SCANCODE_A;
    keymap.gamepad_to_scancode[GK_KEYRIGHT] = GK_SCANCODE_D;
    keymap.gamepad_to_scancode[GK_KEYUP] = GK_SCANCODE_W;
    keymap.gamepad_to_scancode[GK_KEYDOWN] = GK_SCANCODE_S;
    keymap.gamepad_to_scancode[GK_KEYA] = GK_SCANCODE_KP_2;
    keymap.gamepad_to_scancode[GK_KEYB] = GK_SCANCODE_KP_3;
    keymap.gamepad_to_scancode[GK_KEYX] = GK_SCANCODE_RETURN;
    keymap.gamepad_to_scancode[GK_KEYY] = GK_SCANCODE_TAB;
}