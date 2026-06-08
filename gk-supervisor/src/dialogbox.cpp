#include "dialogbox.h"
#include "styles.h"

static bool has_dialog = false;

static lv_obj_t *dialog_parent, *dialog_background, *dialog_text, *dialog_btn_container;
static lv_group_t *dialog_grp;
extern lv_group_t *grp;

struct dialog_btn_impl
{
    dialogbox_button b;
    lv_obj_t *btn, *btn_text;
};

static std::vector<dialog_btn_impl> cdialog;

#ifndef SUPERVISOR_SIMULATOR
void update_kernel_state(bool show);
extern bool last_show;
#endif

static void dialog_cb(lv_event_t *e);
static void dialog_destroy();

void init_dialogbox()
{
    dialog_grp = lv_group_create();

    dialog_parent = lv_obj_create(lv_scr_act());
    lv_obj_set_pos(dialog_parent, 0, 0);
    lv_obj_set_size(dialog_parent, 800, 480);
    lv_obj_add_style(dialog_parent, &style_transp, 0);

    dialog_background = lv_obj_create(dialog_parent);
    lv_obj_set_size(dialog_background, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
    lv_obj_align(dialog_background, LV_ALIGN_TOP_MID, 0, (232 - 40) / 2);    // align to space between osbar and omain
    lv_obj_add_style(dialog_background, &style_cont, 0);
    lv_obj_set_layout(dialog_background, LV_LAYOUT_FLEX);
    lv_obj_set_flex_flow(dialog_background, LV_FLEX_FLOW_COLUMN);
    lv_obj_set_flex_align(dialog_background,
        LV_FLEX_ALIGN_START,
        LV_FLEX_ALIGN_CENTER,
        LV_FLEX_ALIGN_CENTER);
    lv_obj_clear_flag(dialog_background, LV_OBJ_FLAG_SCROLLABLE);

    dialog_text = lv_label_create(dialog_background);
    lv_obj_set_style_text_font(dialog_text, &lv_font_montserrat_20, 0);
    lv_obj_set_size(dialog_text, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
    lv_obj_set_style_text_align(dialog_text, LV_TEXT_ALIGN_CENTER, 0);
    lv_label_set_long_mode(dialog_text, LV_LABEL_LONG_MODE_WRAP);
    lv_obj_add_style(dialog_text, &style_text, 0);
    lv_label_set_text(dialog_text, "dialog box\nline 2");

    dialog_btn_container = lv_obj_create(dialog_background);
    lv_obj_set_size(dialog_btn_container, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
    lv_obj_add_style(dialog_btn_container, &style_transp, 0);
    lv_obj_set_layout(dialog_btn_container, LV_LAYOUT_FLEX);
    lv_obj_set_flex_flow(dialog_btn_container, LV_FLEX_FLOW_ROW);
    lv_obj_set_flex_align(dialog_btn_container,
        LV_FLEX_ALIGN_CENTER,
        LV_FLEX_ALIGN_CENTER,
        LV_FLEX_ALIGN_CENTER);
    lv_obj_align(dialog_btn_container, LV_ALIGN_BOTTOM_MID, 0, 0);

    // hide until needed
    lv_obj_add_flag(dialog_parent, LV_OBJ_FLAG_HIDDEN);

    lv_obj_update_layout(dialog_background);
    lv_obj_set_width(dialog_background, lv_obj_get_content_width(dialog_background));
}

int dialogbox_show(const std::string &msg, const std::vector<dialogbox_button> btns)
{
    if(has_dialog)
    {
        fprintf(stderr, "cannot show dialog box - already shown\n");
        return -1;
    }

    has_dialog = true;

    /* Set title text */
    lv_label_set_text(dialog_text, msg.c_str());

    /* Create the buttons */
    for(const auto &ibtn : btns)
    {
        dialog_btn_impl cbtn;
        cbtn.b = ibtn;

        cbtn.btn = lv_button_create(dialog_btn_container);
        lv_obj_add_style(cbtn.btn, &style_button, 0);
        lv_obj_add_style(cbtn.btn, &style_btn_focus, LV_STATE_FOCUS_KEY);

        cbtn.btn_text = lv_label_create(cbtn.btn);
        lv_obj_set_style_text_font(cbtn.btn_text, &lv_font_montserrat_20, 0);
        lv_obj_set_style_text_align(cbtn.btn_text, LV_TEXT_ALIGN_CENTER, 0);
        lv_obj_add_flag(cbtn.btn_text, LV_OBJ_FLAG_EVENT_BUBBLE);
        lv_obj_set_align(cbtn.btn_text, LV_ALIGN_CENTER);
        lv_obj_add_style(cbtn.btn_text, &style_text, 0);
        lv_label_set_text(cbtn.btn_text, cbtn.b.text.c_str());

        lv_group_add_obj(dialog_grp, cbtn.btn);

        lv_obj_add_event_cb(cbtn.btn, dialog_cb, LV_EVENT_CLICKED, (void *)cdialog.size());
        cdialog.push_back(cbtn);
    }

    //lv_indev_set_group(kbd, dialog_grp);

    lv_obj_update_layout(dialog_btn_container);
    lv_obj_update_layout(dialog_text);
    lv_obj_set_width(dialog_background, std::max(lv_obj_get_width(dialog_btn_container),
        lv_obj_get_width(dialog_text)));
    lv_obj_remove_flag(dialog_parent, LV_OBJ_FLAG_HIDDEN);

#ifndef SUPERVISOR_SIMULATOR
    update_kernel_state(last_show);
#endif

    return 0;
}   

lv_obj_t *dialogbox_get()
{
    lv_obj_update_layout(dialog_background);
    return dialog_background;
}

bool dialog_visible()
{
    return has_dialog;
}

void dialog_cb(lv_event_t *e)
{
    auto ev_idx = (size_t)(uintptr_t)lv_event_get_user_data(e);
    auto ev = cdialog[ev_idx];

    dialog_destroy();

    if(ev.b.func)
    {
        ev.b.func(ev.b.userptr, ev.b.ctxptr);
    }
}

void dialog_destroy()
{
    //lv_indev_set_group(kbd, grp);

    lv_obj_add_flag(dialog_parent, LV_OBJ_FLAG_HIDDEN);

    for(const auto &cbtn : cdialog)
    {
        lv_obj_delete(cbtn.btn_text);
        lv_obj_delete(cbtn.btn);
    }

    lv_group_remove_all_objs(dialog_grp);

    cdialog.clear();

    has_dialog = false;
#ifndef SUPERVISOR_SIMULATOR
    update_kernel_state(last_show);
#endif
}
