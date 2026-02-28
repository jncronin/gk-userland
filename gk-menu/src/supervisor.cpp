#include <lvgl.h>
#include <lvgl/src/drivers/lv_drivers.h>
#include "supervisor.h"
#include <unistd.h>
#include <cstdio>
#include <gk.h>
#include <ctime>

static uint32_t last_supervisor_update = 0;
static lv_display_t *overlay;
static lv_obj_t *oscr, *osbar, *omain;

static lv_obj_t *sbar_date, *sbar_fps, *sbar_temp, *sbar_v, *sbar_w, *sbar_cpu;

void init_supervisor()
{
    overlay = lv_gk_overlaydisplay_create();
    lv_gk_overlaydisplay_set_alpha(255);

    /* Add supervisor style display */
    oscr = lv_disp_get_scr_act(overlay);
    lv_obj_set_style_opa(oscr, LV_OPA_TRANSP, 0);

    osbar = lv_obj_create(oscr);
    lv_obj_set_style_opa(oscr, LV_OPA_COVER, 0);
    lv_obj_set_style_bg_opa(oscr, 192, 0);
    lv_obj_set_pos(osbar, 0, 0);
    lv_obj_set_size(osbar, lv_obj_get_width(oscr), 32);
    lv_obj_set_style_bg_color(osbar, lv_color_make(0xaa, 00, 00), 0);
    lv_obj_set_style_pad_all(osbar, 0, 0);
    lv_obj_set_style_radius(osbar, 0, 0);
    lv_obj_set_style_margin_all(osbar, 0, 0);
    lv_obj_set_style_border_width(osbar, 0, 0);

    sbar_date = lv_label_create(osbar);
    lv_obj_set_style_text_font(sbar_date, &lv_font_montserrat_20, 0);
    lv_obj_set_pos(sbar_date, 4, 4);
    lv_obj_set_style_text_color(sbar_date, lv_color_white(), 0);
    lv_obj_set_style_text_opa(sbar_date, LV_OPA_COVER, 0);

    sbar_fps = lv_label_create(osbar);
    lv_obj_set_style_text_font(sbar_fps, &lv_font_montserrat_20, 0);
    lv_obj_set_pos(sbar_fps, 212, 4);
    lv_obj_set_style_text_color(sbar_fps, lv_color_white(), 0);
    lv_obj_set_style_text_opa(sbar_fps, LV_OPA_COVER, 0);

    sbar_temp = lv_label_create(osbar);
    lv_obj_set_style_text_font(sbar_temp, &lv_font_montserrat_20, 0);
    lv_obj_set_pos(sbar_temp, 296, 4);
    lv_obj_set_style_text_color(sbar_temp, lv_color_white(), 0);
    lv_obj_set_style_text_opa(sbar_temp, LV_OPA_COVER, 0);

    sbar_v = lv_label_create(osbar);
    lv_obj_set_style_text_font(sbar_v, &lv_font_montserrat_20, 0);
    lv_obj_set_pos(sbar_v, 364, 4);
    lv_obj_set_style_text_color(sbar_v, lv_color_white(), 0);
    lv_obj_set_style_text_opa(sbar_v, LV_OPA_COVER, 0);

    sbar_w = lv_label_create(osbar);
    lv_obj_set_style_text_font(sbar_w, &lv_font_montserrat_20, 0);
    lv_obj_set_pos(sbar_w, 432, 4);
    lv_obj_set_style_text_color(sbar_w, lv_color_white(), 0);
    lv_obj_set_style_text_opa(sbar_w, LV_OPA_COVER, 0);

    sbar_cpu = lv_label_create(osbar);
    lv_obj_set_style_text_font(sbar_cpu, &lv_font_montserrat_20, 0);
    lv_obj_set_pos(sbar_cpu, 500, 4);
    lv_obj_set_style_text_color(sbar_cpu, lv_color_white(), 0);
    lv_obj_set_style_text_opa(sbar_cpu, LV_OPA_COVER, 0);


    supervisor_tick();
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
        lv_label_set_text(sbar_date, buf);

        snprintf(buf, 63, "%.1f FPS", GK_KERNEL_INFO->fps);
        lv_label_set_text(sbar_fps, buf);

        snprintf(buf, 63, "%.1fC", GK_KERNEL_INFO->temp);
        lv_label_set_text(sbar_temp, buf);

        snprintf(buf, 63, "%.2fV", GK_KERNEL_INFO->vsys);
        lv_label_set_text(sbar_v, buf);

        snprintf(buf, 63, "%.2fW", GK_KERNEL_INFO->psys);
        lv_label_set_text(sbar_w, buf);

        snprintf(buf, 63, "CPU %.2f", GK_KERNEL_INFO->cpu_usage);
        lv_label_set_text(sbar_cpu, buf);

        last_supervisor_update = lv_tick_get();
    }
}
