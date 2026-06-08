#ifndef OSD_H
#define OSD_H

#include <lvgl/lvgl.h>
#include <string>
#include <vector>
#include <memory>

class osd
{
    public:
        std::vector<lv_obj_t *> user_tabs;

        virtual ~osd();

        std::vector<unsigned short> unpause_actions;
        std::vector<unsigned short> pause_actions;

        virtual int pause();
        virtual int resume();
};

std::unique_ptr<class osd> osd_load_custom(const std::string &fname, lv_obj_t *hidden_tv);
std::unique_ptr<class osd> osd_load_default(lv_obj_t *hidden_tv);

#endif
