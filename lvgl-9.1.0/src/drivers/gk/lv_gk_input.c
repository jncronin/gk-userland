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
#define _SCCHAR(x) *#x
#define _SC(x) case GK_SCANCODE_##x: \
        return _SCCHAR(x);

enum _lv_key_t gk_key_to_lv(unsigned short gkkey)
{
    switch(gkkey)
    {
        _SC(0);
        _SC(1);
        _SC(2);
        _SC(3);
        _SC(4);
        _SC(5);
        _SC(6);
        _SC(7);
        _SC(8);
        _SC(9);
        _SC(A);
        _SC(B);
        _SC(C);
        _SC(D);
        _SC(E);
        _SC(F);
        _SC(G);
        _SC(H);
        _SC(I);
        _SC(J);
        _SC(K);
        _SC(L);
        _SC(M);
        _SC(N);
        _SC(O);
        _SC(P);
        _SC(Q);
        _SC(R);
        _SC(S);
        _SC(T);
        _SC(U);
        _SC(V);
        _SC(W);
        _SC(X);
        _SC(Y);
        _SC(Z);
        case GK_SCANCODE_UP:
            return LV_KEY_UP;
        case GK_SCANCODE_DOWN:
            return LV_KEY_DOWN;
        case GK_SCANCODE_RIGHT:
            return LV_KEY_RIGHT;
        case GK_SCANCODE_LEFT:
            return LV_KEY_LEFT;
        case GK_SCANCODE_ESCAPE:
            return LV_KEY_ESC;
        case GK_SCANCODE_DELETE:
            return LV_KEY_DEL;
        case GK_SCANCODE_BACKSPACE:
            return LV_KEY_BACKSPACE;
        case GK_SCANCODE_RETURN:
            return LV_KEY_ENTER;
        case GK_SCANCODE_AUDIONEXT:
            return LV_KEY_NEXT;
        case GK_SCANCODE_AUDIOPREV:
            return LV_KEY_PREV;
        case GK_SCANCODE_HOME:
            return LV_KEY_HOME;
        case GK_SCANCODE_END:
            return LV_KEY_END;
        case GK_SCANCODE_SLASH:
            return '/';
        case GK_SCANCODE_BACKSLASH:
            return '\\';
        case GK_SCANCODE_GRAVE:
            return '`';
        case GK_SCANCODE_MINUS:
            return '-';
        case GK_SCANCODE_EQUALS:
            return '=';
        case GK_SCANCODE_TAB:
            return '\t';
        case GK_SCANCODE_LEFTBRACKET:
            return '[';
        case GK_SCANCODE_RIGHTBRACKET:
            return ']';
        case GK_SCANCODE_SEMICOLON:
            return ';';
        case GK_SCANCODE_APOSTROPHE:
            return '\'';
        case GK_SCANCODE_COMMA:
            return ',';
        case GK_SCANCODE_PERIOD:
            return '.';

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
