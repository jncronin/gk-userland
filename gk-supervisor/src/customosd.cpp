#include "osd.h"
#include "INIReader.h"
#include "widget.h"
#include "gk.h"
#include <signal.h>

int osd_load_gkmenu(lv_obj_t *parent);
int osd_load_ini(lv_obj_t *parent, const std::string &fname);
int osd_clear(lv_obj_t *parent);

static lv_obj_t *def_overlay_kill;

static void kill_click(lv_event_t *)
{
    auto fpid = GK_GetFocusProcess();
    char fproc_name[256];
    if(GK_GetProcessName(fpid, fproc_name, sizeof(fproc_name) - 1) == 0)
    {
        fproc_name[sizeof(fproc_name) - 1] = 0;
        if(strcmp("gkmenu", fproc_name))
        {
            // not gkmenu - proceed to kill
            kill(fpid, SIGKILL);
        }
    }
}

int osd_load_custom(lv_obj_t *parent, const std::string &fname)
{
    osd_clear(parent);

    if(fname == "gkmenu.osd")
    {
        // special case gkmenu
        return osd_load_gkmenu(parent);
    }

    if(fname.ends_with(".ini"))
    {
        return osd_load_ini(parent, fname);
    }

    fprintf(stderr, "osd: don't know how to load %s\n", fname.c_str());
    return -1;
}

int osd_clear(lv_obj_t *parent)
{
    lv_obj_clean(parent);
    return 0;
}

int osd_load_gkmenu(lv_obj_t *parent)
{
    // gkmenu doesn't have an osd
    return 0;
}

int osd_load_default(lv_obj_t *parent)
{
    def_overlay_kill = gk_btn_create(parent, "Quit");
    lv_obj_center(def_overlay_kill);
    lv_obj_set_size(def_overlay_kill, 120, 80);
    lv_obj_add_event_cb(def_overlay_kill, kill_click, LV_EVENT_CLICKED, nullptr);

    return 0;
}

int osd_load_ini(lv_obj_t *parent, const std::string &fname)
{
    // for now, just print its name
    auto l = gk_label_create(parent, fname);
    lv_obj_center(l);

    return 0;
}
