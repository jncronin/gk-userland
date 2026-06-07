#ifndef IMG_H
#define IMG_H

#include <string>
#include <map>
#include "lvgl.h"

class gk_img_context
{
    public:
        std::map<std::string, lv_image_dsc_t *> img_map;
        ~gk_img_context();
};

extern gk_img_context def_img_context;

const lv_image_dsc_t *get_img(const std::string &img, gk_img_context &ctx = def_img_context);


#endif
