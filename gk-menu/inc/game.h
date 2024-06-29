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

        int screen_x = 640;
        int screen_y = 320;
        int screen_pf = GK_PIXELFORMAT_RGB565;

        bool screen_ignore_vsync;

        size_t stack_size = 64*1024;

        void Load() const;
};


#endif
