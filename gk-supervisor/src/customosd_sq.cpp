#include <squirrel.h>
#include <sqstdio.h>
#include <cstdio>
#include <cstdarg>
#include <string>
#include <cstring>
#include <unordered_map>
#include <vector>
#include <lvgl.h>
#include <assert.h>
#include <unistd.h>
#include "customosd_sq.h"
#include "styles.h"
#include "dialogbox.h"
#include "toasts.h"
#include "img.h"
#include "_gk_scancodes.h"

#ifndef SUPERVISOR_SIMULATOR
#include <gk.h>
#include <_gk_event.h>

void close_supervisor();
#endif

extern lv_group_t *grp;
extern bool focus_obj;

static SQInteger class_set(HSQUIRRELVM v);
static SQInteger log_func(HSQUIRRELVM v);
static SQInteger dialog_func(HSQUIRRELVM v);
static SQInteger toast_func(HSQUIRRELVM v);
static SQInteger sendkeyevent_func(HSQUIRRELVM v);
static SQInteger loadosd_func(HSQUIRRELVM v);
static SQInteger delaykill_func(HSQUIRRELVM v);
static SQInteger close_func(HSQUIRRELVM v);
static SQInteger delay_func(HSQUIRRELVM v);

#define ADD_DEFINE(x) \
    sq_pushstring(v, #x, -1); \
    sq_pushinteger(v, x); \
    sq_newslot(v, -3, false);

void dump_item(HSQUIRRELVM v, int i, bool quote_strings = true);
void dump_stack(HSQUIRRELVM v, bool quote_strings = true);

// storage for event closures
struct sq_event
{
    HSQUIRRELVM v;
    HSQOBJECT obj;
    HSQOBJECT closure;
};

class sq_context : public osd
{
    public:
        HSQUIRRELVM v;

        // mapping from HOBJ to class type
        std::unordered_map<SQInstance *, size_t> obj_map;

        // mapping from HOBJ to LVGL object
        std::unordered_map<SQInstance *, lv_obj_t *> lvgl_map;

        // events
        std::vector<sq_event> events;

        // images
        gk_img_context imgs;

        // pause, resume
        int ev_pause = -1;
        int ev_resume = -1;
        int pause();
        int resume();

        lv_obj_t *tv_container;     // just for initial add - changed later in recreate_tabview()

        ~sq_context();
};

#define STACK_LEN(x) printf("%s: stack top: %lld\n", x, sq_gettop(v));


// custom typeof for the various object classes
#define MAKE_TYPEOF(NAME) \
static SQInteger NAME ## _typeof(HSQUIRRELVM v) \
{ \
    sq_pushstring(v, #NAME, -1); \
    return 1; \
}

MAKE_TYPEOF(Label)
MAKE_TYPEOF(Button)
MAKE_TYPEOF(Image)
MAKE_TYPEOF(Object)
MAKE_TYPEOF(TabViewPage)

#define ID_LABEL    0
#define ID_BUTTON   1
#define ID_IMAGE    2
#define ID_OBJECT   3
#define ID_TABVIEWPAGE  4

struct lvgl_class_t
{
    std::string name;
    SQInteger (*typeof_func)(HSQUIRRELVM);
};

#define MAKE_LVGL_CLASS_T(NAME) { #NAME, NAME ## _typeof }

const lvgl_class_t class_types[] =
{
    MAKE_LVGL_CLASS_T(Label),
    MAKE_LVGL_CLASS_T(Button),
    MAKE_LVGL_CLASS_T(Image),
    MAKE_LVGL_CLASS_T(Object),
    MAKE_LVGL_CLASS_T(TabViewPage)
};

static inline sq_context *get_ctx(HSQUIRRELVM v)
{
    return (sq_context *)sq_getforeignptr(v);
}

static void class_event_cb(lv_event_t *e);

SQInteger class_constructor(HSQUIRRELVM v)
{
    //printf("class_constructor\n");
    auto ctx = get_ctx(v);

    // try and get type
    sq_typeof(v, 1);
    const SQChar *classname = nullptr;
    sq_getstring(v, -1, &classname);
    sq_pop(v, 1);

    //printf("type: %s\n", classname);

    lv_obj_t *pobj = nullptr;

    // TabViewPages have no parent
    if(strcmp("TabViewPage", classname))
    {
        HSQOBJECT pp;
        auto pget = sq_getstackobj(v, 2, &pp);
        if(SQ_FAILED(pget))
        {
            printf("f1\n");
            return pget;
        }
        if(!pp._type == OT_INSTANCE)
        {
            printf("f2\n");
            return SQ_ERROR;
        }
        auto piter = ctx->lvgl_map.find(pp._unVal.pInstance);
        if(piter == ctx->lvgl_map.end())
        {
            printf("could not find parent obj %p\n", pp._unVal.pInstance);
            return SQ_ERROR;
        }
        pobj = piter->second;
    }

    // get class ID (index in class_types)
    for(auto i = 0U; i < sizeof(class_types) / sizeof(class_types[0]); i++)
    {
        if(!strcmp(class_types[i].name.c_str(), classname))
        {
            //printf("ID: %u\n", i);

            // now associate the HOBJ with the ID
            HSQOBJECT po;
            sq_getstackobj(v, 1, &po);

            if(po._type == OT_INSTANCE)
            {
                sq_addref(v, &po);
                ctx->obj_map[po._unVal.pInstance] = i;

                //printf("map %p to %u\n", po._unVal.pInstance, i);

                {
                    // actually create lvgl object here, then save in a separate map
                    lv_obj_t *obj = nullptr;

                    switch(i)
                    {
                        case ID_LABEL:
                            obj = lv_label_create(pobj);
                            lv_obj_add_style(obj, &style_text, 0);
                            lv_obj_set_style_text_font(obj, &lv_font_montserrat_20, 0);
                            break;
                        case ID_BUTTON:
                            obj = lv_button_create(pobj);
                            lv_obj_add_style(obj, &style_button, 0);
                            lv_obj_add_style(obj, &style_btn_focus, LV_STATE_FOCUS_KEY);
                            lv_group_add_obj(grp, pobj);

                            if(!focus_obj)
                            {
                                lv_group_focus_obj(pobj);
                                lv_obj_set_state(pobj, LV_STATE_FOCUS_KEY, true);
                                focus_obj = true;
                            }

                            break;
                        case ID_IMAGE:
                            obj = lv_img_create(pobj);
                            break;
                        case ID_OBJECT:
                            obj = lv_obj_create(pobj);
                            lv_obj_add_style(obj, &style_transp, 0);
                            break;
                        case ID_TABVIEWPAGE:
                            obj = lv_obj_create(ctx->tv_container);
                            lv_obj_add_style(obj, &style_transp, 0);
                            lv_obj_set_size(obj, 800, 240-32);
                            ctx->user_tabs.push_back(obj);
                            break;
                    }

                    if(obj)
                    {
                        lv_obj_set_user_data(obj, ctx);
                        ctx->lvgl_map[po._unVal.pInstance] = obj;
                    }
                }
            }
            else
            {
                printf("not an OT_INSTANCE\n");
            }

            break;
        }
    }

    if(sq_gettop(v) > 1 && sq_gettype(v, -1) == OT_TABLE)
    {
        // handle setting multiple options at once
        // at this point we have a table at stack -1 and the object at 1 (with potentially a 'parent' inbetween)

        // iterate the table
        sq_pushnull(v);
        while(SQ_SUCCEEDED(sq_next(v, -2)))
        {
            // stack is now object, [parent], table, iterator, key, value
            // class_set expects ..., object, key, value
            sq_push(v, 1);
            sq_push(v, -3);
            sq_push(v, -3);

            // stack is now object, [parent], table, iterator, key, value, object, key, value
            class_set(v);

            // restore to just iterator
            sq_pop(v, 5);
        }
        // pop iterator
        sq_pop(v, 1);
    }

    return 0;
}

struct obj_def
{
    HSQOBJECT sq_obj;
    unsigned int obj_class = ~0U;
    lv_obj_t *obj = nullptr;
};

obj_def this_to_obj(HSQUIRRELVM v, int this_idx)
{
    auto ctx = get_ctx(v);
    obj_def ret;
    if(SQ_FAILED(sq_getstackobj(v, this_idx, &ret.sq_obj)))
    {
        return ret;
    }
    auto &po = ret.sq_obj;
    sq_addref(v, &po);
    if(!po._type == OT_INSTANCE)
    {
        sq_release(v, &po);
        return ret;
    }
    auto iter = ctx->obj_map.find(po._unVal.pInstance);
    if(iter == ctx->obj_map.end())
    {
        sq_release(v, &po);
        return ret;
    }
    ret.obj_class = iter->second;

    auto oiter = ctx->lvgl_map.find(po._unVal.pInstance);
    sq_release(v, &po);

    if(oiter == ctx->lvgl_map.end())
    {
        printf("lvgl object not found\n");
        return ret;
    }
    ret.obj = oiter->second;
    return ret;
}

sq_event stack_to_event(HSQUIRRELVM v, HSQOBJECT *sq_obj)
{
    sq_event ev;
    ev.closure._type = OT_NULL;
    ev.obj._type = OT_NULL;
    ev.v = v;

    // get the raw closure
    HSQOBJECT o_closure;
    if(!SQ_SUCCEEDED(sq_getstackobj(v, -1, &o_closure))) return ev;
    if(!o_closure._type == OT_CLOSURE) return ev;

    sq_addref(v, &o_closure);

    if(sq_obj)
    {
        sq_addref(v, sq_obj);
        ev.obj = *sq_obj;
    }
    ev.closure = o_closure;

    return ev;
}

SQInteger class_set(HSQUIRRELVM v)
{
    auto ctx = get_ctx(v);
    auto odef = this_to_obj(v, -3);
    if(odef.obj_class == ~0U)
    {
        printf("invalid class id\n");
        return SQ_ERROR;
    }

    // get function name
    const SQChar *func_name;
    auto gs_ret = sq_getstring(v, -2, &func_name);
    if(SQ_FAILED(gs_ret))
        return gs_ret;

    auto class_id = odef.obj_class;
    auto obj = odef.obj;
    
    //printf("set: %p %u %s: ", obj, class_id, func_name);

    bool handled = false;

    if(!strcmp("x", func_name))
    {
        SQInteger val;
        if(!SQ_SUCCEEDED(sq_getinteger(v, -1, &val))) return SQ_ERROR;
        //printf("x: %lld\n", val);
        lv_obj_set_x(obj, val);
        handled = true;
    }
    else if(!strcmp("y", func_name))
    {
        SQInteger val;
        if(!SQ_SUCCEEDED(sq_getinteger(v, -1, &val))) return SQ_ERROR;
        //printf("y: %lld\n", val);
        lv_obj_set_y(obj, val);
        handled = true;
    }
    else if(!strcmp("w", func_name))
    {
        SQInteger val;
        if(!SQ_SUCCEEDED(sq_getinteger(v, -1, &val))) return SQ_ERROR;
        //printf("w: %lld\n", val);
        lv_obj_set_width(obj, val);
        handled = true;
    }
    else if(!strcmp("h", func_name))
    {
        SQInteger val;
        if(!SQ_SUCCEEDED(sq_getinteger(v, -1, &val))) return SQ_ERROR;
        //printf("h: %lld\n", val);
        lv_obj_set_height(obj, val);
        handled = true;
    }
    else if(!strcmp("align", func_name))
    {
        SQInteger val;
        if(!SQ_SUCCEEDED(sq_getinteger(v, -1, &val))) return SQ_ERROR;
        lv_obj_set_align(obj, (lv_align_t)val);
        handled = true;
    }
    else if(!strcmp("layout", func_name))
    {
        SQInteger val;
        if(!SQ_SUCCEEDED(sq_getinteger(v, -1, &val))) return SQ_ERROR;
        lv_obj_set_layout(obj, (uint32_t)val);
        handled = true;
    }
    else if(!strcmp("flex_flow", func_name))
    {
        SQInteger val;
        if(!SQ_SUCCEEDED(sq_getinteger(v, -1, &val))) return SQ_ERROR;
        lv_obj_set_flex_flow(obj, (lv_flex_flow_t)val);
        handled = true;
    }
    else if(!strcmp("flex_align", func_name))
    {
        SQInteger main, cross, track_cross;
        sq_pushinteger(v, 0);
        if(!SQ_SUCCEEDED(sq_get(v, -2))) return SQ_ERROR;
        if(!SQ_SUCCEEDED(sq_getinteger(v, -1, &main))) return SQ_ERROR;
        sq_poptop(v);
        sq_pushinteger(v, 1);
        if(!SQ_SUCCEEDED(sq_get(v, -2))) return SQ_ERROR;
        if(!SQ_SUCCEEDED(sq_getinteger(v, -1, &cross))) return SQ_ERROR;
        sq_poptop(v);
        sq_pushinteger(v, 2);
        if(!SQ_SUCCEEDED(sq_get(v, -2))) return SQ_ERROR;
        if(!SQ_SUCCEEDED(sq_getinteger(v, -1, &track_cross))) return SQ_ERROR;
        sq_poptop(v);
        lv_obj_set_flex_align(obj, (lv_flex_align_t)main, (lv_flex_align_t)cross,
            (lv_flex_align_t)track_cross);

        handled = true;
    }
    else if(!strcmp("pad", func_name))
    {
        SQInteger pad[4];
        for(auto i = 0u; i < 4; i++)
        {
            sq_pushinteger(v, i);
            if(!SQ_SUCCEEDED(sq_get(v, -2))) return SQ_ERROR;
            if(!SQ_SUCCEEDED(sq_getinteger(v, -1, &pad[i]))) return SQ_ERROR;
            sq_poptop(v);
        }
        lv_obj_set_style_pad_top(obj, pad[0], 0);
        lv_obj_set_style_pad_right(obj, pad[1], 0);
        lv_obj_set_style_pad_bottom(obj, pad[2], 0);
        lv_obj_set_style_pad_left(obj, pad[3], 0);
        handled = true;
    }
    else if(!strcmp("internal_pad", func_name))
    {
        SQInteger pad[2];
        for(auto i = 0u; i < 2; i++)
        {
            sq_pushinteger(v, i);
            if(!SQ_SUCCEEDED(sq_get(v, -2))) return SQ_ERROR;
            if(!SQ_SUCCEEDED(sq_getinteger(v, -1, &pad[i]))) return SQ_ERROR;
            sq_poptop(v);
        }
        lv_obj_set_style_pad_row(obj, pad[0], 0);
        lv_obj_set_style_pad_column(obj, pad[1], 0);
        handled = true;
    }
    else if(!strcmp("text", func_name))
    {
        const SQChar *val;
        if(!SQ_SUCCEEDED(sq_getstring(v, -1, &val))) return SQ_ERROR;
        //printf("text: %s\n", val);
        switch(class_id)
        {
            case ID_LABEL:
                lv_label_set_text(obj, val);
                handled = true;
                break;

            case ID_BUTTON:
                {
                    lv_obj_clean(obj);
                    auto btn_text = lv_label_create(obj);
                    lv_obj_set_style_text_font(btn_text, &lv_font_montserrat_20, 0);
                    lv_obj_set_style_text_align(btn_text, LV_TEXT_ALIGN_CENTER, 0);
                    lv_obj_add_flag(btn_text, LV_OBJ_FLAG_EVENT_BUBBLE);
                    lv_obj_set_align(btn_text, LV_ALIGN_CENTER);
                    lv_obj_add_style(btn_text, &style_text, 0);
                    lv_label_set_text(btn_text, val);
                    handled = true;
                }
                break;
        }
    }
    else if(!strcmp("img", func_name))
    {
        const SQChar *val;
        if(!SQ_SUCCEEDED(sq_getstring(v, -1, &val))) return SQ_ERROR;
        switch(class_id)
        {
            case ID_IMAGE:
                {
                    std::string sval(val);
                    auto img = get_img(val, ctx->imgs);
                    if(!img)
                    {
                        printf("could not load image %s\n", val);
                        return SQ_ERROR;
                    }
                    lv_img_set_src(obj, img);
                    handled = true;
                }
                break;
        }
    }
    else if(!strcmp("onclick", func_name))
    {
        auto ev = stack_to_event(v, &odef.sq_obj);

        if(ev.closure._type == OT_CLOSURE)
        {
            auto ev_id = ctx->events.size();
            ctx->events.push_back(ev);

            lv_obj_add_event_cb(obj, class_event_cb, LV_EVENT_CLICKED, (void *)ev_id);

            handled = true;
        }
        else
        {
            printf("not a closure\n");
            return SQ_ERROR;
        }                
    }

    if(!handled)
    {
        // cannot create new slots on class instances, need to use a table member instead
        sq_pushstring(v, "_intdata", -1);
        sq_get(v, 1);

        if(sq_gettype(v, -1) != OT_TABLE)
        {
            printf("_intdata is no longer a table\n");
            return SQ_ERROR;
        }

        sq_push(v, -3);
        sq_push(v, -3);

        auto rsret = sq_rawset(v, -3);

        if(!SQ_SUCCEEDED(rsret))
        {
            printf("rawset failed\n");
            return rsret;
        }
    }

    //sq_release(v, &odef.sq_obj);

    return 0;
}

void run_ev_closure(size_t ev_idx, sq_context *ctx)
{
    if(!ctx)
    {
        printf("run_ev_closure: no ctx\n");
        return;
    }

    assert(ev_idx < ctx->events.size());
    auto &ev = ctx->events[ev_idx];
    auto v = ev.v;
    assert(v == ctx->v);
    assert(ev.closure._type == OT_CLOSURE);

    // setup stack
    sq_settop(ev.v, 0);
    sq_pushobject(ev.v, ev.closure);

    // get its root
    sq_getclosureroot(v, 1);

    // then the param
    auto nparams = 1;

    if(ev.obj._type != OT_NULL)
    {
        sq_pushobject(ev.v, ev.obj);
        nparams++;
    }

    auto cret = sq_call(ev.v, nparams, SQFalse, SQTrue);

    if(!SQ_SUCCEEDED(cret))
    {
        printf("class_event_cb: sq_call failed\n");
        return;
    }

    sq_settop(v, 0);
}

static void class_event_cb(lv_event_t *e)
{
    auto ev_idx = (size_t)(uintptr_t)lv_event_get_user_data(e);
    auto obj = lv_event_get_target_obj(e);
    auto ctx = (sq_context *)lv_obj_get_user_data(obj);
    run_ev_closure(ev_idx, ctx);
}

SQInteger class_get(HSQUIRRELVM v)
{
    auto odef = this_to_obj(v, 1);
    if(odef.obj_class == ~0U)
    {
        printf("invalid class id\n");
        return SQ_ERROR;
    }

    // get function name
    const SQChar *func_name;
    auto gs_ret = sq_getstring(v, 2, &func_name);
    if(SQ_FAILED(gs_ret))
        return gs_ret;

    auto class_id = odef.obj_class;
    auto obj = odef.obj;
    
    //printf("get: %p %u %s: ", obj, class_id, func_name);


    if(!strcmp("x", func_name))
    {
        sq_settop(v, 0);
        sq_pushinteger(v, lv_obj_get_x(obj));
    }
    else if(!strcmp("y", func_name))
    {
        sq_settop(v, 0);
        sq_pushinteger(v, lv_obj_get_y(obj));
    }
    else if(!strcmp("w", func_name))
    {
        sq_settop(v, 0);
        sq_pushinteger(v, lv_obj_get_width(obj));
    }
    else if(!strcmp("h", func_name))
    {
        sq_settop(v, 0);
        sq_pushinteger(v, lv_obj_get_height(obj));
    }
    else
    {
        // cannot create new slots on class instances, need to use a table member instead
        sq_pushstring(v, "_intdata", -1);
        sq_get(v, 1);

        if(sq_gettype(v, -1) != OT_TABLE)
        {
            printf("_intdata is no longer a table\n");
            return SQ_ERROR;
        }

        sq_push(v, -2);

        auto rgret = sq_rawget(v, -2);

        //STACK_LEN("after sq_rawget");

        if(!SQ_SUCCEEDED(rgret))
        {
            printf("rawget failed\n");
            return rgret;
        }

        sq_remove(v, 1);
        sq_remove(v, 1);
        sq_remove(v, 1);
    }

    return 1;
}

SQInteger log_func(HSQUIRRELVM v)
{
    sq_tostring(v, 2);
    const SQChar *str;
    sq_getstring(v, 2, &str);

    printf("customosd_sq3: %s\n", str);

    return 0;
}

int add_lvgl_classes(HSQUIRRELVM v, lv_obj_t *_parent)
{
    auto ctx = get_ctx(v);
    ctx->tv_container = _parent;

    /* Add the basic classes */
    for(const auto &class_type : class_types)
    {
        // create class
        sq_pushstring(v, class_type.name.c_str(), -1);       // root, "class_type"
        sq_newclass(v, SQFalse);                        // root, "class_type", class

        // add members
        sq_pushstring(v, _SC("constructor"), -1);            // root, "class_type", class, "constructor"
        sq_newclosure(v, class_constructor, 0);         // root, "class_type", class, "constructor", fn: class_constructor
        sq_newslot(v, -3, SQFalse);                     // root, "class_type", class

        sq_pushstring(v, "_typeof", -1);
        sq_newclosure(v, class_type.typeof_func, 0);
        sq_newslot(v, -3, SQFalse);

        sq_pushstring(v, "_set", -1);
        sq_newclosure(v, class_set, 0);
        sq_newslot(v, -3, SQFalse);

        sq_pushstring(v, "_newslot", -1);
        sq_newclosure(v, class_set, 0);
        sq_newslot(v, -3, SQFalse);

        sq_pushstring(v, "_get", -1);
        sq_newclosure(v, class_get, 0);
        sq_newslot(v, -3, SQFalse);

        // add a table to hold all other member vars that may be set in .nuts
        sq_pushstring(v, "_intdata", -1);
        sq_newtable(v);
        sq_newslot(v, -3, SQFalse);

        // save class to root table
        sq_newslot(v, -3, SQFalse);                     // root
    }

    // create utility functions
    sq_pushstring(v, "log", -1);
    sq_newclosure(v, log_func, 0);
    sq_newslot(v, -3, SQFalse);

    sq_pushstring(v, "dialog", -1);
    sq_newclosure(v, dialog_func, 0);
    sq_newslot(v, -3, SQFalse);

    sq_pushstring(v, "toast", -1);
    sq_newclosure(v, toast_func, 0);
    sq_newslot(v, -3, SQFalse);

    sq_pushstring(v, "sendkeyevent", -1);
    sq_newclosure(v, sendkeyevent_func, 0);
    sq_newslot(v, -3, SQFalse);

    sq_pushstring(v, "loadosd", -1);
    sq_newclosure(v, loadosd_func, 0);
    sq_newslot(v, -3, SQFalse);

    sq_pushstring(v, "delaykill", -1);
    sq_newclosure(v, delaykill_func, 0);
    sq_newslot(v, -3, SQFalse);

    sq_pushstring(v, "close", -1);
    sq_newclosure(v, close_func, 0);
    sq_newslot(v, -3, SQFalse);

    sq_pushstring(v, "delay", -1);
    sq_newclosure(v, delay_func, 0);
    sq_newslot(v, -3, SQFalse);

    // create the lv namespace with various defines
    sq_pushstring(v, "lv", -1);
    sq_newtable(v);

    ADD_DEFINE(LV_LAYOUT_NONE);
    ADD_DEFINE(LV_LAYOUT_FLEX);
    ADD_DEFINE(LV_LAYOUT_GRID);

    ADD_DEFINE(LV_FLEX_FLOW_ROW);
    ADD_DEFINE(LV_FLEX_FLOW_COLUMN);
    ADD_DEFINE(LV_FLEX_WRAP);
    ADD_DEFINE(LV_FLEX_REVERSE);
    ADD_DEFINE(LV_FLEX_FLOW_ROW_WRAP);
    ADD_DEFINE(LV_FLEX_FLOW_ROW_REVERSE);
    ADD_DEFINE(LV_FLEX_FLOW_ROW_WRAP_REVERSE);
    ADD_DEFINE(LV_FLEX_FLOW_COLUMN_WRAP);
    ADD_DEFINE(LV_FLEX_FLOW_COLUMN_REVERSE);
    ADD_DEFINE(LV_FLEX_FLOW_COLUMN_WRAP_REVERSE);

    ADD_DEFINE(LV_ALIGN_DEFAULT);
    ADD_DEFINE(LV_ALIGN_TOP_LEFT);
    ADD_DEFINE(LV_ALIGN_TOP_MID);
    ADD_DEFINE(LV_ALIGN_TOP_RIGHT);
    ADD_DEFINE(LV_ALIGN_BOTTOM_LEFT);
    ADD_DEFINE(LV_ALIGN_BOTTOM_MID);
    ADD_DEFINE(LV_ALIGN_BOTTOM_RIGHT);
    ADD_DEFINE(LV_ALIGN_LEFT_MID);
    ADD_DEFINE(LV_ALIGN_RIGHT_MID);
    ADD_DEFINE(LV_ALIGN_CENTER);

    ADD_DEFINE(LV_ALIGN_OUT_TOP_LEFT);
    ADD_DEFINE(LV_ALIGN_OUT_TOP_MID);
    ADD_DEFINE(LV_ALIGN_OUT_TOP_RIGHT);
    ADD_DEFINE(LV_ALIGN_OUT_BOTTOM_LEFT);
    ADD_DEFINE(LV_ALIGN_OUT_BOTTOM_MID);
    ADD_DEFINE(LV_ALIGN_OUT_BOTTOM_RIGHT);
    ADD_DEFINE(LV_ALIGN_OUT_LEFT_TOP);
    ADD_DEFINE(LV_ALIGN_OUT_LEFT_MID);
    ADD_DEFINE(LV_ALIGN_OUT_LEFT_BOTTOM);
    ADD_DEFINE(LV_ALIGN_OUT_RIGHT_TOP);
    ADD_DEFINE(LV_ALIGN_OUT_RIGHT_MID);
    ADD_DEFINE(LV_ALIGN_OUT_RIGHT_BOTTOM);

    ADD_DEFINE(LV_FLEX_ALIGN_START);
    ADD_DEFINE(LV_FLEX_ALIGN_END);
    ADD_DEFINE(LV_FLEX_ALIGN_CENTER);
    ADD_DEFINE(LV_FLEX_ALIGN_SPACE_EVENLY);
    ADD_DEFINE(LV_FLEX_ALIGN_SPACE_AROUND);
    ADD_DEFINE(LV_FLEX_ALIGN_SPACE_BETWEEN);

    sq_newslot(v, -3, SQFalse);

    // scancodes in root table
    
    ADD_DEFINE(GK_SCANCODE_0);
    ADD_DEFINE(GK_SCANCODE_1);
    ADD_DEFINE(GK_SCANCODE_2);
    ADD_DEFINE(GK_SCANCODE_3);
    ADD_DEFINE(GK_SCANCODE_4);
    ADD_DEFINE(GK_SCANCODE_5);
    ADD_DEFINE(GK_SCANCODE_6);
    ADD_DEFINE(GK_SCANCODE_7);
    ADD_DEFINE(GK_SCANCODE_8);
    ADD_DEFINE(GK_SCANCODE_9);

    ADD_DEFINE(GK_SCANCODE_A);
    ADD_DEFINE(GK_SCANCODE_B);
    ADD_DEFINE(GK_SCANCODE_C);
    ADD_DEFINE(GK_SCANCODE_D);
    ADD_DEFINE(GK_SCANCODE_E);
    ADD_DEFINE(GK_SCANCODE_F);
    ADD_DEFINE(GK_SCANCODE_G);
    ADD_DEFINE(GK_SCANCODE_H);
    ADD_DEFINE(GK_SCANCODE_I);
    ADD_DEFINE(GK_SCANCODE_J);
    ADD_DEFINE(GK_SCANCODE_K);
    ADD_DEFINE(GK_SCANCODE_L);
    ADD_DEFINE(GK_SCANCODE_M);
    ADD_DEFINE(GK_SCANCODE_N);
    ADD_DEFINE(GK_SCANCODE_O);
    ADD_DEFINE(GK_SCANCODE_P);
    ADD_DEFINE(GK_SCANCODE_Q);
    ADD_DEFINE(GK_SCANCODE_R);
    ADD_DEFINE(GK_SCANCODE_S);
    ADD_DEFINE(GK_SCANCODE_T);
    ADD_DEFINE(GK_SCANCODE_U);
    ADD_DEFINE(GK_SCANCODE_V);
    ADD_DEFINE(GK_SCANCODE_W);
    ADD_DEFINE(GK_SCANCODE_X);
    ADD_DEFINE(GK_SCANCODE_Y);
    ADD_DEFINE(GK_SCANCODE_Z);

    ADD_DEFINE(GK_SCANCODE_RETURN);
    ADD_DEFINE(GK_SCANCODE_RETURN2);
    ADD_DEFINE(GK_SCANCODE_TAB);
    ADD_DEFINE(GK_SCANCODE_ESCAPE);
    ADD_DEFINE(GK_SCANCODE_BACKSPACE);
    ADD_DEFINE(GK_SCANCODE_SPACE);
    ADD_DEFINE(GK_SCANCODE_MINUS);
    ADD_DEFINE(GK_SCANCODE_EQUALS);
    ADD_DEFINE(GK_SCANCODE_LEFTBRACKET);
    ADD_DEFINE(GK_SCANCODE_RIGHTBRACKET);
    ADD_DEFINE(GK_SCANCODE_BACKSLASH);
    ADD_DEFINE(GK_SCANCODE_SEMICOLON);
    ADD_DEFINE(GK_SCANCODE_APOSTROPHE);
    ADD_DEFINE(GK_SCANCODE_GRAVE);
    ADD_DEFINE(GK_SCANCODE_COMMA);
    ADD_DEFINE(GK_SCANCODE_PERIOD);
    ADD_DEFINE(GK_SCANCODE_SLASH);
    ADD_DEFINE(GK_SCANCODE_LCTRL);
    ADD_DEFINE(GK_SCANCODE_RCTRL);
    ADD_DEFINE(GK_SCANCODE_LALT);
    ADD_DEFINE(GK_SCANCODE_RALT);
    ADD_DEFINE(GK_SCANCODE_LSHIFT);
    ADD_DEFINE(GK_SCANCODE_RSHIFT);
    ADD_DEFINE(GK_SCANCODE_CAPSLOCK);

    ADD_DEFINE(GK_SCANCODE_F1);
    ADD_DEFINE(GK_SCANCODE_F2);
    ADD_DEFINE(GK_SCANCODE_F3);
    ADD_DEFINE(GK_SCANCODE_F4);
    ADD_DEFINE(GK_SCANCODE_F5);
    ADD_DEFINE(GK_SCANCODE_F6);
    ADD_DEFINE(GK_SCANCODE_F7);
    ADD_DEFINE(GK_SCANCODE_F8);
    ADD_DEFINE(GK_SCANCODE_F9);
    ADD_DEFINE(GK_SCANCODE_F10);
    ADD_DEFINE(GK_SCANCODE_F11);
    ADD_DEFINE(GK_SCANCODE_F12);

    ADD_DEFINE(GK_SCANCODE_PRINTSCREEN);
    ADD_DEFINE(GK_SCANCODE_SCROLLLOCK);
    ADD_DEFINE(GK_SCANCODE_PAUSE);
    ADD_DEFINE(GK_SCANCODE_INSERT);
    ADD_DEFINE(GK_SCANCODE_HOME);
    ADD_DEFINE(GK_SCANCODE_PAGEUP);
    ADD_DEFINE(GK_SCANCODE_PAGEDOWN);
    ADD_DEFINE(GK_SCANCODE_DELETE);
    ADD_DEFINE(GK_SCANCODE_END);
    ADD_DEFINE(GK_SCANCODE_LEFT);
    ADD_DEFINE(GK_SCANCODE_RIGHT);
    ADD_DEFINE(GK_SCANCODE_UP);
    ADD_DEFINE(GK_SCANCODE_DOWN);
    ADD_DEFINE(GK_SCANCODE_NUMLOCKCLEAR);

    ADD_DEFINE(GK_SCANCODE_KP_DIVIDE);
    ADD_DEFINE(GK_SCANCODE_KP_MULTIPLY);
    ADD_DEFINE(GK_SCANCODE_KP_MINUS);
    ADD_DEFINE(GK_SCANCODE_KP_PLUS);
    ADD_DEFINE(GK_SCANCODE_KP_ENTER);
    ADD_DEFINE(GK_SCANCODE_KP_1);
    ADD_DEFINE(GK_SCANCODE_KP_2);
    ADD_DEFINE(GK_SCANCODE_KP_3);
    ADD_DEFINE(GK_SCANCODE_KP_4);
    ADD_DEFINE(GK_SCANCODE_KP_5);
    ADD_DEFINE(GK_SCANCODE_KP_6);
    ADD_DEFINE(GK_SCANCODE_KP_7);
    ADD_DEFINE(GK_SCANCODE_KP_8);
    ADD_DEFINE(GK_SCANCODE_KP_9);
    ADD_DEFINE(GK_SCANCODE_KP_0);
    ADD_DEFINE(GK_SCANCODE_KP_PERIOD);
    ADD_DEFINE(GK_SCANCODE_KP_EQUALS);

    ADD_DEFINE(GK_SCANCODE_AUDIONEXT);
    ADD_DEFINE(GK_SCANCODE_AUDIOPREV);
    ADD_DEFINE(GK_SCANCODE_AUDIOPLAY);
    ADD_DEFINE(GK_SCANCODE_AUDIOSTOP);

    ADD_DEFINE(GK_MODIFIER_ALT);
    ADD_DEFINE(GK_MODIFIER_CTRL);
    ADD_DEFINE(GK_MODIFIER_SHIFT);

    return 0;
}

static void dialog_cb(void *userptr, void *ctx)
{
    auto ev_id = (size_t)(uintptr_t)userptr;
    run_ev_closure(ev_id, (sq_context *)ctx);
}

SQInteger toast_func(HSQUIRRELVM v)
{
    // toast(string text, integer ms = 500)
    if(sq_gettop(v) < 2)
    {
        printf("toast_func: incorrect nparams: %lld\n", sq_gettop(v));
        return SQ_ERROR;
    }

    SQInteger ms_delay;
    if(SQ_SUCCEEDED(sq_getinteger(v, -1, &ms_delay)))
    {
        sq_poptop(v);
    }
    else
    {
        ms_delay = 500;
    }

    const SQChar *msg;
    if(!SQ_SUCCEEDED(sq_getstring(v, -1, &msg)))
    {
        printf("toast_func: no msg passed\n");
        return SQ_ERROR;
    }

    std::string smsg(msg);
    toast_message(smsg, std::chrono::milliseconds(ms_delay));
    return 0;
}

SQInteger dialog_func(HSQUIRRELVM v)
{
    auto ctx = get_ctx(v);

    // dialog(string text, array)
    if(sq_gettop(v) < 3)
    {
        printf("dialog_func: incorrect nparams: %lld\n", sq_gettop(v));
        return SQ_ERROR;
    }
    if(sq_gettype(v, -2) != OT_STRING)
    {
        printf("dialog_func: param 1 not string\n");
        return SQ_ERROR;
    }
    if(sq_gettype(v, -1) != OT_ARRAY)
    {
        printf("dialog_func: param 2 not array\n");
        return SQ_ERROR;
    }

    const SQChar *txt;
    sq_getstring(v, -2, &txt);
    std::string stxt(txt);

    // iterate array
    std::vector<dialogbox_button> btns;

    sq_pushnull(v);
    while(SQ_SUCCEEDED(sq_next(v, -2)))
    {
        if(sq_gettype(v, -1) != OT_TABLE)
        {
            printf("dialog_func: btn member not a table\n");
            return SQ_ERROR;
        }

        dialogbox_button cbtn;

        sq_pushstring(v, "text", -1);
        std::string sbtn_text;
        if(SQ_SUCCEEDED(sq_get(v, -2)))
        {
            const SQChar *btn_text;
            if(sq_gettype(v, -1) != OT_STRING)
            {
                printf("dialog_func: btn.text is not a string\n");
                return SQ_ERROR;
            }
            sq_getstring(v, -1, &btn_text);
            sbtn_text = std::string(btn_text);

            sq_pop(v, 1);
        }
        cbtn.text = sbtn_text;

        sq_pushstring(v, "onclick", -1);
        if(!SQ_SUCCEEDED(sq_get(v, -2)))
        {
            // do nothing
            cbtn.func = nullptr;
        }
        else
        {
            // ensure it is a closure
            if(!sq_gettype(v, -1) == OT_CLOSURE)
            {
                printf("dialog_func: btn.onclick not a closure\n");
                return SQ_ERROR;
            }

            HSQOBJECT o_closure;
            HSQOBJECT o_obj;
            if(!SQ_SUCCEEDED(sq_getstackobj(v, -1, &o_closure)))
            {
                printf("dialog_func: getstackobj(closure) failed\n");
                return SQ_ERROR;
            }
            if(!SQ_SUCCEEDED(sq_getstackobj(v, 1, &o_obj)))
            {
                printf("dialog_func: getstackobj(obj) failed\n");
                return SQ_ERROR;
            }
            sq_addref(v, &o_closure);
            sq_addref(v, &o_obj);
            sq_pop(v, 1);

            sq_event ev;
            ev.closure = o_closure;
            ev.obj = o_obj;
            ev.v = v;
            auto ev_id = ctx->events.size();
            ctx->events.push_back(ev);

            cbtn.func = dialog_cb;
            cbtn.userptr = (void *)ev_id;
            cbtn.ctxptr = ctx;
        }

        btns.push_back(cbtn);

        sq_pop(v, 2);
    }
    sq_pop(v, 1);

    dialogbox_show(stxt, btns);
    return 0;
}

SQInteger sendkeyevent_func(HSQUIRRELVM v)
{
    if(sq_gettop(v) < 3)
    {
        printf("sendkeyevent_func: incorrect nparams: %lld\n", sq_gettop(v));
        return SQ_ERROR;
    }
    SQInteger key, pressed;
    if(!SQ_SUCCEEDED(sq_getinteger(v, -2, &key)))
    {
        printf("sendkeyevent_func: param 1 not integer\n");
        return SQ_ERROR;
    }
    if(!SQ_SUCCEEDED(sq_getinteger(v, -1, &pressed)))
    {
        printf("sendkeyevent_func: param 2 not integer\n");
        return SQ_ERROR;
    }

    printf("key_event: key %lld %s\n", key, (pressed == 0) ? "released" : "pressed");

#ifndef SUPERVISOR_SIMULATOR
    auto fpid = GK_GetFocusProcess();

    Event ev_press;
    ev_press.type = (pressed == 0) ? Event::event_type_t::KeyUp : Event::event_type_t::KeyDown;
    ev_press.key = key;
    GK_EventSend(fpid, &ev_press);
#endif

    return 0;
}

void loadosd(const std::string &fname);

static void loadosd_cb(void *p)
{
    auto sosd_file = reinterpret_cast<std::string *>(p);
    loadosd(*sosd_file);
    delete sosd_file;
}

SQInteger loadosd_func(HSQUIRRELVM v)
{
    if(sq_gettop(v) < 2)
    {
        printf("loadosd_func: incorrect nparams: %lld\n", sq_gettop(v));
        return SQ_ERROR;
    }
    const SQChar *osd_file;
    if(!SQ_SUCCEEDED(sq_getstring(v, -1, &osd_file)))
    {
        printf("loadosd_func: param 1 not string\n");
        return SQ_ERROR;
    }
    auto sosd_file = new std::string(osd_file);

    lv_async_call(loadosd_cb, sosd_file);

    return 0;
}

#ifndef SUPERVISOR_SIMULATOR
static void delaykill_delayed_cb(lv_timer_t *t)
{
    auto fpid = (pid_t)(intptr_t)lv_timer_get_user_data(t);
    kill(fpid, SIGKILL);
}
#endif

SQInteger delaykill_func(HSQUIRRELVM v)
{
#ifdef SUPERVISOR_SIMULATOR
    printf("delaykill\n");
#else
    // delaykill(ms = 1000);
    SQInteger delaykill_param;

    // schedule a callback for 1 s time to actually kill the process if not already by other keystrokes
    auto fpid = GK_GetFocusProcess();
    auto t = lv_timer_create(delaykill_delayed_cb,
        SQ_SUCCEEDED(sq_getinteger(v, -1, &delaykill_param)) ? (uint32_t)delaykill_param : 1000,
        (void *)(intptr_t)fpid);
    lv_timer_set_repeat_count(t, 1);
#endif

    return 0;
}

SQInteger close_func(HSQUIRRELVM v)
{
#ifdef SUPERVISOR_SIMULATOR
    printf("close\n");
#else
    close_supervisor();
#endif
    return 0;
}

SQInteger delay_func(HSQUIRRELVM v)
{
    // delay(ms = 1);
    SQInteger delay_param;
    auto delay_ms = SQ_SUCCEEDED(sq_getinteger(v, -1, &delay_param)) ? (int)delay_param : 1;
    usleep(delay_ms * 1000);
    return 0;
}

/* Generic stuff for setting up a vm follows */
void dump_item(HSQUIRRELVM v, int i, bool quote_strings)
{
    auto type = sq_gettype(v, i);

    if(i < 0)
        i = sq_gettop(v) + i + 1;

    switch(type)
    {
        case OT_STRING:
            {
                const SQChar *str;
                sq_getstring(v, i, &str);
                if(quote_strings)
                    printf("\"%s\"", str);
                else
                    printf("%s", str);
            }
            break;

        case OT_INTEGER:
            {
                SQInteger val;
                sq_getinteger(v, i, &val);
                printf("%lld", val);
            }
            break;

        case OT_CLOSURE:
            {
#if SQUIRREL_VERSION_NUMBER >= 320
                SQInteger nparams, nfree;
#else
                SQUnsignedInteger nparams, nfree;
#endif
                sq_getclosureinfo(v, i, &nparams, &nfree);
                sq_getclosurename(v, i);
                printf("closure: ");
                dump_item(v, -1, false);
                sq_pop(v, 1);
                printf("(%lld)", nparams);

                /*sq_getclosureroot(v, i);
                dump_item(-1);
                sq_pop(v, 1); */        
            }
            break;

        case OT_TABLE:
        case OT_INSTANCE:
            {
                if(type == OT_INSTANCE) printf("object ");
                printf("{ ");
                sq_pushnull(v);
                while(SQ_SUCCEEDED(sq_next(v, i)))
                {
                    const SQChar *key;
                    dump_item(v, -2, false);
                    printf("=");
                    dump_item(v, -1);
                    printf(" ");
                    sq_pop(v, 2);
                }
                sq_pop(v, 1);
                printf("}");
            }
            break;

        case OT_NULL:
            printf("NULL");
            break;

        case OT_NATIVECLOSURE:
            printf("native()");
            break;

        case OT_CLASS:
            printf("class");
            break;

        case OT_BOOL:
            {
                SQBool val;
                sq_getbool(v, i, &val);
                if(val) printf("true"); else printf("false");
            }
            break;

        case OT_ARRAY:
            {
                printf("[ ");
                sq_pushnull(v);
                while(SQ_SUCCEEDED(sq_next(v, i)))
                {
                    const SQChar *key;
                    dump_item(v, -1);
                    printf(", ");
                    sq_pop(v, 2);
                }
                sq_pop(v, 1);
                printf("]");
            }
            break;


        default:
            printf("<UNKNOWN TYPE %d>", type);
            break;
    }
}

void dump_stack(HSQUIRRELVM v, bool quote_strings)
{
    for(auto i = 1; i <= sq_gettop(v); i++)
    {
        printf("%i: ", i);
        dump_item(v, i, quote_strings);
        printf("\n");
    }
}
static SQInteger error_handler(HSQUIRRELVM vm)
{
    printf("error raised: ");
    dump_item(vm, -1, false);
    printf("\n");

    dump_stack(vm);

    return 0;
}

static void comp_error(HSQUIRRELVM vm, const SQChar *desc, const SQChar *src, SQInteger line, SQInteger col)
{
    printf("compile error %s\n", desc);
}

static void printfunc(HSQUIRRELVM vm, const SQChar *msg, ...)
{
    va_list ap;
    va_start(ap, msg);
    vprintf(msg, ap);
    va_end(ap);
}

static void errfunc(HSQUIRRELVM vm, const SQChar *msg, ...)
{
    printf("ERROR: ");
    va_list ap;
    va_start(ap, msg);
    vprintf(msg, ap);
    va_end(ap);
    printf("\n");
}

static SQInteger file_lexfeedASCII(SQUserPointer file)
{
    int ret;
    char c;
    if( ( ret=fread(&c,sizeof(c),1,(FILE *)file )>0) )
        return c;
    return 0;
}

static int compile_file(HSQUIRRELVM v,const char *filename)
{
    FILE *f=fopen(filename,"rb");
    if(f)
    {
        auto compile_ret = sq_compile(v,file_lexfeedASCII,f,filename,1);
#ifndef DEBUG
        if(compile_ret != 0)
#endif
            printf("compile: %s: %lld\n", filename, compile_ret);
        fclose(f);
        return 1;
    }
    return 0;
}

std::unique_ptr<osd> customosd_sq_create(const std::string &fname, lv_obj_t *hidden_tv)
{
    /* Set up VM */
    HSQUIRRELVM v;
    v = sq_open(1024);

    auto ctx = std::make_unique<sq_context>();
    ctx->v = v;
    sq_setforeignptr(v, ctx.get());

    sq_newclosure(v, error_handler, 0);
    sq_seterrorhandler(v);
    
    sq_setprintfunc(v, printfunc, errfunc);

    sq_setcompilererrorhandler(v, comp_error);
    //sq_seterrorhandler(v);
    sq_pushroottable(v);
    sqstd_register_iolib(v);
    sq_pop(v, 1);

    // set global defines
    sq_pushroottable(v);
    add_lvgl_classes(v, hidden_tv);
    sq_pop(v, 1);

    // load gk_sq_lib.nut library
    if(compile_file(v, "osd/gk_sq_lib.nut") != 1)
    {
        printf("customosq_sq_create: compile_file(osd/gk_sq_lib.nut) failed\n");
        return nullptr;
    }

    sq_pushroottable(v);
    if(!SQ_SUCCEEDED(sq_call(v, 1, false, true)))
    {
        printf("customosq_sq_create: sq_call(gk_sq_lib.nut) failed\n");
        return nullptr;
    }
    sq_pop(v, 1);

    // Load and run script
    if(compile_file(v, fname.c_str()) != 1)
    {
        printf("customosq_sq_create: compile_file failed\n");
        return nullptr;
    }

    sq_pushroottable(v);
    if(!SQ_SUCCEEDED(sq_call(v, 1, false, true)))
    {
        printf("customosq_sq_create: sq_call failed\n");
        return nullptr;
    }
    sq_pop(v, 1);

    // Did the script set any pause or unpause actions?
    sq_pushroottable(v);
    sq_pushstring(v, "pause", -1);
    if(SQ_SUCCEEDED(sq_get(v, -2)))
    {
        auto ev = stack_to_event(v, nullptr);

        if(ev.closure._type == OT_CLOSURE)
        {
            auto ev_id = ctx->events.size();
            ctx->events.push_back(ev);
            ctx->ev_pause = ev_id;
        }
    }
    sq_settop(v, 0);

    sq_pushroottable(v);
    sq_pushstring(v, "unpause", -1);
    if(SQ_SUCCEEDED(sq_get(v, -2)))
    {
        auto ev = stack_to_event(v, nullptr);

        if(ev.closure._type == OT_CLOSURE)
        {
            auto ev_id = ctx->events.size();
            ctx->events.push_back(ev);
            ctx->ev_resume = ev_id;
        }
    }
    sq_settop(v, 0);

    if(ctx->ev_resume == -1 && ctx->ev_pause != -1)
        ctx->ev_resume = ctx->ev_pause;    

    return ctx;
}

sq_context::~sq_context()
{
    printf("sq_context destructor\n");
    if(v)
    {
        for(auto ev : events)
        {
            sq_release(v, &ev.closure);

            if(ev.obj._type != OT_NULL)
                sq_release(v, &ev.obj);
        }

        for(auto [ inst, _ ] : obj_map)
        {
            
            HSQOBJECT obj;
            obj._type = OT_INSTANCE;
            obj._unVal.pInstance = inst;
            sq_release(v, &obj);
        }

        sq_collectgarbage(v);
        sq_close(v);
        v = nullptr;
    }
}

int sq_context::pause()
{
    if(ev_pause != -1)
    {
        run_ev_closure(ev_pause, this);
    }
    return 0;
}

int sq_context::resume()
{
    if(ev_resume != -1)
    {
        run_ev_closure(ev_resume, this);
    }
    return 0;
}
