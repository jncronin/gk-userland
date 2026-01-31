#ifndef GAME_H
#define GAME_H

#include <string>
#include <vector>

#include <gk.h>

class Game
{
    public:
        std::string name;
        std::string desc;

        std::string fname;
        std::vector<std::string> args;
        std::string cwd;
        std::string img = "img/joystick.png";
        std::string osd = "";
        std::string osd_text = "";

        int screen_w = 640;
        int screen_h = 320;
        int screen_pf = GK_PIXELFORMAT_RGB565;
        int screen_refresh = 60;

        bool screen_ignore_vsync = true;
        int screen_update_method = GK_SCREEN_UPDATE_FULL;

        size_t stack_size = 64*1024;
        size_t heap_size = 32*1024*1024;
        size_t graphics_texture_size = 0;

        prockeymap_t keymap = { 0 };

        void Load();
        Game();
};


#endif
