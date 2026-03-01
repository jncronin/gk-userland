#include "widget.h"
#include "styles.h"

lv_obj_t *gk_btn_create(lv_obj_t *parent, const std::string &lab, const lv_font_t *font)
{
    auto ret = lv_btn_create(parent);
    lv_obj_add_style(ret, &style_button, 0);

    if(lab != "")
    {
        auto ret_text = lv_label_create(ret);
        lv_obj_set_style_text_font(ret_text, font, 0);
        lv_obj_set_style_text_align(ret_text, LV_TEXT_ALIGN_CENTER, 0);
        lv_obj_add_flag(ret_text, LV_OBJ_FLAG_EVENT_BUBBLE);
        lv_obj_set_align(ret_text, LV_ALIGN_CENTER);
        lv_obj_add_style(ret_text, &style_text, 0);
        lv_label_set_text(ret_text, lab.c_str());
    }

    return ret;
}

lv_obj_t *gk_label_create(lv_obj_t *parent, const std::string &lab, const lv_font_t *font)
{
    auto ret = lv_label_create(parent);
    lv_obj_add_style(ret, &style_text, 0);
    lv_obj_set_style_text_font(ret, font, 0);
    lv_label_set_text(ret, lab.c_str());
    return ret;
}
