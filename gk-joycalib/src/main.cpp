#include <SDL2/SDL.h>
#include <list>
#include <gk.h>

struct pt { int16_t x; int16_t y; };

const unsigned int scr_w = 640;
const unsigned int scr_h = 480;

static void do_calib(unsigned int axis_pair, const std::list<pt> &raw_pts);
static void draw_thickline(SDL_Renderer * renderer, int x1, int y1, int x2, int y2, int thickness = 3);
static int joy_to_screen(int v, unsigned int scr_size);
static int joy_to_screen(double v, unsigned int scr_size);
static int joyx_to_screen(int v);
static int joyy_to_screen(int v);
static int joyx_to_screen(double v);
static int joyy_to_screen(double v);

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

    std::list<pt> jpt[4];
    std::list<pt> rawpt[4];
    auto nsticks = std::min(4U, 4U /* TODO: update SDL (unsigned int)SDL_JoystickNumAxes(j) */);

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

            int raw_x, raw_y;
            GK_GetJoystickAxesEx(stick, &raw_x, &raw_y, true);
            rawpt[stick].push_front({ .x = (int16_t)raw_x, .y = (int16_t)raw_y });
            while(rawpt[stick].size() >= 256)
            {
                rawpt[stick].pop_back();
            }
        }

        SDL_SetRenderDrawColor(r, 0, 0, 0, 0);
        SDL_RenderClear(r);

        /* Draw digital bounding boxes */
        // deadzone hard coded at 8000 - TODO: read from kernel somehow
        const double deadzone = 8000.0;
        const int circle_segments = 32;
        SDL_Point cs[circle_segments + 1];
        for(auto ccs = 0; ccs < circle_segments; ccs++)
        {
            auto ang = 2.0 * (double)M_PI / (double)circle_segments * (double)ccs;
            auto x = (int)std::round((deadzone * std::cos(ang) + 32768.0) * (double)scr_w / 65536.0);
            auto y = (int)std::round((deadzone * std::sin(ang) + 32768.0) * (double)scr_h / 65536.0);
            cs[ccs].x = joyx_to_screen(deadzone * std::cos(ang));
            cs[ccs].y = joyy_to_screen(deadzone * std::sin(ang));
        }
        cs[circle_segments].x = joyx_to_screen(deadzone);
        cs[circle_segments].y = joyy_to_screen(0);
        SDL_SetRenderDrawColor(r, 255, 255, 255, 255);
        SDL_RenderDrawLines(r, cs, circle_segments + 1);

        for(auto ang_i = -7; ang_i <= 7; ang_i += 2)
        {
            auto ang = (double)ang_i * (double)M_PI / 8.0;
            SDL_RenderDrawLine(r,
                joyx_to_screen(deadzone * cos(ang)),
                joyy_to_screen(deadzone * sin(ang)),
                joyx_to_screen(32768.0 * cos(ang)),
                joyy_to_screen(32768.0 * sin(ang)));
        }

        for(auto stick = 0U; stick < nsticks; stick++)
        {
            auto intens = 255;
            unsigned int last_x = ~0U;
            unsigned int last_y = ~0U;
            for(const auto &p : jpt[stick])
            {
                SDL_SetRenderDrawColor(r, 
                    (stick == 0 || stick == 3) ? intens : 0,
                    (stick == 1) ? intens : 0,
                    (stick == 2 || stick == 3) ? intens : 0, 255);

                auto scr_x = (((int)p.x + 32768) * scr_w) / 65536;
                auto scr_y = (((int)p.y + 32768) * scr_h) / 65536;

                if(last_x != ~0U && last_y != ~0U)
                    draw_thickline(r, last_x, last_y, scr_x, scr_y);
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

static void draw_thickline(SDL_Renderer * renderer, int x1, int y1, int x2, int y2, int thickness)
{
    for(int y = -thickness/2; y <= -thickness/2 + thickness; y++)
    {
        for(int x = -thickness/2; x <= -thickness/2 + thickness; x++)
        {
            SDL_RenderDrawLine(renderer, x1 + x, y1 + y, x2 + x, y2 + y);
        }
    }
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

static int joy_to_screen(int v, unsigned int scr)
{
    return ((v + 32768) * (int)scr) / 65536;
}

static int joy_to_screen(double v, unsigned int scr)
{
    return (int)std::round((v + 32768.0) * (double)scr / 65536.0);
}

static inline int joyx_to_screen(int v)
{
    return joy_to_screen(v, scr_w);
}

static inline int joyy_to_screen(int v)
{
    return joy_to_screen(v, scr_h);
}

static inline int joyx_to_screen(double v)
{
    return joy_to_screen(v, scr_w);
}

static inline int joyy_to_screen(double v)
{
    return joy_to_screen(v, scr_h);
}
