#include <SDL2/SDL.h>
#include <list>

struct pt { int16_t x; int16_t y; };

const unsigned int scr_w = 640;
const unsigned int scr_h = 480;

int main()
{
    SDL_Init(SDL_INIT_VIDEO | SDL_INIT_JOYSTICK);

    SDL_Window *w;
    SDL_Renderer *r;
    SDL_CreateWindowAndRenderer(scr_w, scr_h,
        SDL_WINDOW_FULLSCREEN,
        &w, &r);

    SDL_Joystick *j;
    j = SDL_JoystickOpen(0);

    printf("gkjoy: naxes: %u, nbuttons: %u\n",
        SDL_JoystickNumAxes(j),
        SDL_JoystickNumButtons(j));
    
    SDL_Event ev;
    bool quit = false;

    std::list<pt> jpt[3];
    auto nsticks = std::min(3U, (unsigned int)SDL_JoystickNumAxes(j));

    while(!quit)
    {
        while(SDL_PollEvent(&ev))
        {
            if(ev.type == SDL_QUIT)
            {
                quit = true;
                break;
            }
        }

        if(quit)
            break;

        for(auto stick = 0U; stick < nsticks; stick++)
        {
            auto x = SDL_JoystickGetAxis(j, stick * 2 + 0);
            auto y = SDL_JoystickGetAxis(j, stick * 2 + 1);

            jpt[stick].push_front({ .x = x, .y = y });
            while(jpt[stick].size() >= 256)
            {
                jpt[stick].pop_back();
            }
        }

        SDL_SetRenderDrawColor(r, 0, 0, 0, 0);
        SDL_RenderClear(r);

        for(auto stick = 0U; stick < nsticks; stick++)
        {
            auto intens = 255;
            unsigned int last_x = ~0U;
            unsigned int last_y = ~0U;
            for(const auto &p : jpt[stick])
            {
                SDL_SetRenderDrawColor(r, 
                    (stick == 0) ? intens : 0,
                    (stick == 1) ? intens : 0,
                    (stick == 2) ? intens : 0, 255);

                auto scr_x = (((int)p.x + 32768) * scr_w) / 65536;
                auto scr_y = (((int)p.y + 32768) * scr_h) / 65536;

                if(last_x != ~0U && last_y != ~0U)
                    SDL_RenderDrawLine(r, last_x, last_y, scr_x, scr_y);
                last_x = scr_x;
                last_y = scr_y;

                if(intens > 0)
                    intens--;
            }
        }

        SDL_RenderPresent(r);

        SDL_Delay(20);
    }

    return 0;
}
