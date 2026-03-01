#include <lvgl.h>
#include <lvgl/src/drivers/lv_drivers.h>
#include <gk.h>
#include <cstdio>
#include <cstring>
#include <ctime>
#include <unistd.h>
#include "supervisor.h"

static uint32_t last_supervisor_update = 0;
static lv_display_t *overlay;
static lv_obj_t *oscr, *osbar, *omain;

static lv_obj_t *sbar_date, *sbar_fps, *sbar_temp, *sbar_v, *sbar_w, *sbar_cpu;

pid_t gkmenu_pid;

int main(int argc, char *argv[])
{
    fprintf(stderr, "gksupervisor 2026-03-01 12:00 startup\n");
    
    lv_init();

    overlay = lv_gk_overlaydisplay_create();
    lv_gk_overlaydisplay_set_alpha(255);
    lv_display_set_default(overlay);

    auto kbd = lv_gk_kbd_create();

    auto grp = lv_group_create();
    lv_indev_set_group(kbd, grp);
    lv_group_set_wrap(grp, false);

    auto touch = lv_gk_mouse_create();


    /* Add supervisor style display */
    oscr = lv_disp_get_scr_act(overlay);
    lv_obj_set_style_bg_opa(oscr, LV_OPA_TRANSP, 0);

    osbar = lv_obj_create(oscr);
    lv_obj_set_style_opa(osbar, LV_OPA_COVER, 0);
    lv_obj_set_style_bg_opa(osbar, 128, 0);
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

    // Spawn gkmenu
    proccreate_t pcinfo;

    memset(&pcinfo, 0, sizeof(proccreate_t));
    pcinfo.cwd = "/gkmenu-0.1.1-gk";
    pcinfo.argc = 0;
    pcinfo.argv = nullptr;
    pcinfo.screen_w = 800;
    pcinfo.screen_h = 480;
    pcinfo.screen_overwritten_each_frame = GK_SCREEN_UPDATE_PARTIAL_NOREADBACK;
    pcinfo.pixel_format = GK_PIXELFORMAT_RGB565;
    pcinfo.screen_refresh = 60;
    pcinfo.with_focus = 1;
    pcinfo.cpu_freq = 800000000;
    pcinfo.osd = nullptr;
    // inherit stdin/out/err, nil else
    pcinfo.acquire_fds[0] = 1;
    pcinfo.acquire_fds[1] = 2;
    pcinfo.acquire_fds[2] = 3;

    pcinfo.keymap.gamepad_to_scancode[GK_KEYA] = GK_SCANCODE_RETURN;
    pcinfo.keymap.gamepad_to_scancode[GK_KEYB] = GK_SCANCODE_ESCAPE;
    pcinfo.keymap.gamepad_to_scancode[GK_KEYJOYDIGILEFT] = 0;
    pcinfo.keymap.gamepad_to_scancode[GK_KEYJOYDIGIRIGHT] = 0;
    pcinfo.keymap.gamepad_to_scancode[GK_KEYJOYDIGIUP] = GK_SCANCODE_AUDIOPREV;
    pcinfo.keymap.gamepad_to_scancode[GK_KEYJOYDIGIDOWN] = GK_SCANCODE_AUDIONEXT;
    pcinfo.keymap.gamepad_to_scancode[GK_KEYLEFT] = 0;
    pcinfo.keymap.gamepad_to_scancode[GK_KEYRIGHT] = 0;
    pcinfo.keymap.gamepad_to_scancode[GK_KEYUP] = GK_SCANCODE_AUDIOPREV;
    pcinfo.keymap.gamepad_to_scancode[GK_KEYDOWN] = GK_SCANCODE_AUDIONEXT;
    pcinfo.keymap.touch_is_mouse = 1;

    GK_CreateProcess("/gkmenu-0.1.1-gk/bin/gkmenu", &pcinfo, &gkmenu_pid);

    while(1)
    {
        supervisor_tick();

        auto delay = lv_timer_handler();

        usleep(delay * 1000);
    }

    return 0;

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
