#include <squirrel.h>
#include <lvgl.h>
#include <lvgl/src/drivers/lv_drivers.h>
#include "game.h"
#include <algorithm>
#include <png.h>
#include <img.h>

std::vector<Game> games;

static lv_obj_t *list;

int load_games();
static void game_click(lv_event_t *e);
int nrefresh = 0;

int main()
{
    lv_init();

    auto display = lv_gk_display_create();
    lv_display_set_default(display);
    auto kbd = lv_gk_kbd_create();

    auto grp = lv_group_create();
    lv_indev_set_group(kbd, grp);
    lv_group_set_wrap(grp, false);

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

    /* background style */
    lv_style_t style_bg;
    lv_style_init(&style_bg);
    lv_style_set_bg_color(&style_bg, lv_color_black());
    lv_style_set_border_width(&style_bg, 0);

    /* text styles */
    lv_style_t style_text;
    lv_style_init(&style_text);
    lv_style_set_opa(&style_text, LV_OPA_100);
    lv_style_set_text_color(&style_text, lv_color_white());

    /* populate games list */
    list = lv_obj_create(lv_screen_active());
    lv_obj_set_size(list, LV_PCT(100), LV_PCT(100));
    lv_obj_set_flex_flow(list, LV_FLEX_FLOW_COLUMN);
    lv_obj_add_style(list, &style_bg, 0);
    lv_obj_set_style_radius(list, 0, 0);

    for(int i = 0; i < games.size(); i++)
    {
        const auto &g = games[i];

        auto lbtn = lv_button_create(list);
        lv_obj_set_flex_flow(lbtn, LV_FLEX_FLOW_COLUMN);
        lv_obj_add_event_cb(lbtn, game_click, LV_EVENT_CLICKED, (void *)i);
        lv_obj_set_size(lbtn, LV_PCT(100), LV_SIZE_CONTENT);
        if(i & 1)
        {
            lv_obj_set_style_bg_color(lbtn, lv_color_make(160, 0, 0), 0);
            lv_obj_set_style_shadow_color(lbtn, lv_color_make(120, 0, 0), 0);
            lv_obj_set_style_outline_color(lbtn, lv_color_make(255, 40, 40), LV_STATE_FOCUSED);
            lv_obj_set_style_outline_color(lbtn, lv_color_make(255, 40, 40), LV_STATE_FOCUS_KEY);
        }
        else
        {
            // 160, 0, 0 -> Hue - 15deg -> 160, 0, 40
            // 120, 0, 0 -> Hue - 15deg -> 120, 0, 30
            // 255, 40, 40 -> Hue - 15deg -> 255, 40, 94
            lv_obj_set_style_bg_color(lbtn, lv_color_make(160, 0, 40), 0);
            lv_obj_set_style_shadow_color(lbtn, lv_color_make(120, 0, 30), 0);
            lv_obj_set_style_outline_color(lbtn, lv_color_make(255, 40, 94), LV_STATE_FOCUSED);
            lv_obj_set_style_outline_color(lbtn, lv_color_make(255, 40, 94), LV_STATE_FOCUS_KEY);
        }
        lv_obj_set_style_outline_opa(lbtn, LV_OPA_100, LV_STATE_FOCUSED);
        lv_obj_set_style_outline_opa(lbtn, LV_OPA_100, LV_STATE_FOCUS_KEY);

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


        auto limg = lv_img_create(lbtn_row2_cont);
        lv_obj_set_size(limg, 160, 120);
        lv_img_set_src(limg, get_img(g.img));
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
    g.Load();
    lv_obj_invalidate(lv_scr_act());    // redraw screen after the game is finished and for a few more frames
    nrefresh = 3;
}
