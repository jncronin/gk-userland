#include <lvgl.h>
#include <lvgl/src/drivers/lv_drivers.h>
#include "supervisor.h"
#include <unistd.h>
#include <cstdio>
#include <gk.h>
#include <ctime>

static uint32_t last_supervisor_update = 0;
static lv_display_t *overlay;
static lv_obj_t *oscr, *olab;

void init_supervisor()
{
    overlay = lv_gk_overlaydisplay_create();
    lv_gk_overlaydisplay_set_alpha(128);

    /* Add supervisor style display */
    oscr = lv_disp_get_scr_act(overlay);
    olab = lv_label_create(oscr);
    lv_obj_set_style_text_font(olab, &lv_font_montserrat_20, 0);
    lv_obj_set_height(olab, LV_SIZE_CONTENT);
    lv_label_set_text(olab, "");
}

void supervisor_tick()
{
    if(last_supervisor_update == 0 || lv_tick_get() >= (last_supervisor_update + 1000))
    {
        timespec tp;
        clock_gettime(CLOCK_REALTIME, &tp);
        auto t = localtime(&tp.tv_sec);
        char buf[64];
        strftime(buf, 63, "%F %T", t);
        buf[63] = 0;
        char buf_line[128];
        snprintf(buf_line, 127, "%s %.1f FPS %.1fC %.2fV %.2fW CPU %.2f", buf, 
            GK_KERNEL_INFO->fps, GK_KERNEL_INFO->temp,
            GK_KERNEL_INFO->vsys, GK_KERNEL_INFO->psys, GK_KERNEL_INFO->cpu_usage);
        buf_line[127] = 0;

        lv_label_set_text(olab, buf_line);

        last_supervisor_update = lv_tick_get();
    }
}
