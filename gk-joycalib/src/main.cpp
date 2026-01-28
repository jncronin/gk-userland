#include <SDL2/SDL.h>
#include <list>
#include <gk.h>

struct pt { int16_t x; int16_t y; };

const unsigned int scr_w = 640;
const unsigned int scr_h = 480;

static void do_calib(unsigned int axis_pair, const std::list<pt> &raw_pts);

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
    std::list<pt> rawpt[2];
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

            if(ev.type == SDL_JOYBUTTONDOWN)
            {
                if(ev.jbutton.button < 2)
                {
                    do_calib(ev.jbutton.button, rawpt[ev.jbutton.button]);
                }
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

            if(stick < 2)
            {
                int raw_x, raw_y;
                GK_GetJoystickAxesEx(stick, &raw_x, &raw_y, true);
                rawpt[stick].push_front({ .x = (int16_t)raw_x, .y = (int16_t)raw_y });
                while(rawpt[stick].size() >= 256)
                {
                    rawpt[stick].pop_back();
                }
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

static void do_calib(unsigned int axis_pair, const std::list<pt> &raw_pts)
{
    // centre is most recent
    auto centre = *raw_pts.begin();

    // get top/bottom/left/right etc
    int16_t top = -32767;
    int16_t right = -32767;
    int16_t bottom = 32767;
    int16_t left = 32767;

    for(const auto &p : raw_pts)
    {
        if(p.x < left)
            left = p.x;
        if(p.x > right)
            right = p.x;
        if(p.y > top)
            top = p.y;
        if(p.y < bottom)
            bottom = p.y;
    }

    GK_SetJoystickCalibration(axis_pair, left, right, top, bottom, (int)centre.x, (int)centre.y);
}
