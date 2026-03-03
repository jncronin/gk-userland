#include "osd.h"
#include "INIReader.h"
#include "widget.h"
#include "gk.h"
#include <signal.h>
#include <vector>

int osd_load_gkmenu(lv_obj_t *parent);
int osd_load_ini(lv_obj_t *parent, const std::string &fname);
int osd_clear(lv_obj_t *parent);

static lv_obj_t *def_overlay_kill;
static unsigned short str_to_key(const std::string &s);
static void cosd_click_cb(lv_event_t *e);
static void cosd_clickquit_cb(lv_event_t *e);

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

static int getint(const std::string &s)
{
    return strtol(s.c_str(), nullptr, 0);
}

int osd_load_ini(lv_obj_t *parent, const std::string &fname)
{
    INIReader rdr(fname);

    fprintf(stderr, "load_ini: %s, parse error: %d\n", fname.c_str(), rdr.ParseError());

    for(const auto &sect : rdr.SectionLinenums())
    {
        lv_obj_t *cur = nullptr;
        lv_obj_t *lab = nullptr;

        auto sname = rdr.GetSection(sect);

        fprintf(stderr, "load_ini: section: %s\n", sname.c_str());
        if(sname == "label")
        {
            cur = gk_label_create(parent, "");
            lab = cur;
        }
        else if(sname == "button")
        {
            std::tie(cur, lab) = gk_btnlab_create(parent);
        }

        if(cur)
        {
            for(const auto &key : rdr.KeyLinenums(sect))
            {
                auto kname = rdr.GetKey(key);
                auto kval = rdr.Get(key);

                if(kname == "x")
                    lv_obj_set_x(cur, getint(kval));
                else if(kname == "y")
                    lv_obj_set_y(cur, getint(kval));
                else if(kname == "w")
                    lv_obj_set_width(cur, getint(kval));
                else if(kname == "h")
                    lv_obj_set_height(cur, getint(kval));
                else if(kname == "text")
                {
                    lv_label_set_text(lab, kval.c_str());
                }
                else if(kname == "click")
                {
                    auto i = str_to_key(kval);
                    lv_obj_add_event_cb(cur, cosd_click_cb, LV_EVENT_CLICKED, (void *)(uintptr_t)i);
                }
                else if(kname == "clickquit")
                {
                    if(kval == "true")
                    {
                        auto fpid = GK_GetFocusProcess();
                        lv_obj_add_event_cb(cur, cosd_clickquit_cb, LV_EVENT_CLICKED, (void *)(intptr_t)fpid);
                    }
                }
            }
        }
    }

    lv_obj_invalidate(parent);

    return 0;
}

void cosd_click_cb(lv_event_t *e)
{
    auto key = (unsigned short)(uintptr_t)lv_event_get_user_data(e);
    fprintf(stderr, "cosd: send key %u\n", key);

    auto fpid = GK_GetFocusProcess();

    Event ev_press;
    ev_press.type = Event::event_type_t::KeyDown;
    ev_press.key = key;
    GK_EventSend(fpid, &ev_press);

    Event ev_release;
    ev_release.type = Event::event_type_t::KeyUp;
    ev_release.key = key;
    GK_EventSend(fpid, &ev_release);
}

static void cosd_clickquick_delayed_cb(lv_timer_t *t)
{
    auto fpid = (pid_t)(intptr_t)lv_timer_get_user_data(t);
    kill(fpid, SIGKILL);
}

void cosd_clickquit_cb(lv_event_t *e)
{
    auto fpid = lv_event_get_user_data(e);

    // schedule a callback for 1 s time to actually kill the process if not already by other keystrokes
    auto t = lv_timer_create(cosd_clickquick_delayed_cb, 1000, fpid);
    lv_timer_set_repeat_count(t, 1);
}

#define SK(x) if(s == #x) return x;

static unsigned short _str_to_key(const std::string &s)
{
    SK(GK_SCANCODE_0);
    SK(GK_SCANCODE_1);
    SK(GK_SCANCODE_2);
    SK(GK_SCANCODE_3);
    SK(GK_SCANCODE_4);
    SK(GK_SCANCODE_5);
    SK(GK_SCANCODE_6);
    SK(GK_SCANCODE_7);
    SK(GK_SCANCODE_8);
    SK(GK_SCANCODE_9);
    SK(GK_SCANCODE_Q);
    SK(GK_SCANCODE_W);
    SK(GK_SCANCODE_E);
    SK(GK_SCANCODE_R);
    SK(GK_SCANCODE_T);
    SK(GK_SCANCODE_Y);
    SK(GK_SCANCODE_U);
    SK(GK_SCANCODE_I);
    SK(GK_SCANCODE_O);
    SK(GK_SCANCODE_P);
    SK(GK_SCANCODE_A);
    SK(GK_SCANCODE_S);
    SK(GK_SCANCODE_D);
    SK(GK_SCANCODE_F);
    SK(GK_SCANCODE_G);
    SK(GK_SCANCODE_H);
    SK(GK_SCANCODE_J);
    SK(GK_SCANCODE_K);
    SK(GK_SCANCODE_L);
    SK(GK_SCANCODE_Z);
    SK(GK_SCANCODE_X);
    SK(GK_SCANCODE_C);
    SK(GK_SCANCODE_V);
    SK(GK_SCANCODE_B);
    SK(GK_SCANCODE_N);
    SK(GK_SCANCODE_M);
    SK(GK_SCANCODE_F1);
    SK(GK_SCANCODE_F2);
    SK(GK_SCANCODE_F3);
    SK(GK_SCANCODE_F4);
    SK(GK_SCANCODE_F5);
    SK(GK_SCANCODE_F6);
    SK(GK_SCANCODE_F7);
    SK(GK_SCANCODE_F8);
    SK(GK_SCANCODE_F9);
    SK(GK_SCANCODE_F10);
    SK(GK_SCANCODE_F11);
    SK(GK_SCANCODE_F12);
    SK(GK_SCANCODE_RETURN);
    SK(GK_SCANCODE_LCTRL);
    SK(GK_SCANCODE_LALT);
    SK(GK_SCANCODE_LSHIFT);
    SK(GK_SCANCODE_RCTRL);
    SK(GK_SCANCODE_RALT);
    SK(GK_SCANCODE_RSHIFT);
    SK(GK_SCANCODE_SPACE);
    SK(GK_SCANCODE_COMMA);
    SK(GK_SCANCODE_PERIOD);
    SK(GK_SCANCODE_MINUS);
    SK(GK_SCANCODE_EQUALS);
    SK(GK_SCANCODE_GRAVE);
    SK(GK_SCANCODE_KP_0);
    SK(GK_SCANCODE_KP_1);
    SK(GK_SCANCODE_KP_2);
    SK(GK_SCANCODE_KP_3);
    SK(GK_SCANCODE_KP_4);
    SK(GK_SCANCODE_KP_5);
    SK(GK_SCANCODE_KP_6);
    SK(GK_SCANCODE_KP_7);
    SK(GK_SCANCODE_KP_8);
    SK(GK_SCANCODE_KP_9);
    SK(GK_SCANCODE_KP_PERCENT);
    SK(GK_SCANCODE_KP_PERIOD);
    SK(GK_SCANCODE_KP_PLUS);
    SK(GK_SCANCODE_KP_PLUSMINUS);
    SK(GK_SCANCODE_KP_DIVIDE);
    SK(GK_SCANCODE_KP_ENTER);
    SK(GK_SCANCODE_KP_MULTIPLY);
    SK(GK_SCANCODE_KP_MINUS);
    SK(GK_SCANCODE_INSERT);
    SK(GK_SCANCODE_DELETE);
    SK(GK_SCANCODE_PRINTSCREEN);
    SK(GK_SCANCODE_LEFTBRACKET);
    SK(GK_SCANCODE_RIGHTBRACKET);
    SK(GK_SCANCODE_VOLUMEUP);
    SK(GK_SCANCODE_VOLUMEDOWN);
    SK(GK_SCANCODE_TAB);
    SK(GK_SCANCODE_WWW);
    SK(GK_SCANCODE_MENU);
    SK(GK_SCANCODE_APP1);
    SK(GK_SCANCODE_APP2);
    SK(GK_SCANCODE_APPLICATION);
    SK(GK_SCANCODE_PAUSE);
    SK(GK_SCANCODE_ESCAPE);
    SK(GK_MODIFIER_SHIFT);
    SK(GK_MODIFIER_CTRL);
    SK(GK_MODIFIER_ALT);

    /* Interpret as integer */
    return atoi(s.c_str());
    
    return 0;
}

unsigned short str_to_key(const std::string &s)
{
    unsigned short ret = 0;
    size_t pos_start = 0, pos_end;
    while((pos_end = s.find("|", pos_start)) != std::string::npos)
    {
        std::string token = s.substr(pos_start, pos_end - pos_start);
        pos_start = pos_end + 1;
        ret |= _str_to_key(token);
    }
    ret |= _str_to_key(s.substr(pos_start));
    return ret;
}
