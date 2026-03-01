#include <lvgl/lvgl.h>
#include "styles.h"

lv_style_t style_cont, style_text, style_button;

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
    lv_style_set_border_width(&style_cont, 0);

    /* Button styles */
    lv_style_init(&style_button);
    lv_style_set_bg_opa(&style_button, 160);
    lv_style_set_bg_color(&style_button, lv_color_make(0xaa, 0, 0));
    lv_style_set_opa(&style_button, LV_OPA_COVER);
    lv_style_set_border_width(&style_button, 4);
    lv_style_set_border_opa(&style_button, LV_OPA_COVER);
    lv_style_set_border_color(&style_button, lv_color_white());
    lv_style_set_pad_all(&style_button, 4);
    lv_style_set_radius(&style_button, 2);
    lv_style_set_margin_all(&style_button, 0);
    lv_style_set_text_color(&style_button, lv_color_white());
    lv_style_set_shadow_width(&style_button, 0);
    lv_style_set_text_opa(&style_button, LV_OPA_COVER);

    /* Text style */
    lv_style_init(&style_text);
    lv_style_set_text_color(&style_text, lv_color_white());
    lv_style_set_text_opa(&style_text, LV_OPA_COVER);

}
