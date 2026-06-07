#include <lvgl.h>
#include <array>
#include <cmath>
#include "toasts.h"
#include "styles.h"

/* osbar height is 32, omain starts at 240
    using 20 pixel font we can allow a toast height of 24 to allow a small separation
*/

using namespace std::chrono_literals;

static const int toasts_start_y = 40;
static const int toasts_end_y = 232;
static const int toasts_height = toasts_end_y - toasts_start_y;
static const int toast_height = 24;
static const size_t ntoasts = (size_t)(toasts_height / toast_height);
static const auto fadeout_time = 200ms;
static const auto fadeout_tick = 16ms;
static const int fadeout_opa_bg_max = 160;
static const int fadeout_opa_text_max = 255;
static bool toasts_visible = false;

static_assert(ntoasts > 0);

using toast_clock = std::chrono::steady_clock;
using toast_tp = toast_clock::time_point;

struct toast_info_t
{
    lv_obj_t *o;
    std::string msg;
    toast_tp until;
    toast_tp now;

    bool is_visible() const
    {
        return until > now;
    }
};

static std::array<toast_info_t, ntoasts> toasts;
static lv_timer_t *toast_timer;

static void toast_slideup(size_t id, toast_tp now = toast_clock::now());
static void toast_anim(lv_timer_t *);
static size_t toast_firstfree();
static void toast_remove_gaps();
static void toast_update();

int init_toasts()
{
    auto oscr = lv_scr_act();

    toast_timer = lv_timer_create_basic();
    lv_timer_set_cb(toast_timer, toast_anim);
    lv_timer_set_repeat_count(toast_timer, -1);
    lv_timer_set_period(toast_timer, 60000);
    lv_timer_pause(toast_timer);

    for(auto i = 0U; i < ntoasts; i++)
    {
        toasts[i].o = lv_label_create(oscr);
        lv_obj_set_style_text_font(toasts[i].o, &lv_font_montserrat_20, 0);
        lv_obj_align(toasts[i].o, LV_ALIGN_TOP_MID, 0, toasts_start_y + i * toast_height);
        lv_obj_set_height(toasts[i].o, LV_SIZE_CONTENT);
        lv_obj_set_width(toasts[i].o, LV_SIZE_CONTENT);
        lv_obj_set_style_text_align(toasts[i].o, LV_TEXT_ALIGN_CENTER, 0);
        lv_label_set_long_mode(toasts[i].o, LV_LABEL_LONG_MODE_SCROLL);
        lv_obj_add_style(toasts[i].o, &style_cont, 0);
        lv_obj_add_style(toasts[i].o, &style_text, 0);
        lv_label_set_text(toasts[i].o, "");
        lv_obj_add_flag(toasts[i].o, LV_OBJ_FLAG_HIDDEN);

        toasts[i].until = toast_clock::now();
    }

    return 0;
}

int toast_message(const std::string &msg, const std::chrono::milliseconds msg_time)
{
    toast_remove_gaps();

    // get last
    auto ffree = toast_firstfree();
    if(ffree == ntoasts)
    {
        // slide all up
        for(auto i = 0u; i < ntoasts; i++)
        {
            toast_slideup(i);
        }
        ffree--;
    }

    auto &ctoast = toasts[ffree];

    ctoast.msg = msg;
    lv_label_set_text_static(ctoast.o, ctoast.msg.c_str());
    ctoast.until = toast_clock::now() + msg_time;

    toast_update();

    return 0;
}

size_t toast_firstfree()
{
    for(auto i = 0u; i < ntoasts; i++)
    {
        if(!toasts[i].is_visible())
        {
            return i;
        }
    }
    return ntoasts;
}

void toast_slideup(size_t id, toast_tp now)
{
    if(id == 0)
    {
        // slide off i.e. do nothing
        return;
    }

    auto to_id = id - 1;
    toasts[to_id].msg = toasts[id].msg;
    toasts[to_id].until = toasts[id].until;
    lv_label_set_text_static(toasts[to_id].o, toasts[to_id].msg.c_str());

    toasts[id].until = now;
}

void toast_anim(lv_timer_t *)
{
    toast_remove_gaps();
    toast_update();
}

void toast_remove_gaps()
{
    /* First, ensure we have a common 'now' for each object */
    auto now = toast_clock::now();
    for(auto i = 0u; i < ntoasts; i++)
    {
        toasts[i].now = now;
    }

    /* Then, remove any gaps in the toast display */
    for(auto i = 0u; i < ntoasts - 1; i++)
    {
        // if no visible toasts after us then stop
        bool has_visible_after = false;
        for(auto j = i + 1; j < ntoasts; j++)
        {
            if(toasts[j].is_visible())
            {
                has_visible_after = true;
                break;
            }
        }
        if(!has_visible_after)
            break;

        // keep scrolling everything up until we are filled
        while(toasts[i].is_visible() == false)
        {
            for(auto j = i + 1; j < ntoasts; j++)
            {
                toast_slideup(j, now);
            }
        }
    }
}

void toast_update()
{
    /* Now iterate through determining the next timer tick and setting the lvgl properties
        for each toast */
    toast_clock::duration next_tick;
    bool has_next_tick = false;
    bool any_toast_visible = false;

    for(auto i = 0u; i < ntoasts; i++)
    {
        if(toasts[i].is_visible() == false)
        {
            lv_obj_add_flag(toasts[i].o, LV_OBJ_FLAG_HIDDEN);
            continue;
        }

        any_toast_visible = true;
        
        lv_obj_remove_flag(toasts[i].o, LV_OBJ_FLAG_HIDDEN);

        auto time_left = toasts[i].until - toasts[i].now;
        if(time_left < fadeout_time)
        {
            // do fade
            auto fade_frac = std::chrono::duration<double>(time_left) / 
                std::chrono::duration<double>(fadeout_time);
            auto fade_bg_alpha = (int)std::round((double)fadeout_opa_bg_max * fade_frac);
            if(fade_bg_alpha < 0) fade_bg_alpha = 0;
            if(fade_bg_alpha > fadeout_opa_bg_max) fade_bg_alpha = fadeout_opa_bg_max;
            auto fade_text_alpha = (int)std::round((double)fadeout_opa_text_max * fade_frac);
            if(fade_text_alpha < 0) fade_text_alpha = 0;
            if(fade_text_alpha > fadeout_opa_text_max) fade_text_alpha = fadeout_opa_text_max;
            lv_obj_set_style_bg_opa(toasts[i].o, fade_bg_alpha, 0);
            lv_obj_set_style_text_opa(toasts[i].o, fade_text_alpha, 0);
            time_left = std::min(time_left, std::chrono::duration_cast<toast_clock::duration>(fadeout_tick));
        }
        else
        {
            time_left -= fadeout_time;
            lv_obj_set_style_bg_opa(toasts[i].o, fadeout_opa_bg_max, 0);
            lv_obj_set_style_text_opa(toasts[i].o, fadeout_opa_text_max, 0);
        }

        if(!has_next_tick || time_left < next_tick)
        {
            next_tick = time_left;
            has_next_tick = true;
        }
    }

    // now ask lvgl to call us again when the next toast needs doing
    if(has_next_tick)
    {
        auto tdelay = (uint32_t)std::chrono::duration_cast<std::chrono::milliseconds>(next_tick).count();
        
        lv_timer_set_period(toast_timer, tdelay);
        lv_timer_reset(toast_timer);
        lv_timer_resume(toast_timer);
    }
    else
    {
        lv_timer_pause(toast_timer);
    }

    toasts_visible = any_toast_visible;
}

bool toast_visible()
{
    return toasts_visible;
}
