#include <squirrel.h>
#include <lvgl.h>
#include <lvgl/src/drivers/lv_drivers.h>
#include "game.h"

HSQUIRRELVM v;

std::vector<Game> games;

static void init_games();
static void game_click(lv_event_t *e);

SQInteger file_lexfeedASCII(SQUserPointer file)
{
    int ret;
    char c;
    if( ( ret=fread(&c,sizeof(c),1,(FILE *)file )>0) )
        return c;
    return 0;
}

int compile_file(HSQUIRRELVM v,const char *filename)
{
    FILE *f=fopen(filename,"rb");
    if(f)
    {
         sq_compile(v,file_lexfeedASCII,f,filename,1);
         fclose(f);
         return 1;
    }
    return 0;
}

int main()
{
    v = sq_open(1024);
    sq_close(v);

    lv_init();

    auto display = lv_gk_display_create();
    lv_display_set_default(display);
    auto kbd = lv_gk_kbd_create();
    auto mouse = lv_gk_mouse_create();

    auto grp = lv_group_create();
    lv_indev_set_group(kbd, grp);

    auto cursor = lv_label_create(lv_screen_active());
    lv_label_set_text(cursor, "<");
    lv_indev_set_cursor(mouse, cursor);

    init_games();       // example list for now
    /* populate games list */
    auto list = lv_list_create(lv_screen_active());

    for(auto &g : games)
    {
        auto lbtn = lv_list_add_button(list, NULL, g.name.c_str());
        lv_obj_add_event_cb(lbtn, game_click, LV_EVENT_CLICKED, &g);
        lv_group_add_obj(grp, lbtn);
    }

    while(1)
    {
        usleep(lv_timer_handler() * 1000);
    }

    return 0;
}

void game_click(lv_event_t *e)
{
    auto g = reinterpret_cast<const Game *>(e->param);
    g->Load();
}

void init_games()
{
    // some test games
    Game g_sonic;
    g_sonic.name = "Sonic the Hedgehog";
    g_sonic.desc = "Sega Master System version";
    g_sonic.fname = "/mednafen-gk/bin/mednafen";
    g_sonic.cwd = "/mednafen-gk";
    g_sonic.args.push_back("-video.driver");
    g_sonic.args.push_back("softfb");
    g_sonic.args.push_back("-video.fs");
    g_sonic.args.push_back("1");
    g_sonic.args.push_back("-sound");
    g_sonic.args.push_back("0");
    g_sonic.args.push_back("-sound.rate");
    g_sonic.args.push_back("22050");
    g_sonic.args.push_back("-gb.stretch");
    g_sonic.args.push_back("aspect_int");
    g_sonic.args.push_back("-osd.alpha_blend");
    g_sonic.args.push_back("0");
    g_sonic.args.push_back("-video.blit_timesync");
    g_sonic.args.push_back("0");
    g_sonic.args.push_back("-fps.autoenable");
    g_sonic.args.push_back("1");
    g_sonic.args.push_back("-fps.scale");
    g_sonic.args.push_back("2");
    g_sonic.args.push_back("-md.videoip");
    g_sonic.args.push_back("0");
    g_sonic.args.push_back("/usr/share/mednafen/games/Sonic The Hedgehog (UE) [!].sms");

    games.push_back(g_sonic);

    Game g_cvania;
    g_cvania.name = "Castlevania";
    g_cvania.desc = "NES version";
    g_cvania.fname = "/mednafen-gk/bin/mednafen";
    g_cvania.cwd = "/mednafen-gk";
    g_cvania.args.push_back("-video.driver");
    g_cvania.args.push_back("softfb");
    g_cvania.args.push_back("-video.fs");
    g_cvania.args.push_back("1");
    g_cvania.args.push_back("-sound");
    g_cvania.args.push_back("0");
    g_cvania.args.push_back("-sound.rate");
    g_cvania.args.push_back("22050");
    g_cvania.args.push_back("-gb.stretch");
    g_cvania.args.push_back("aspect_int");
    g_cvania.args.push_back("-osd.alpha_blend");
    g_cvania.args.push_back("0");
    g_cvania.args.push_back("-video.blit_timesync");
    g_cvania.args.push_back("0");
    g_cvania.args.push_back("-fps.autoenable");
    g_cvania.args.push_back("1");
    g_cvania.args.push_back("-fps.scale");
    g_cvania.args.push_back("2");
    g_cvania.args.push_back("-md.videoip");
    g_cvania.args.push_back("0");
    g_cvania.args.push_back("/usr/share/mednafen/games/Castlevania (E).nes");

    games.push_back(g_cvania);
}
