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
#include "osd.h"

static uint32_t last_supervisor_update = 0;
static lv_display_t *overlay;
static lv_obj_t *oscr, *osbar, *omain, *ovol;

static lv_obj_t *sbar_date, *sbar_fps, *sbar_temp, *sbar_v, *sbar_w, *sbar_cpu, *sbar_icons;
static lv_obj_t *main_title;
static lv_obj_t *bright_ctrl, *btn_wifi, *btn_rawsd;
static lv_obj_t *vol_ctrl;

static lv_obj_t *main_tv1;

static lv_timer_t *vol_timer;

pid_t gkmenu_pid;

static int cc_cb();
static void kill_click(lv_event_t *e);
static void bright_change(lv_event_t *e);
static void wifi_change(lv_event_t *e);
static void rawsd_change(lv_event_t *e);
static void main_gesture(lv_event_t *e);
static void close_supervisor();
static void show_supervisor();
static void toggle_supervisor();
static bool supervisor_last_show_cmd = false;
static bool volume_last_show_cmd = false;
static int rk_cb(unsigned short, int);
static void vol_timer_cb(lv_timer_t *t);

int main(int argc, char *argv[])
{
    fprintf(stderr, "gksupervisor 2026-03-01 12:00 startup\n");
    
    lv_init();

    init_styles();

    overlay = lv_gk_overlaydisplay_create();
    lv_gk_overlaydisplay_set_alpha(192);
    lv_display_set_default(overlay);
    lv_gk_register_caption_change_callback(cc_cb);
    lv_gk_register_rawkey_callback(rk_cb);

    auto kbd = lv_gk_kbd_create();

    auto grp = lv_group_create();
    lv_indev_set_group(kbd, grp);
    lv_group_set_wrap(grp, false);

    auto touch = lv_gk_mouse_create();


    /* Add supervisor style display */
    oscr = lv_disp_get_scr_act(overlay);
    lv_obj_set_style_bg_color(oscr, lv_color_make(0x80, 0x80, 0), 0);
    lv_obj_clear_flag(oscr, LV_OBJ_FLAG_SCROLLABLE);

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

    sbar_icons = lv_label_create(osbar);
    lv_obj_set_style_text_font(sbar_icons, &lv_font_montserrat_20, 0);
    lv_obj_set_pos(sbar_icons, 612, 4);
    lv_obj_set_width(sbar_icons, 800-4-612);
    lv_obj_set_style_text_align(sbar_icons, LV_TEXT_ALIGN_RIGHT, 0);
    lv_obj_add_style(sbar_icons, &style_text, 0);

    /* Main part */
    omain = lv_obj_create(oscr);
    lv_obj_set_pos(omain, 0, 240);
    lv_obj_set_size(omain, lv_obj_get_width(oscr), lv_obj_get_height(oscr) - 240);
    lv_obj_add_style(omain, &style_cont, 0);
    lv_obj_add_event_cb(oscr, main_gesture, LV_EVENT_GESTURE, nullptr);

    main_title = lv_label_create(omain);
    lv_obj_set_style_text_font(main_title, &lv_font_montserrat_24, 0);
    lv_obj_set_pos(main_title, 4, 4);
    lv_obj_set_width(main_title, lv_obj_get_width(oscr) - 8);
    lv_obj_set_style_text_align(main_title, LV_TEXT_ALIGN_CENTER, 0);
    lv_label_set_long_mode(main_title, LV_LABEL_LONG_MODE_SCROLL);
    lv_obj_add_style(main_title, &style_text, 0);

    auto main_tv = lv_tabview_create(omain);
    lv_obj_add_style(main_tv, &style_transp, 0);
    lv_obj_set_pos(main_tv, 0, 32);
    lv_obj_set_size(main_tv, 800, 240-32);
    lv_obj_add_flag(lv_tabview_get_tab_btns(main_tv), LV_OBJ_FLAG_HIDDEN);
    main_tv1 = lv_tabview_add_tab(main_tv, "");
    lv_obj_add_style(main_tv1, &style_transp, 0);
    auto main_tv2 = lv_tabview_add_tab(main_tv, "");
    lv_obj_add_style(main_tv2, &style_transp, 0);
    auto main_tv3 = lv_tabview_add_tab(main_tv, "");
    lv_obj_add_style(main_tv3, &style_transp, 0);

    // Custom OSD on page 1
    osd_load_custom(main_tv1, "gkmenu.osd");

    // Options on page 2
    lv_obj_set_layout(main_tv2, LV_LAYOUT_FLEX);
    lv_obj_set_flex_flow(main_tv2, LV_FLEX_FLOW_COLUMN);
    lv_obj_set_style_pad_row(main_tv2, 16, 0);
    lv_obj_set_style_pad_all(main_tv2, 16, 0);

    auto p2_l1 = lv_obj_create(main_tv2);
    lv_obj_add_style(p2_l1, &style_transp, 0);
    lv_obj_set_layout(p2_l1, LV_LAYOUT_FLEX);
    lv_obj_set_flex_flow(p2_l1, LV_FLEX_FLOW_ROW);
    lv_obj_set_width(p2_l1, LV_PCT(100));
    lv_obj_set_flex_align(p2_l1, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_START);
    lv_obj_set_height(p2_l1, 64);

    auto bright_text = gk_label_create(p2_l1, "Brightness", &lv_font_montserrat_20);
    lv_obj_set_width(bright_text, 150);
    bright_ctrl = lv_slider_create(p2_l1);
    lv_obj_set_width(bright_ctrl, 500);
    lv_obj_set_height(bright_ctrl, 32);
    lv_obj_set_style_margin_all(bright_ctrl, 16, 0);
    lv_slider_set_range(bright_ctrl, 0, 100);
    lv_slider_set_value(bright_ctrl, GK_KERNEL_INFO->brightness, LV_ANIM_OFF);
    lv_obj_add_event_cb(bright_ctrl, bright_change, LV_EVENT_VALUE_CHANGED, nullptr);
    lv_obj_add_style(bright_ctrl, &style_slider_main, LV_PART_MAIN);
    lv_obj_add_style(bright_ctrl, &style_slider_indicator, LV_PART_INDICATOR);
    lv_obj_add_style(bright_ctrl, &style_slider_knob, LV_PART_KNOB);

    auto p2_l2 = lv_obj_create(main_tv2);
    lv_obj_add_style(p2_l2, &style_transp, 0);
    lv_obj_set_layout(p2_l2, LV_LAYOUT_FLEX);
    lv_obj_set_flex_flow(p2_l2, LV_FLEX_FLOW_ROW_WRAP);
    lv_obj_set_height(p2_l2, 64);
    lv_obj_set_width(p2_l2, LV_PCT(100));
    btn_wifi = gk_btn_create(p2_l2, "Wifi");
    lv_obj_add_flag(btn_wifi, LV_OBJ_FLAG_CHECKABLE);
    lv_obj_add_event_cb(btn_wifi, wifi_change, LV_EVENT_VALUE_CHANGED, nullptr);
    if(GK_KERNEL_INFO->wifi_state > 0)
        lv_obj_add_state(btn_wifi, LV_STATE_CHECKED);

    btn_rawsd = gk_btn_create(p2_l2, "RawSD");
    lv_obj_add_flag(btn_rawsd, LV_OBJ_FLAG_CHECKABLE);
    lv_obj_add_event_cb(btn_rawsd, rawsd_change, LV_EVENT_VALUE_CHANGED, nullptr);

    // Keyboard on p3
    lv_obj_set_style_pad_all(main_tv3, 16, 0);
    auto kbd_widget = lv_keyboard_create(main_tv3);
    lv_obj_add_style(kbd_widget, &style_transp, LV_PART_MAIN);
    lv_obj_add_style(kbd_widget, &style_button, LV_PART_ITEMS);
    lv_obj_set_size(kbd_widget, LV_PCT(100), LV_PCT(100));

    /* Volume on separate panel */
    ovol = lv_obj_create(oscr);
    lv_obj_add_style(ovol, &style_cont, 0);
    lv_obj_set_style_pad_all(ovol, 8, 0);
    lv_obj_set_size(ovol, 48, 112);
    lv_obj_set_pos(ovol, 704, 80);

    vol_ctrl = lv_bar_create(ovol);
    lv_obj_set_size(vol_ctrl, LV_PCT(100), LV_PCT(100));
    lv_bar_set_orientation(vol_ctrl, LV_BAR_ORIENTATION_VERTICAL);
    lv_obj_add_style(vol_ctrl, &style_slider_main, LV_PART_MAIN);
    lv_obj_add_style(vol_ctrl, &style_slider_indicator, LV_PART_INDICATOR);
    lv_bar_set_range(vol_ctrl, 0, 100);
    lv_bar_set_value(vol_ctrl, GK_KERNEL_INFO->volume, LV_ANIM_OFF);

    /* Timer that can be used to hide the volume control after a period of inactivity */
    vol_timer = lv_timer_create(vol_timer_cb, 2000, nullptr);
    lv_timer_set_repeat_count(vol_timer, -1);

    lv_obj_update_layout(lv_scr_act());

    // hide supervisor at startup
    lv_obj_set_y(omain, 480);
    lv_obj_set_y(osbar, 0 - lv_obj_get_height(osbar));
    lv_obj_set_x(ovol, 800);
    lv_obj_add_flag(omain, LV_OBJ_FLAG_HIDDEN);
    lv_obj_add_flag(osbar, LV_OBJ_FLAG_HIDDEN);
    lv_obj_add_flag(ovol, LV_OBJ_FLAG_HIDDEN);
    lv_obj_update_layout(lv_scr_act());

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
    pcinfo.processdata = "gkmenu.osd";
    pcinfo.nprocessdata = sizeof("gkmenu.osd") - 1; // don't null terminate because we know the length

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

        const char *blank = "";
        const char *bt_sym, *wifi_sym, *charge_sym, *usb_sym, *bat_sym;

        switch(GK_KERNEL_INFO->bt_state)
        {
            case 1:
            case 2:
                bt_sym = LV_SYMBOL_BLUETOOTH;
                break;
            default:
                bt_sym = blank;
                break;
        }
        switch(GK_KERNEL_INFO->wifi_state)
        {
            case 1:
            case 2:
                wifi_sym = LV_SYMBOL_WIFI;
                break;
            default:
                wifi_sym = blank;
                break;
        }
        usb_sym = GK_KERNEL_INFO->usb_state ? LV_SYMBOL_USB : blank;
        charge_sym = GK_KERNEL_INFO->pwr_vbus ? LV_SYMBOL_CHARGE : blank;
        if(GK_KERNEL_INFO->soc >= 75)
            bat_sym = LV_SYMBOL_BATTERY_FULL;
        else if(GK_KERNEL_INFO->soc >= 50)
            bat_sym = LV_SYMBOL_BATTERY_3;
        else if(GK_KERNEL_INFO->soc >= 25)
            bat_sym = LV_SYMBOL_BATTERY_2;
        else if(GK_KERNEL_INFO->soc >= 5)
            bat_sym = LV_SYMBOL_BATTERY_1;
        else
            bat_sym = LV_SYMBOL_BATTERY_EMPTY;
        
        snprintf(buf, 63, "%s%s%s%s%s", bt_sym, wifi_sym, usb_sym, charge_sym, bat_sym);
        lv_label_set_text(sbar_icons, buf);

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

    /* Is there a custom osd? */
    char osdname[512];
    auto nosd = GK_GetProcessData(fpid, osdname, sizeof(osdname) - 1);
    osdname[sizeof(osdname) - 1] = 0;
    if(nosd > 0)
    {
        fprintf(stderr, "supervisor: custom_osd (%d): %s\n", nosd, osdname);
        std::string cosdname(osdname, nosd);
        osd_load_custom(main_tv1, cosdname);
    }
    else
    {
        osd_load_default(main_tv1);
    }

    return 0;
}

void bright_change(lv_event_t *)
{
    auto new_bright = lv_slider_get_value(bright_ctrl);
    GK_GPUSetBrightness(new_bright);
}

void wifi_change(lv_event_t *)
{
    auto checked = (lv_obj_get_state(btn_wifi) & LV_STATE_CHECKED) != 0;
    fprintf(stderr, "wifi: %s\n", checked ? "enabled" : "disabled");
    if(GK_WifiEnable(checked ? 1 : 0) == -1)
    {
        // handle failure - set to whatever is reported
        auto reported_val = GK_KERNEL_INFO->wifi_state != 0;
        if(checked != reported_val)
        {
            if(reported_val)
            {
                lv_obj_add_state(btn_wifi, LV_STATE_CHECKED);
            }
            else
            {
                lv_obj_clear_state(btn_wifi, LV_STATE_CHECKED);
            }
        }
    }
}

void main_gesture(lv_event_t *e)
{
    auto gesture = lv_indev_get_gesture_dir(lv_indev_get_act());
    if(gesture == LV_DIR_BOTTOM)
    {
        close_supervisor();
    }
}

void rawsd_change(lv_event_t *)
{
    auto checked = (lv_obj_get_state(btn_rawsd) & LV_STATE_CHECKED) != 0;
    fprintf(stderr, "rawsd: %s\n", checked ? "enabled" : "disabled");
    if(GK_RawSDEnable(checked ? 1 : 0) == -1)
    {
        fprintf(stderr, "rawsd: setting failed - TODO implement this\n");
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

static void supervisor_anim_cb(void *enc_arriving, int32_t v)
{
    bool arriving = enc_arriving != nullptr;
    static bool last_arriving = false;

    // v here is based upon the y coordinate of omain, i.e. from 240 to 480
    lv_obj_set_y(omain, v);

    // map [240, 480] to [0, -height(osbar)]
    lv_obj_set_y(osbar, -lv_obj_get_height(osbar) * (v - 240) / 240);

    if(arriving && !last_arriving)
    {
        // set visible
        lv_obj_clear_flag(omain, LV_OBJ_FLAG_HIDDEN);
        lv_obj_clear_flag(osbar, LV_OBJ_FLAG_HIDDEN);
        last_arriving = arriving;
    }
    else if(!arriving && arriving)
    {
        last_arriving = arriving;
    }

    if(arriving && v == 240)
    {
        // last frame
        update_kernel_state(true);
    }
    if(!arriving && v == 480)
    {
        // last frame
        lv_obj_add_flag(omain, LV_OBJ_FLAG_HIDDEN);
        lv_obj_add_flag(osbar, LV_OBJ_FLAG_HIDDEN);
        update_kernel_state(false);
    }
}

static void volume_anim_cb(void *enc_arriving, int32_t v)
{
    bool arriving = enc_arriving != nullptr;

    lv_obj_set_x(ovol, v);

    if(!arriving && v == 800)
    {
        // last frame
        lv_obj_add_flag(ovol, LV_OBJ_FLAG_HIDDEN);
        lv_obj_add_flag(ovol, LV_OBJ_FLAG_HIDDEN);
    }
}

// shutdown the main parts of supervisor (status bar and main screen overlay)
void close_supervisor()
{
    fprintf(stderr, "close supervisor\n");
    supervisor_last_show_cmd = false;

    // stop any running show animation
    lv_anim_del((void*)(intptr_t)1, supervisor_anim_cb);

    lv_anim_t anim;
    lv_anim_init(&anim);

    lv_anim_set_values(&anim, lv_obj_get_y(omain), 480);
    lv_anim_set_time(&anim, 250);
    lv_anim_set_exec_cb(&anim, supervisor_anim_cb);
    lv_anim_set_path_cb(&anim, lv_anim_path_ease_in);
    lv_anim_set_var(&anim, (void*)0);       // leaving
    lv_anim_start(&anim);
}

void show_supervisor()
{
    fprintf(stderr, "show supervisor\n");
    supervisor_last_show_cmd = true;

    // stop any running hide animation
    lv_anim_del((void*)0, supervisor_anim_cb);
    
    lv_obj_clear_flag(omain, LV_OBJ_FLAG_HIDDEN);
    lv_obj_clear_flag(osbar, LV_OBJ_FLAG_HIDDEN);

    lv_anim_t anim;
    lv_anim_init(&anim);

    lv_anim_set_values(&anim, lv_obj_get_y(omain), 240);
    lv_anim_set_time(&anim, 250);
    lv_anim_set_exec_cb(&anim, supervisor_anim_cb);
    lv_anim_set_path_cb(&anim, lv_anim_path_ease_in);
    lv_anim_set_var(&anim, (void*)(intptr_t)1);       // arriving
    lv_anim_start(&anim);
}

void toggle_supervisor()
{
    if(supervisor_last_show_cmd)
        close_supervisor();
    else
        show_supervisor();
}

static void show_volume()
{
    fprintf(stderr, "show volume\n");
    volume_last_show_cmd = true;

    // stop any running hide animation
    lv_anim_del((void*)0, volume_anim_cb);
    
    lv_obj_clear_flag(ovol, LV_OBJ_FLAG_HIDDEN);

    lv_anim_t anim;
    lv_anim_init(&anim);

    lv_anim_set_values(&anim, lv_obj_get_x(ovol), 704);
    lv_anim_set_time(&anim, 250);
    lv_anim_set_exec_cb(&anim, volume_anim_cb);
    lv_anim_set_path_cb(&anim, lv_anim_path_ease_in);
    lv_anim_set_var(&anim, (void*)(intptr_t)1);       // arriving
    lv_anim_start(&anim);
}

void close_volume()
{
    fprintf(stderr, "close volume\n");
    volume_last_show_cmd = false;

    // stop any running show animation
    lv_anim_del((void*)(intptr_t)1, volume_anim_cb);

    lv_anim_t anim;
    lv_anim_init(&anim);

    lv_anim_set_values(&anim, lv_obj_get_x(ovol), 800);
    lv_anim_set_time(&anim, 250);
    lv_anim_set_exec_cb(&anim, volume_anim_cb);
    lv_anim_set_path_cb(&anim, lv_anim_path_ease_in);
    lv_anim_set_var(&anim, (void*)0);       // leaving
    lv_anim_start(&anim);
}

static int gkkey_to_special(unsigned short key)
{
    switch(key)
    {
        case GK_SCANCODE_POWER:
            return 0;
        case GK_SCANCODE_MENU:
            return 1;
        case GK_SCANCODE_VOLUMEUP:
            return 2;
        case GK_SCANCODE_VOLUMEDOWN:
            return 3;
        default:
            return -1;
    }
}

static void handle_volchange(int amount)
{
    // reset visibility timer
    lv_timer_reset(vol_timer);

    // animate in
    if(volume_last_show_cmd == false)
    {
        show_volume();
    }

    // update kernel
    auto newvol = GK_AudioSetVolume(lv_bar_get_value(vol_ctrl) + amount);

    // Update the bar
    lv_bar_set_value(vol_ctrl, newvol, LV_ANIM_ON);
}

static int handle_newpress(unsigned short key)
{
    fprintf(stderr, "newpress: %u\n", key);

    switch(key)
    {
        case GK_SCANCODE_MENU:
            toggle_supervisor();
            break;

        case GK_SCANCODE_VOLUMEUP:
            handle_volchange(10);
            break;

        case GK_SCANCODE_VOLUMEDOWN:
            handle_volchange(-10);
            break;
    }

    return 1;
}

static int handle_ongoingpress(unsigned short key)
{
    fprintf(stderr, "ongoingpress: %u\n", key);

    switch(key)
    {
        case GK_SCANCODE_VOLUMEUP:
            handle_volchange(10);
            break;

        case GK_SCANCODE_VOLUMEDOWN:
            handle_volchange(-10);
            break;
    }

    return 1;
}

static int handle_release(unsigned short key)
{
    fprintf(stderr, "release: %u\n", key);
    return 1;
}

static int handle_click(unsigned short key)
{
    fprintf(stderr, "click: %u\n", key);
    return 1;
}

int rk_cb(unsigned short key, int pressed)
{
    // we intercept menu, power, volup and voldown as they mean special things to us
    static int special_pressed[4] = { 0 };

    auto special_key = gkkey_to_special(key);
    if(special_key == -1)
        return 0;   // handle via default lvgl indev
    
    bool is_newpress = pressed && special_pressed[special_key] == 0;
    bool is_ongoingpress = pressed && special_pressed[special_key] == 1;
    bool is_release = (pressed == 0);
    bool is_click = (pressed == 0) && special_pressed[special_key] == 1;

    special_pressed[special_key] = pressed ? 1 : 0;

    if(is_newpress)
        return handle_newpress(key);
    else if(is_ongoingpress)
        return handle_ongoingpress(key);
    else if(is_release && is_click)
        return handle_release(key) | handle_click(key);
    else if(is_release)
        return handle_release(key);
    else
        return 0;
}

void vol_timer_cb(lv_timer_t *)
{
    if(volume_last_show_cmd == true)
        close_volume();
}
