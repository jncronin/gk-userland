#include <squirrel.h>
#include <lvgl.h>
#include <lvgl/src/drivers/lv_drivers.h>
#include "game.h"
#include <algorithm>
#include <png.h>
#include <img.h>
#include <SDL.h>
#include <SDL_mixer.h>
#include <cstdio>
#include <unistd.h>

std::vector<Game> games;

static lv_obj_t *list;

int load_games();
static void game_click(lv_event_t *e);
static void btn_focus(lv_event_t *e);
int nrefresh = 0;

// black background for loading
const uint8_t bbg_data[640*480*3] = { 0 };
const lv_image_dsc_t bbg {
    .header {
        .magic = LV_IMAGE_HEADER_MAGIC,
        .cf = LV_COLOR_FORMAT_RGB888,
        .flags = 0,
        .w = 640,
        .h = 480,
        .stride = 640*3,
    },
    .data_size = 640*480*3,
    .data = (const uint8_t *)bbg_data
};

int main(int argc, char *argv[])
{
    lv_init();

    auto display = lv_gk_display_create();
    lv_display_set_default(display);

    // show black background with loading text
    list = lv_img_create(lv_screen_active());    
    lv_obj_set_size(list, lv_display_get_horizontal_resolution(display), lv_display_get_vertical_resolution(display));
    lv_img_set_src(list, &bbg);
    //lv_obj_add_style(list, &style_bg, 0);
    //lv_obj_set_style_radius(list, 0, 0);

    auto h_scale = 640 / lv_display_get_horizontal_resolution(display);
    auto v_scale = 480 / lv_display_get_vertical_resolution(display);

    lv_freetype_init(LV_FREETYPE_CACHE_FT_GLYPH_CNT);
    const lv_font_t *load_font = lv_freetype_font_create("gkmenu-start-font.ttf",
        LV_FREETYPE_FONT_RENDER_MODE_BITMAP, 48/v_scale,
        LV_FREETYPE_FONT_STYLE_BOLD);
    if(!load_font)
    {
        auto f = fopen("gkmenu-start-font.ttf", "rb");
        if(f)
        {
            fseek(f, 0L, SEEK_END);
            auto flen = ftell(f);
            rewind(f);

            auto fnt = new char[flen];
            if(fnt)
            {
                if(fread(fnt, 1, flen, f) == flen)
                {
                    load_font = lv_tiny_ttf_create_data(fnt, flen, 48/v_scale);
                }
            }
        }
    }
    if(!load_font)
        load_font = &lv_font_montserrat_48;

    auto load_text = lv_label_create(list);
    lv_obj_set_style_text_font(load_text, load_font, 0);
    lv_obj_set_style_text_color(load_text, lv_color_white(), 0);
    lv_label_set_text(load_text, "READY PLAYER 1");
    lv_obj_set_width(load_text, lv_display_get_horizontal_resolution(display));
    lv_obj_set_style_text_align(load_text, LV_TEXT_ALIGN_CENTER, 0);
    lv_obj_set_pos(load_text, 0, 200/v_scale);

    lv_obj_invalidate(lv_scr_act());
    lv_timer_handler();

    SDL_Init(SDL_INIT_AUDIO);

    if(Mix_OpenAudio(22050, MIX_DEFAULT_FORMAT, 2, 1024) == 0)
    {
        auto intro_music = Mix_LoadMUS("gkmenu-start.mp3");
        if(intro_music)
        {
            if(Mix_PlayMusic(intro_music, 0) == -1)
            {
                Mix_FreeMusic(intro_music);
                fprintf(stderr, "gkmenu: Mix_PlayMusic failed\n");
            }
            else
            fprintf(stderr, "gkmenu: epic intro music playing\n");
        }
        else
        {
            fprintf(stderr, "gkmenu: Mix_PlayMusic failed\n");
        }
    }
    else
    {
        fprintf(stderr, "gkmenu: Mix_OpenAudio failed\n");
    }

    auto bg_img = get_img("gkmenu-background.png");
    if(bg_img)
    {
        lv_img_set_src(list, bg_img);
        lv_img_set_zoom(list, 256 / h_scale);
        lv_obj_invalidate(lv_scr_act());
        lv_timer_handler();
    }


    auto kbd = lv_gk_kbd_create();

    auto grp = lv_group_create();
    lv_indev_set_group(kbd, grp);
    lv_group_set_wrap(grp, false);

    auto touch = lv_gk_mouse_create();

    load_games();
    std::sort(games.begin(), games.end(),
        [](const Game &ga, const Game &gb)
        {
            const auto &a = ga.name;
            const auto &b = gb.name;

            const auto result = std::mismatch(a.cbegin(), a.cend(),
                b.cbegin(), b.cend(),
                [](const char ca, const char cb)
                {
                    return std::tolower(ca) == tolower(cb);
                });

            return result.second != b.cend() &&
                (result.first == a.cend() || 
                std::tolower(*result.first) < tolower(*result.second));
        });
            

    /* blank style for container objects */
    lv_style_t style_cont;
    lv_style_init(&style_cont);
    lv_style_set_bg_opa(&style_cont, LV_OPA_TRANSP);
    //lv_style_set_opa(&style_cont, LV_OPA_TRANSP);
    lv_style_set_border_width(&style_cont, 0);
    lv_style_set_border_opa(&style_cont, LV_OPA_TRANSP);

    /* text styles */
    lv_style_t style_text;
    lv_style_init(&style_text);
    lv_style_set_opa(&style_text, LV_OPA_100);
    lv_style_set_text_color(&style_text, lv_color_white());

    lv_color_t neon_colors[] = {
        lv_color_make(77, 238, 234),
        lv_color_make(116, 238, 21),
        lv_color_make(255, 231, 0),
        lv_color_make(240, 0, 255),
        lv_color_make(0, 30, 255)
    };
    constexpr auto n_neon_colors = sizeof(neon_colors) / sizeof(neon_colors[0]);

    for(int i = 0; i < games.size(); i++)
    {
        const auto &g = games[i];

        auto lbtn = lv_button_create(list);
        lv_obj_set_flex_flow(lbtn, LV_FLEX_FLOW_COLUMN);
        lv_obj_add_event_cb(lbtn, game_click, LV_EVENT_CLICKED, (void *)i);
        lv_obj_set_size(lbtn, LV_PCT(100), LV_SIZE_CONTENT);

        auto col_id = i % n_neon_colors;
        auto col = neon_colors[col_id];
        lv_obj_set_style_bg_color(lbtn, lv_color_black(), 0);
        lv_obj_set_style_bg_opa(lbtn, LV_OPA_TRANSP, 0);
        lv_obj_set_style_shadow_color(lbtn, lv_color_black(), 0);
        lv_obj_set_style_shadow_width(lbtn, 0, 0);
        lv_obj_set_style_outline_width(lbtn, 8/h_scale, 0);
        lv_obj_set_style_outline_pad(lbtn, -8/h_scale, 0);
        lv_obj_set_style_outline_color(lbtn, col, 0);
        lv_obj_set_style_outline_color(lbtn, col, LV_STATE_FOCUSED);
        lv_obj_set_style_outline_color(lbtn, col, LV_STATE_FOCUS_KEY);
        lv_obj_set_style_outline_opa(lbtn, LV_OPA_50, 0);
        lv_obj_set_style_outline_opa(lbtn, LV_OPA_100, LV_STATE_FOCUSED);
        lv_obj_set_style_outline_opa(lbtn, LV_OPA_100, LV_STATE_FOCUS_KEY);

        lv_obj_add_event_cb(lbtn, btn_focus, LV_EVENT_FOCUSED, (void *)lv_color_to_u32(col));

        lv_group_add_obj(grp, lbtn);

        auto lbtn_text = lv_label_create(lbtn);
        lv_obj_set_style_text_font(lbtn_text, &lv_font_montserrat_24, 0);
        lv_obj_set_size(lbtn_text, LV_PCT(100), LV_SIZE_CONTENT);
        lv_label_set_text(lbtn_text, g.name.c_str());
        lv_obj_add_style(lbtn_text, &style_text, 0);

        auto lbtn_row2_cont = lv_obj_create(lbtn);
        lv_obj_set_flex_flow(lbtn_row2_cont, LV_FLEX_FLOW_ROW);
        lv_obj_set_size(lbtn_row2_cont, LV_PCT(100), LV_SIZE_CONTENT);
        lv_obj_set_flex_align(lbtn_row2_cont, LV_FLEX_ALIGN_SPACE_BETWEEN,
            LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_START);
        lv_obj_set_style_pad_row(lbtn_row2_cont, 0, 0);
        lv_obj_set_style_pad_column(lbtn_row2_cont, 0, 0);
        lv_obj_set_style_pad_all(lbtn_row2_cont, 0, 0);
        lv_obj_add_style(lbtn_row2_cont, &style_cont, 0);

        auto lbtn_extra_text = lv_label_create(lbtn_row2_cont);
        lv_obj_set_height(lbtn_extra_text, LV_SIZE_CONTENT);
        lv_obj_set_flex_grow(lbtn_extra_text, 1);
        lv_label_set_text(lbtn_extra_text, g.desc.c_str());
        lv_obj_add_style(lbtn_extra_text, &style_text, 0);

        auto img = get_img(g.img);
        if(img)
        {
            auto limg = lv_img_create(lbtn_row2_cont);
            lv_obj_set_size(limg, img->header.w/h_scale, img->header.h/v_scale);
            lv_img_set_src(limg, img);
            lv_img_set_zoom(limg, 256/h_scale);
        }
    }

    lv_obj_delete(load_text);
    lv_obj_set_flex_flow(list, LV_FLEX_FLOW_COLUMN);
    lv_obj_set_style_pad_left(list, 16/h_scale, 0);
    lv_obj_set_style_pad_right(list, 16/h_scale, 0);
    lv_obj_set_style_pad_top(list, 16/v_scale, 0);
    lv_obj_set_style_pad_bottom(list, 16/v_scale, 0);
    lv_obj_set_style_pad_row(list, 12/v_scale, 0);

    /* Load a default game, if requested */
    if(argc > 1 && argv[1])
    {
        std::string gname(argv[1]);
        for(const auto &g : games)
        {
            if(g.name == gname)
            {
                g.Load();
            }
        }
    }

    while(1)
    {
        if(nrefresh)
        {
            lv_obj_invalidate(lv_scr_act());
            nrefresh--;
        }
        usleep(lv_timer_handler() * 1000);
    }

    return 0;
}

void game_click(lv_event_t *e)
{
    const auto &g = games[(int)e->user_data];
    Mix_HaltMusic();
    g.Load();
    lv_obj_invalidate(lv_scr_act());    // redraw screen after the game is finished and for a few more frames
    nrefresh = 3;
}

void btn_focus(lv_event_t *e)
{
    auto col = (uint32_t)e->user_data;
    GK_SetLED(1, col);
}
