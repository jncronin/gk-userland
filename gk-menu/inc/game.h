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

        bool screen_ignore_vsync = true;

        size_t stack_size = 64*1024;
        size_t heap_size = 32*1024*1024;
        size_t graphics_texture_size = 0;

        prockeymap_t keymap = { 0 };

        void Load();
        Game();
};


#endif
