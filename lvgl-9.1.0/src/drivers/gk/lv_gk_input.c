#include "../../core/lv_group.h"
#include "../../stdlib/lv_string.h"
#include "lv_gk_input.h"
#include "../../indev/lv_indev.h"
#include "gk.h"
#include "_gk_event.h"

static void gk_kbd_read_cb(lv_indev_t *indev, lv_indev_data_t *data);
static void gk_mouse_read_cb(lv_indev_t *indev, lv_indev_data_t *data);
static void gk_touch_read_cb(lv_indev_t *indev, lv_indev_data_t *data);
static void gk_update_state();
static enum _lv_key_t gk_key_to_lv(unsigned short gkkey);

static lv_indev_data_t d_kbd, d_mouse, d_touch;

void lv_gk_register_inputs()
{
    lv_gk_kbd_create();
    lv_gk_mouse_create();
    lv_gk_touchscreen_create();
}

lv_indev_t *lv_gk_kbd_create()
{
    lv_indev_t *indev = lv_indev_create();
    if(indev == NULL) return NULL;

    d_kbd.key = 0;
    d_kbd.state = 0;

    lv_indev_set_type(indev, LV_INDEV_TYPE_KEYPAD);
    lv_indev_set_read_cb(indev, gk_kbd_read_cb);

    return indev;
}

lv_indev_t *lv_gk_mouse_create()
{
    lv_indev_t *indev = lv_indev_create();
    if(indev == NULL) return NULL;

    d_mouse.point.x = 0;
    d_mouse.point.y = 0;
    d_mouse.state = 0;

    lv_indev_set_type(indev, LV_INDEV_TYPE_POINTER);
    lv_indev_set_read_cb(indev, gk_mouse_read_cb);

    return indev;
}

lv_indev_t *lv_gk_touchscreen_create()
{
    lv_indev_t *indev = lv_indev_create();
    if(indev == NULL) return NULL;

    d_touch.point.x = 0;
    d_touch.point.y = 0;
    d_touch.state = 0;

    lv_indev_set_type(indev, LV_INDEV_TYPE_POINTER);
    lv_indev_set_read_cb(indev, gk_touch_read_cb);

    return indev;
}

void gk_kbd_read_cb(lv_indev_t *indev, lv_indev_data_t *data)
{
    gk_update_state();

    data->key = d_kbd.key;
    data->state = d_kbd.state;
}

void gk_mouse_read_cb(lv_indev_t *indev, lv_indev_data_t *data)
{
    gk_update_state();

    data->point.x = d_mouse.point.x;
    data->point.y = d_mouse.point.y;
    data->state = d_mouse.state;
}

void gk_touch_read_cb(lv_indev_t *indev, lv_indev_data_t *data)
{
    gk_update_state();

    data->point.x = d_touch.point.x;
    data->point.y = d_touch.point.y;
    data->state = d_touch.state;
}

void gk_update_state()
{
    struct Event ev;
    while(GK_EventPeek(&ev) > 0)
    {
        switch(ev.type)
        {
            case KeyDown:
                d_kbd.key = gk_key_to_lv(ev.key);
                d_kbd.state = LV_INDEV_STATE_PRESSED;
                break;
            case KeyUp:
                d_kbd.key = gk_key_to_lv(ev.key);
                d_kbd.state = LV_INDEV_STATE_RELEASED;
                break;
            case MouseDown:
                if(ev.mouse_data.buttons & 0x1)
                    d_mouse.state = LV_INDEV_STATE_PRESSED;
                break;
            case MouseUp:
                if(ev.mouse_data.buttons & 0x1)
                    d_mouse.state = LV_INDEV_STATE_RELEASED;
                break;
            case MouseMove:
                if(ev.mouse_data.is_rel)
                {
                    d_mouse.point.x += ev.mouse_data.x;
                    d_mouse.point.y += ev.mouse_data.y;
                }
                else
                {
                    d_mouse.point.x = ev.mouse_data.x;
                    d_mouse.point.y = ev.mouse_data.y;
                }
                if(d_mouse.point.x < 0) d_mouse.point.x = 0;
                if(d_mouse.point.x >= 640) d_mouse.point.x = 639;
                if(d_mouse.point.y < 0) d_mouse.point.y = 0;
                if(d_mouse.point.y >= 480) d_mouse.point.y = 479;
                break;

            default:
                break;
        }
    }
}

enum _lv_key_t gk_key_to_lv(unsigned short gkkey)
{
    switch(gkkey)
    {
        case 82:
            return LV_KEY_UP;
        case 81:
            return LV_KEY_DOWN;
        case 79:
            return LV_KEY_RIGHT;
        case 80:
            return LV_KEY_LEFT;
        case 41:
            return LV_KEY_ESC;
        case 76:
            return LV_KEY_DEL;
        case 42:
            return LV_KEY_BACKSPACE;
        case 40:
            return LV_KEY_ENTER;
        case 258:
            return LV_KEY_NEXT;
        case 259:
            return LV_KEY_PREV;
        case 74:
            return LV_KEY_HOME;
        case 77:
            return LV_KEY_END;
        default:
            return 0;
    }
}
