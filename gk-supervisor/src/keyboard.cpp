#include <lvgl/lvgl.h>
#include "gk.h"
#include "_gk_scancodes.h"
#include <cstdio>

static const char * const lowercase_map[] = 
{
    "Esc", "F1", "F2", "F3", "F4", "F5", "F6", "F7", "F8", "F9", "F10", "F11", "F12", "PrtSc", "Pause", "Ins", "Del", "\n",
    "`", "1", "2", "3", "4", "5", "6", "7", "8", "9", "0", "-", "=", "<-", "Home", "\n",
    "Tab", "q", "w", "e", "r", "t", "y", "u", "i", "o", "p", "[", "]", "\\", "PgUp", "\n",
    "Caps", "a", "s", "d", "f", "g", "h", "j", "k", "l", ";", "\'", LV_SYMBOL_NEW_LINE, "PgDn", "\n",
    "Shift", "z", "x", "c", "v", "b", "n", "m", ",", ".", "/", "Shift", LV_SYMBOL_UP, "End", "\n",
    "Ctrl", LV_SYMBOL_LIST, "Alt", " ", "Alt", LV_SYMBOL_SETTINGS, "Ctrl", LV_SYMBOL_LEFT, LV_SYMBOL_DOWN, LV_SYMBOL_RIGHT,
    ""
};

static const char * const uppercase_map[] = 
{
    "Esc", "F1", "F2", "F3", "F4", "F5", "F6", "F7", "F8", "F9", "F10", "F11", "F12", "PrtSc", "Pause", "Ins", "Del", "\n",
    "`", "!", "\"", "#", "$", "%", "^", "&", "*", "(", ")", "_", "+", "<-", "Home", "\n",
    "Tab", "Q", "W", "E", "R", "T", "Y", "U", "I", "O", "P", "{", "}", "|", "PgUp", "\n",
    "Caps", "A", "S", "D", "F", "G", "H", "J", "K", "L", ":", "@", LV_SYMBOL_NEW_LINE, "PgDn", "\n",
    "Shift", "Z", "X", "C", "V", "B", "N", "M", "<", ">", "?", "Shift", LV_SYMBOL_UP, "End", "\n",
    "Ctrl", LV_SYMBOL_LIST, "Alt", " ", "Alt", LV_SYMBOL_SETTINGS, "Ctrl", LV_SYMBOL_LEFT, LV_SYMBOL_DOWN, LV_SYMBOL_RIGHT,
    ""
};

#define W5 LV_BUTTONMATRIX_CTRL_WIDTH_5
#define W8 LV_BUTTONMATRIX_CTRL_WIDTH_8
#define W9 LV_BUTTONMATRIX_CTRL_WIDTH_9
#define W10 LV_BUTTONMATRIX_CTRL_WIDTH_10
#define W15 LV_BUTTONMATRIX_CTRL_WIDTH_15

//#define CH(x) (lv_buttonmatrix_ctrl_t)((x) | LV_BUTTONMATRIX_CTRL_CHECKABLE)
#define CH(x) (x)
#define CHECKED(x) (lv_buttonmatrix_ctrl_t)((x) | LV_BUTTONMATRIX_CTRL_CHECKED)

static const lv_buttonmatrix_ctrl_t ctrl_map[] =
{
    W8, W5, W5, W5, W5, W5, W5, W5, W5, W5, W5, W5, W5, W8, W8, W8, W8,             // 0
    W5, W10, W10, W10, W10, W10, W10, W10, W10, W10, W10, W10, W10, W10, W10,       // 17
    W8, W5, W5, W5, W5, W5, W5, W5, W5, W5, W5, W5, W5, W5, W5,                     // 32
    CH(W10), W10, W10, W10, W10, W10, W10, W10, W10, W10, W10, W10, W10, W10,       // 47
    CH(W8), W5, W5, W5, W5, W5, W5, W5, W5, W5, W5, CH(W5), W5, W5,                 // 61
    CH(W5), W5, CH(W5), W15, CH(W5), W5, CH(W5), W5, W5, W5,                        // 75
};

#define IDX_CAPSLOCK        47
#define IDX_LSHIFT          61
#define IDX_RSHIFT          72
#define IDX_LCTRL           75
#define IDX_LALT            77
#define IDX_RALT            79
#define IDX_RCTRL           81

static const unsigned int scancodes[] =
{
    GK_SCANCODE_ESCAPE, GK_SCANCODE_F1, GK_SCANCODE_F2, GK_SCANCODE_F3, GK_SCANCODE_F4, GK_SCANCODE_F5,
        GK_SCANCODE_F6, GK_SCANCODE_F7, GK_SCANCODE_F8, GK_SCANCODE_F9, GK_SCANCODE_F10, GK_SCANCODE_F11,
        GK_SCANCODE_F12, GK_SCANCODE_PRINTSCREEN, GK_SCANCODE_PAUSE, GK_SCANCODE_INSERT, GK_SCANCODE_DELETE,
    GK_SCANCODE_GRAVE, GK_SCANCODE_1, GK_SCANCODE_2, GK_SCANCODE_3, GK_SCANCODE_4, GK_SCANCODE_5,
        GK_SCANCODE_6, GK_SCANCODE_7, GK_SCANCODE_8, GK_SCANCODE_9, GK_SCANCODE_0, GK_SCANCODE_MINUS,
        GK_SCANCODE_EQUALS, GK_SCANCODE_BACKSPACE, GK_SCANCODE_HOME,
    GK_SCANCODE_TAB, GK_SCANCODE_Q, GK_SCANCODE_W, GK_SCANCODE_E, GK_SCANCODE_R, GK_SCANCODE_T,
        GK_SCANCODE_Y, GK_SCANCODE_U, GK_SCANCODE_I, GK_SCANCODE_O, GK_SCANCODE_P, GK_SCANCODE_LEFTBRACKET,
        GK_SCANCODE_RIGHTBRACKET, GK_SCANCODE_BACKSLASH, GK_SCANCODE_PAGEUP,
    GK_SCANCODE_CAPSLOCK, GK_SCANCODE_A, GK_SCANCODE_S, GK_SCANCODE_D, GK_SCANCODE_F, GK_SCANCODE_G,
        GK_SCANCODE_H, GK_SCANCODE_J, GK_SCANCODE_K, GK_SCANCODE_L, GK_SCANCODE_SEMICOLON,
        GK_SCANCODE_APOSTROPHE, GK_SCANCODE_RETURN, GK_SCANCODE_PAGEDOWN,
    GK_SCANCODE_LSHIFT, GK_SCANCODE_Z, GK_SCANCODE_X, GK_SCANCODE_C, GK_SCANCODE_V, GK_SCANCODE_B,
        GK_SCANCODE_N, GK_SCANCODE_M, GK_SCANCODE_COMMA, GK_SCANCODE_PERIOD, GK_SCANCODE_SLASH,
        GK_SCANCODE_RSHIFT, GK_SCANCODE_UP, GK_SCANCODE_END,
    GK_SCANCODE_LCTRL, GK_SCANCODE_LGUI, GK_SCANCODE_LALT, GK_SCANCODE_SPACE, GK_SCANCODE_RALT,
        GK_SCANCODE_APPLICATION, GK_SCANCODE_RCTRL, GK_SCANCODE_LEFT, GK_SCANCODE_DOWN, GK_SCANCODE_RIGHT
};

struct gk_kbd_data
{
    bool is_shift = false;
    bool is_alt = false;
    bool is_ctrl = false;
    bool is_capslock = false;
};

static void gk_kbd_click(lv_event_t *e);

static void gk_kbd_handler(unsigned int sc);

lv_obj_t *gk_kbd_create(lv_obj_t *parent)
{
    auto k = lv_btnmatrix_create(parent);

    auto ud = new gk_kbd_data();

    lv_buttonmatrix_set_map(k, lowercase_map);
    lv_buttonmatrix_set_ctrl_map(k, ctrl_map);

    lv_obj_add_event_cb(k, gk_kbd_click, LV_EVENT_VALUE_CHANGED, ud);
    lv_obj_add_event_cb(k,
        [](lv_event_t *e) { auto _ud = lv_event_get_user_data(e); if(_ud) delete (gk_kbd_data *)_ud; },
        LV_EVENT_DELETE, ud);

    return k;
}

static void set_checked(lv_obj_t *k, unsigned int idx, bool checked)
{
    if(checked)
        lv_buttonmatrix_set_button_ctrl(k, idx, LV_BUTTONMATRIX_CTRL_CHECKED);
    else
        lv_buttonmatrix_clear_button_ctrl(k, idx, LV_BUTTONMATRIX_CTRL_CHECKED);
}

void gk_kbd_click(lv_event_t *e)
{
    auto k = (lv_obj_t *)lv_event_get_target(e);
    auto ud = (gk_kbd_data *)lv_event_get_user_data(e);

    auto idx = lv_btnmatrix_get_selected_btn(k);

    if(idx >= sizeof(scancodes) / sizeof(scancodes[0]))
    {
        return;
    }

    auto sc = scancodes[idx];

    fprintf(stderr, "kbd: idx: %u, sc: %u\n", idx, sc);

    bool cur_uppercase = ud->is_shift || ud->is_capslock;

    bool is_capslock = sc == GK_SCANCODE_CAPSLOCK;
    bool is_shift = (sc == GK_SCANCODE_LSHIFT) || (sc == GK_SCANCODE_RSHIFT);
    bool is_ctrl = (sc == GK_SCANCODE_LCTRL) || (sc == GK_SCANCODE_RCTRL);
    bool is_alt = (sc == GK_SCANCODE_LALT) || (sc == GK_SCANCODE_RALT);

    if(is_capslock)
    {
        ud->is_capslock = !ud->is_capslock;
        set_checked(k, IDX_CAPSLOCK, ud->is_capslock);
        gk_kbd_handler(sc);
    }
    else if(is_shift)
    {
        ud->is_shift = !ud->is_shift;
        set_checked(k, IDX_LSHIFT, ud->is_shift);
        set_checked(k, IDX_RSHIFT, ud->is_shift);
        gk_kbd_handler(sc);
    }
    else if(is_ctrl)
    {
        ud->is_ctrl = !ud->is_ctrl;
        set_checked(k, IDX_LCTRL, ud->is_ctrl);
        set_checked(k, IDX_RCTRL, ud->is_ctrl);
        gk_kbd_handler(sc);
    }
    else if(is_alt)
    {
        ud->is_alt = !ud->is_alt;
        set_checked(k, IDX_LALT, ud->is_alt);
        set_checked(k, IDX_RALT, ud->is_alt);
        gk_kbd_handler(sc);
    }
    else
    {
        auto shift_mod = (ud->is_shift || ud->is_capslock) ? GK_MODIFIER_SHIFT : 0;
        auto ctrl_mod = ud->is_ctrl ? GK_MODIFIER_CTRL : 0;
        auto alt_mod = ud->is_alt ? GK_MODIFIER_ALT : 0;
        gk_kbd_handler(sc | shift_mod | ctrl_mod | alt_mod);

        // any other key press cancels modifiers (except for caps lock)
        if(ud->is_shift)
        {
            ud->is_shift = false;
            set_checked(k, IDX_LSHIFT, false);
            set_checked(k, IDX_RSHIFT, false);
        }
        if(ud->is_ctrl)
        {
            ud->is_ctrl = false;
            set_checked(k, IDX_LCTRL, false);
            set_checked(k, IDX_RCTRL, false);
        }
        if(ud->is_alt)
        {
            ud->is_alt = false;
            set_checked(k, IDX_LALT, false);
            set_checked(k, IDX_RALT, false);
        }
    }

    bool new_uppercase = ud->is_shift || ud->is_capslock;

    if(new_uppercase != cur_uppercase)
    {
        lv_buttonmatrix_set_map(k, new_uppercase ? uppercase_map : lowercase_map);
    }
}

void gk_kbd_handler(unsigned int sc)
{
    auto fpid = GK_GetFocusProcess();

    Event ev_press;
    ev_press.type = Event::event_type_t::KeyDown;
    ev_press.key = sc;
    GK_EventSend(fpid, &ev_press);

    Event ev_release;
    ev_release.type = Event::event_type_t::KeyUp;
    ev_release.key = sc;
    GK_EventSend(fpid, &ev_release);
}
