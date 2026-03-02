#ifndef OSD_H
#define OSD_H

#include <lvgl/lvgl.h>
#include <string>

int osd_load_default(lv_obj_t *parent);
int osd_load_custom(lv_obj_t *parent, const std::string &fname);

#endif
