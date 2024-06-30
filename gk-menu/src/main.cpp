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

    auto grp = lv_group_create();
    lv_indev_set_group(kbd, grp);

    init_games();       // example list for now

    /* populate games list */
    auto list = lv_obj_create(lv_screen_active());
    lv_obj_set_size(list, LV_PCT(100), LV_PCT(100));
    lv_obj_set_flex_flow(list, LV_FLEX_FLOW_COLUMN);

    for(int i = 0; i < games.size(); i++)
    {
        const auto &g = games[i];

        auto lbtn = lv_button_create(list);
        lv_obj_set_flex_flow(lbtn, LV_FLEX_FLOW_COLUMN);
        lv_obj_add_event_cb(lbtn, game_click, LV_EVENT_CLICKED, (void *)i);
        lv_obj_set_size(lbtn, LV_PCT(100), LV_SIZE_CONTENT);
        lv_group_add_obj(grp, lbtn);

        auto lbtn_text = lv_label_create(lbtn);
        lv_obj_set_style_text_font(lbtn_text, &lv_font_montserrat_24, 0);
        lv_obj_set_size(lbtn_text, LV_PCT(100), LV_SIZE_CONTENT);
        lv_label_set_text(lbtn_text, g.name.c_str());

        auto lbtn_row2_cont = lv_obj_create(lbtn);
        lv_obj_set_flex_flow(lbtn_row2_cont, LV_FLEX_FLOW_ROW);
        lv_obj_set_size(lbtn_row2_cont, LV_PCT(100), LV_SIZE_CONTENT);
        lv_obj_set_flex_align(lbtn_row2_cont, LV_FLEX_ALIGN_SPACE_BETWEEN,
            LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_START);

        auto lbtn_extra_text = lv_label_create(lbtn_row2_cont);
        lv_obj_set_height(lbtn_extra_text, LV_SIZE_CONTENT);
        lv_obj_set_flex_grow(lbtn_extra_text, 1);
        lv_label_set_text(lbtn_extra_text, g.desc.c_str());

        auto limg = lv_obj_create(lbtn_row2_cont);
        lv_obj_set_size(limg, 160, 120);
    }

    while(1)
    {
        usleep(lv_timer_handler() * 1000);
    }

    return 0;
}

void game_click(lv_event_t *e)
{
    const auto &g = games[(int)e->user_data];
    g.Load();
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

    Game g_blight;
    g_blight.name = "Blue Lightning";
    g_blight.desc = "Lorem ipsum dolor sit amet, consectetur adipiscing elit. Aenean facilisis odio ac lorem ultricies, eget viverra massa laoreet. Morbi feugiat ullamcorper tortor ac efficitur. In viverra egestas lectus laoreet facilisis. Vestibulum tempus diam sapien, sit amet placerat massa posuere nec. Vestibulum in augue arcu. Quisque nec risus non ligula finibus dapibus. Curabitur vitae scelerisque velit. Vestibulum interdum molestie justo eu viverra. Quisque molestie massa eu ante sagittis, vitae dictum leo varius. Quisque non leo feugiat, pellentesque nunc ac, malesuada libero. Nam quis lacus nec massa sodales bibendum. Pellentesque id tincidunt libero. Donec tincidunt nisl elementum tortor congue volutpat.";
    g_blight.fname = "/mednafen-gk/bin/mednafen";
    g_blight.cwd = "/mednafen-gk";
    g_blight.args.push_back("-video.driver");
    g_blight.args.push_back("softfb");
    g_blight.args.push_back("-video.fs");
    g_blight.args.push_back("1");
    g_blight.args.push_back("-sound");
    g_blight.args.push_back("0");
    g_blight.args.push_back("-sound.rate");
    g_blight.args.push_back("22050");
    g_blight.args.push_back("-gb.stretch");
    g_blight.args.push_back("aspect_int");
    g_blight.args.push_back("-osd.alpha_blend");
    g_blight.args.push_back("0");
    g_blight.args.push_back("-video.blit_timesync");
    g_blight.args.push_back("0");
    g_blight.args.push_back("-fps.autoenable");
    g_blight.args.push_back("1");
    g_blight.args.push_back("-fps.scale");
    g_blight.args.push_back("2");
    g_blight.args.push_back("-md.videoip");
    g_blight.args.push_back("0");
    g_blight.args.push_back("/usr/share/mednafen/games/Blue Lightning (1989) [a1].lnx");

    games.push_back(g_blight);

}
