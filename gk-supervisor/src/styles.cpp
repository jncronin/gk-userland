#include <lvgl/lvgl.h>
#include "styles.h"

lv_style_t style_cont, style_text, style_button, style_transp;
lv_style_t style_slider_main, style_slider_knob, style_slider_indicator;
lv_style_t style_kbd_button, style_kbd;
lv_style_t style_btn_focus, style_kbd_btn_focus;

void init_styles()
{
    /* Semitransparent style used for backgrounds */
    lv_style_init(&style_cont);
    lv_style_set_bg_opa(&style_cont, 160);
    lv_style_set_opa(&style_cont, LV_OPA_COVER);
    lv_style_set_border_width(&style_cont, 0);
    lv_style_set_border_opa(&style_cont, LV_OPA_TRANSP);
    lv_style_set_bg_color(&style_cont, lv_color_make(0xaa, 00, 00));
    lv_style_set_pad_all(&style_cont, 0);
    lv_style_set_radius(&style_cont, 0);
    lv_style_set_margin_all(&style_cont, 0);

    /* Completely transparent style for tabs etc */
    lv_style_init(&style_transp);
    lv_style_set_bg_opa(&style_transp, LV_OPA_TRANSP);
    lv_style_set_opa(&style_transp, LV_OPA_COVER);
    lv_style_set_border_width(&style_transp, 0);
    lv_style_set_border_opa(&style_transp, LV_OPA_TRANSP);
    lv_style_set_radius(&style_transp, 0);
    lv_style_set_margin_all(&style_transp, 0);
    lv_style_set_pad_all(&style_transp, 0);

    lv_style_init(&style_kbd);
    lv_style_set_bg_opa(&style_kbd, 160);
    lv_style_set_opa(&style_kbd, LV_OPA_COVER);
    lv_style_set_border_width(&style_kbd, 0);
    lv_style_set_border_opa(&style_kbd, LV_OPA_TRANSP);
    lv_style_set_bg_color(&style_kbd, lv_color_make(0xaa, 00, 00));
    lv_style_set_pad_all(&style_kbd, 0);
    lv_style_set_pad_row(&style_kbd, 0);
    lv_style_set_pad_column(&style_kbd, 0);
    lv_style_set_radius(&style_kbd, 0);
    lv_style_set_margin_all(&style_kbd, 0);

    /* Button styles */
    lv_style_init(&style_button);
    lv_style_set_bg_opa(&style_button, 160);
    lv_style_set_bg_color(&style_button, lv_color_make(0xaa, 0, 0));
    lv_style_set_opa(&style_button, LV_OPA_COVER);
    lv_style_set_border_width(&style_button, 4);
    lv_style_set_border_opa(&style_button, LV_OPA_COVER);
    lv_style_set_border_color(&style_button, lv_color_white());
    lv_style_set_pad_all(&style_button, 8);
    lv_style_set_radius(&style_button, 2);
    lv_style_set_margin_all(&style_button, 0);
    lv_style_set_text_color(&style_button, lv_color_white());
    lv_style_set_shadow_width(&style_button, 0);
    lv_style_set_text_opa(&style_button, LV_OPA_COVER);

    lv_style_init(&style_kbd_button);
    lv_style_set_bg_opa(&style_kbd_button, 160);
    lv_style_set_bg_color(&style_kbd_button, lv_color_make(0xaa, 0, 0));
    lv_style_set_opa(&style_kbd_button, LV_OPA_COVER);
    lv_style_set_border_width(&style_kbd_button, 2);
    lv_style_set_border_opa(&style_kbd_button, LV_OPA_COVER);
    lv_style_set_border_color(&style_kbd_button, lv_color_white());
    lv_style_set_pad_top(&style_kbd_button, 8);
    lv_style_set_pad_bottom(&style_kbd_button, 8);
    lv_style_set_pad_left(&style_kbd_button, 2);
    lv_style_set_pad_right(&style_kbd_button, 2);
    lv_style_set_radius(&style_kbd_button, 2);
    lv_style_set_margin_all(&style_kbd_button, 0);
    lv_style_set_text_color(&style_kbd_button, lv_color_white());
    lv_style_set_shadow_width(&style_kbd_button, 0);
    lv_style_set_text_opa(&style_kbd_button, LV_OPA_COVER);

    /* Button with focus */
    lv_style_init(&style_btn_focus);
    lv_style_set_outline_width(&style_btn_focus, 4);
    lv_style_set_outline_color(&style_btn_focus, lv_color_make(0, 32, 192));
    lv_style_set_outline_pad(&style_btn_focus, -4);
    lv_style_set_outline_opa(&style_btn_focus, LV_OPA_COVER);

    lv_style_init(&style_kbd_btn_focus);
    lv_style_copy(&style_kbd_btn_focus, &style_btn_focus);
    lv_style_set_bg_color(&style_kbd_btn_focus, lv_color_make(64, 96, 255));

    /* Text style */
    lv_style_init(&style_text);
    lv_style_set_text_color(&style_text, lv_color_white());
    lv_style_set_text_opa(&style_text, LV_OPA_COVER);

    /* Slider styles.  Main is the background, indicator is the bit to the left of the knob,
     knob is the knob */
    lv_style_init(&style_slider_main);
    lv_style_set_radius(&style_slider_main, 2);

    lv_style_init(&style_slider_indicator);
    lv_style_set_radius(&style_slider_indicator, 2);
    lv_style_set_bg_color(&style_slider_indicator, lv_color_make(0xcc, 0, 0));

    lv_style_init(&style_slider_knob);
    lv_style_set_radius(&style_slider_knob, 2);
    lv_style_set_bg_color(&style_slider_knob, lv_color_make(0xff, 0, 0));
    lv_style_set_pad_left(&style_slider_knob, -8);
    lv_style_set_pad_right(&style_slider_knob, -8);
    lv_style_set_pad_top(&style_slider_knob, 8);
    lv_style_set_pad_bottom(&style_slider_knob, 8);
}
