#ifndef CUSTOMOSD_SQ_H
#define CUSTOMOSD_SQ_H

#include <squirrel.h>
#include <lvgl.h>
#include <memory>
#include "osd.h"

std::unique_ptr<osd> customosd_sq_create(const std::string &fname, lv_obj_t *hidden_tv);
int add_lvgl_classes(HSQUIRRELVM v, lv_obj_t *tv_parent);


#endif
