#include "../../core/lv_group.h"
#include "../../stdlib/lv_string.h"
#include "lv_gk_input.h"
#include "../../indev/lv_indev.h"
#include "../../core/lv_obj.h"
#include "../../display/lv_display.h"
#include "gk.h"
#include "_gk_event.h"

static void gk_kbd_read_cb(lv_indev_t *indev, lv_indev_data_t *data);
static void gk_mouse_read_cb(lv_indev_t *indev, lv_indev_data_t *data);
static void gk_update_state();
static enum _lv_key_t gk_key_to_lv(unsigned short gkkey);

/* Simple FIFO ring buffer, not thread safe */
#define GK_RB_SIZE 8
struct gk_rb
{
    lv_indev_data_t d[GK_RB_SIZE];
    unsigned int rp, wp, init;
};

static int gk_rb_push(struct gk_rb *rb, const lv_indev_data_t *d);
static int gk_rb_pop(struct gk_rb *rb, lv_indev_data_t *d);
static unsigned int ptr_plus_one(unsigned int ptr);

static struct gk_rb d_mouse, d_kbd;

/* cache the previously returned value */
static lv_indev_data_t cur_mouse, cur_kbd;

void lv_gk_register_inputs()
{
    lv_gk_kbd_create();
    lv_gk_mouse_create();
}

lv_indev_t *lv_gk_kbd_create()
{
    lv_indev_t *indev = lv_indev_create();
    if(indev == NULL) return NULL;

    cur_kbd.btn_id = 0;
    cur_kbd.key = 0;
    cur_kbd.state = 0;
    cur_kbd.continue_reading = 0;

    d_kbd.rp = 0;
    d_kbd.wp = 0;
    d_kbd.init = 1;

    lv_indev_set_type(indev, LV_INDEV_TYPE_KEYPAD);
    lv_indev_set_read_cb(indev, gk_kbd_read_cb);

    return indev;
}

lv_indev_t *lv_gk_mouse_create()
{
    lv_indev_t *indev = lv_indev_create();
    if(indev == NULL) return NULL;

    cur_mouse.point.x = 0;
    cur_mouse.point.y = 0;
    cur_mouse.state = 0;
    cur_mouse.continue_reading = 0;

    d_mouse.rp = 0;
    d_mouse.wp = 0;
    d_mouse.init = 1;

    lv_indev_set_type(indev, LV_INDEV_TYPE_POINTER);
    lv_indev_set_read_cb(indev, gk_mouse_read_cb);

    return indev;
}

void gk_kbd_read_cb(lv_indev_t *indev, lv_indev_data_t *data)
{
    gk_update_state();

    gk_rb_pop(&d_kbd, &cur_kbd);
    memcpy(data, &cur_kbd, sizeof(lv_indev_data_t));
}

void gk_mouse_read_cb(lv_indev_t *indev, lv_indev_data_t *data)
{
    gk_update_state();

    gk_rb_pop(&d_mouse, &cur_mouse);
    memcpy(data, &cur_mouse, sizeof(lv_indev_data_t));
}

void gk_update_state()
{
    struct Event ev;
    while(GK_EventPeek(&ev) > 0)
    {
        lv_indev_data_t d;
        memset(&d, 0, sizeof(lv_indev_data_t));

        switch(ev.type)
        {
            case KeyDown:
                d.key = gk_key_to_lv(ev.key);
                d.state = LV_INDEV_STATE_PRESSED;
                gk_rb_push(&d_kbd, &d);
                break;
            case KeyUp:
                d.key = gk_key_to_lv(ev.key);
                d.state = LV_INDEV_STATE_RELEASED;
                gk_rb_push(&d_kbd, &d);
                break;
            case MouseDown:
            case MouseUp:
            case MouseMove:
                if(ev.mouse_data.buttons & 0x1)
                {
                    d.state = (ev.type == MouseUp) ? LV_INDEV_STATE_RELEASED : LV_INDEV_STATE_PRESSED;
                }

                if(ev.mouse_data.is_rel)
                {
                    d.point.x += ev.mouse_data.x;
                    d.point.y += ev.mouse_data.y;
                }
                else
                {
                    d.point.x = ev.mouse_data.x;
                    d.point.y = ev.mouse_data.y;
                }
                if(d.point.x < 0) d.point.x = 0;
                if(d.point.x >= 640) d.point.x = 639;
                if(d.point.y < 0) d.point.y = 0;
                if(d.point.y >= 480) d.point.y = 479;

                gk_rb_push(&d_mouse, &d);

                break;

            case RefreshScreen:
                lv_obj_invalidate(lv_screen_active());
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

static int gk_rb_push(struct gk_rb *rb, const lv_indev_data_t *d)
{
    if(!rb->init)
        return -1;

    unsigned int new_wp = ptr_plus_one(rb->wp);
    if(new_wp == rb->rp)
    {
        return -1;
    }
    memcpy(&rb->d[rb->wp], d, sizeof(lv_indev_data_t));
    rb->wp = new_wp;
    return 1;
}

static int gk_rb_pop(struct gk_rb *rb, lv_indev_data_t *d)
{
    if(!rb->init)
        return -1;
    
    if(rb->rp == rb->wp)
    {
        return 0;
    }

    memcpy(d, &rb->d[rb->rp], sizeof(lv_indev_data_t));
    rb->rp = ptr_plus_one(rb->rp);
    if(rb->rp == rb->wp)
    {
        d->continue_reading = 0;
    }
    else
    {
        d->continue_reading = 1;
    }

    return 1;
}

unsigned int ptr_plus_one(unsigned int ptr)
{
    ptr++;
    if(ptr >= GK_RB_SIZE)
        ptr = 0;
    return ptr;
}
