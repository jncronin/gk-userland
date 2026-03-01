#include <lvgl.h>
#include <lvgl/src/drivers/lv_drivers.h>
#include <gk.h>
#include <cstdio>
#include <cstring>
#include <ctime>
#include <unistd.h>
#include <vector>
#include "supervisor.h"
#include "styles.h"
#include "widget.h"

static uint32_t last_supervisor_update = 0;
static lv_display_t *overlay;
static lv_obj_t *oscr, *osbar, *omain;

static lv_obj_t *sbar_date, *sbar_fps, *sbar_temp, *sbar_v, *sbar_w, *sbar_cpu;
static lv_obj_t *main_title;

static lv_obj_t *def_overlay_kill;

pid_t gkmenu_pid;

static int cc_cb();
static void kill_click(lv_event_t *e);

int main(int argc, char *argv[])
{
    fprintf(stderr, "gksupervisor 2026-03-01 12:00 startup\n");
    
    lv_init();

    init_styles();

    overlay = lv_gk_overlaydisplay_create();
    lv_gk_overlaydisplay_set_alpha(255);
    lv_display_set_default(overlay);
    lv_gk_register_caption_change_callback(cc_cb);

    auto kbd = lv_gk_kbd_create();

    auto grp = lv_group_create();
    lv_indev_set_group(kbd, grp);
    lv_group_set_wrap(grp, false);

    auto touch = lv_gk_mouse_create();


    /* Add supervisor style display */
    oscr = lv_disp_get_scr_act(overlay);
    lv_obj_set_style_bg_opa(oscr, LV_OPA_TRANSP, 0);

    osbar = lv_obj_create(oscr);
    lv_obj_set_pos(osbar, 0, 0);
    lv_obj_set_size(osbar, lv_obj_get_width(oscr), 32);
    lv_obj_add_style(osbar, &style_cont, 0);

    sbar_date = lv_label_create(osbar);
    lv_obj_set_style_text_font(sbar_date, &lv_font_montserrat_20, 0);
    lv_obj_set_pos(sbar_date, 4, 4);
    lv_obj_add_style(sbar_date, &style_text, 0);

    sbar_fps = lv_label_create(osbar);
    lv_obj_set_style_text_font(sbar_fps, &lv_font_montserrat_20, 0);
    lv_obj_set_pos(sbar_fps, 212, 4);
    lv_obj_add_style(sbar_fps, &style_text, 0);

    sbar_temp = lv_label_create(osbar);
    lv_obj_set_style_text_font(sbar_temp, &lv_font_montserrat_20, 0);
    lv_obj_set_pos(sbar_temp, 308, 4);
    lv_obj_add_style(sbar_temp, &style_text, 0);

    sbar_v = lv_label_create(osbar);
    lv_obj_set_style_text_font(sbar_v, &lv_font_montserrat_20, 0);
    lv_obj_set_pos(sbar_v, 376, 4);
    lv_obj_add_style(sbar_v, &style_text, 0);

    sbar_w = lv_label_create(osbar);
    lv_obj_set_style_text_font(sbar_w, &lv_font_montserrat_20, 0);
    lv_obj_set_pos(sbar_w, 444, 4);
    lv_obj_add_style(sbar_w, &style_text, 0);

    sbar_cpu = lv_label_create(osbar);
    lv_obj_set_style_text_font(sbar_cpu, &lv_font_montserrat_20, 0);
    lv_obj_set_pos(sbar_cpu, 512, 4);
    lv_obj_add_style(sbar_cpu, &style_text, 0);

    /* Main part */
    omain = lv_obj_create(oscr);
    lv_obj_set_pos(omain, 0, 240);
    lv_obj_set_size(omain, lv_obj_get_width(oscr), lv_obj_get_height(oscr) - 240);
    lv_obj_add_style(omain, &style_cont, 0);

    main_title = lv_label_create(omain);
    lv_obj_set_style_text_font(main_title, &lv_font_montserrat_24, 0);
    lv_obj_set_pos(main_title, 4, 4);
    lv_obj_set_width(main_title, lv_obj_get_width(oscr) - 8);
    lv_obj_set_style_text_align(main_title, LV_TEXT_ALIGN_CENTER, 0);
    lv_label_set_long_mode(main_title, LV_LABEL_LONG_MODE_SCROLL);
    lv_obj_add_style(main_title, &style_text, 0);

    def_overlay_kill = gk_btn_create(omain, "Quit");
    lv_obj_set_pos(def_overlay_kill, lv_obj_get_width(oscr) / 2 - 60, 240/2 - 40);
    lv_obj_set_size(def_overlay_kill, 120, 80);
    lv_obj_add_event_cb(def_overlay_kill, kill_click, LV_EVENT_CLICKED, nullptr);

    lv_obj_update_layout(lv_scr_act());

    // show supervisor at startup for testing
    update_kernel_state(true);

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
        update_kernel_state(true);

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

int cc_cb()
{
    auto fpid = GK_GetFocusProcess();
    if(fpid < 0)
        return -1;

    size_t w, h;
    unsigned int pf;
    int refresh;

    if(GK_GPUGetScreenModeForProcess(fpid, &w, &h, &pf, &refresh) != 0)
        return -1;
    
    char pname[256];
    if(GK_GetProcessName(fpid, pname, sizeof(pname) - 1) != 0)
        return -1;

    pname[sizeof(pname) - 1] = 0;
    
    char titlebuf[512];
    snprintf(titlebuf, sizeof(titlebuf) - 1, "%s (%llux%llux%u@%d)", pname,
        w, h, pf, refresh);
    titlebuf[sizeof(titlebuf) - 1] = 0;

    lv_label_set_text(main_title, titlebuf);
    lv_obj_invalidate(main_title);

    return 0;
}

void kill_click(lv_event_t *)
{
    auto fpid = GK_GetFocusProcess();
    char fproc_name[256];
    if(GK_GetProcessName(fpid, fproc_name, sizeof(fproc_name) - 1) == 0)
    {
        fproc_name[sizeof(fproc_name) - 1] = 0;
        if(strcmp("gkmenu", fproc_name))
        {
            // not gkmenu - proceed to kill
            kill(fpid, SIGKILL);
        }
    }
}

template<typename T> static T clamp(T v, T minval, T maxval)
{
    if(v < minval) return minval;
    if(v > maxval) return maxval;
    return v;
}

static void update_ks_obj(std::vector<gk_supervisor_visible_region> &visreg, lv_obj_t *obj)
{
    if(lv_obj_has_flag(obj, LV_OBJ_FLAG_HIDDEN))
        return;
    lv_area_t coords;
    lv_obj_get_coords(obj, &coords);
    coords.x1 = clamp(coords.x1, 0, 800);
    coords.x2 = clamp(coords.x2, 0, 800);
    coords.y1 = clamp(coords.y1, 0, 480);
    coords.y2 = clamp(coords.y2, 0, 480);

    gk_supervisor_visible_region vr;
    vr.x = coords.x1;
    vr.y = coords.y1;
    vr.w = coords.x2 - coords.x1;
    vr.h = coords.y2 - coords.y1;

    visreg.push_back(vr);    
}

void update_kernel_state(bool show)
{
    if(!show)
    {
        GK_SetSupervisorVisibleEx(0, nullptr, 0);
    }
    else
    {
        std::vector<gk_supervisor_visible_region> visreg;

        update_ks_obj(visreg, omain);
        update_ks_obj(visreg, osbar);

        GK_SetSupervisorVisibleEx(1, visreg.data(), visreg.size());
    }
}
