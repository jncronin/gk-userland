#ifndef WIDGET_H
#define WIDGET_H

#include <lvgl/lvgl.h>
#include <string>

lv_obj_t *gk_btn_create(lv_obj_t *parent, const std::string &lab = "", const lv_font_t *font =
    &lv_font_montserrat_24);
lv_obj_t *gk_label_create(lv_obj_t *parent, const std::string &lab);

#endif
