#ifndef STYLES_H
#define STYLES_H

#include <lvgl/lvgl.h>

void init_styles();

extern lv_style_t style_cont, style_button, style_text, style_transp;
extern lv_style_t style_slider_main, style_slider_knob, style_slider_indicator;
extern lv_style_t style_kbd, style_kbd_button;
extern lv_style_t style_btn_focus, style_kbd_btn_focus;

#endif
