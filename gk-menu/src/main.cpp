#include <squirrel.h>
#include <lvgl.h>
#include <lvgl/src/drivers/lv_drivers.h>
#include "game.h"

std::vector<Game> games;

int load_games();
static void game_click(lv_event_t *e);

int main()
{
    lv_init();

    auto display = lv_gk_display_create();
    lv_display_set_default(display);
    auto kbd = lv_gk_kbd_create();

    auto grp = lv_group_create();
    lv_indev_set_group(kbd, grp);

    load_games();

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
