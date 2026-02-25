#ifndef LV_GK_INPUT_H
#define LV_GK_INPUT_H

lv_indev_t * lv_gk_mouse_create(void);
lv_indev_t * lv_gk_touchscreen_create(void);
lv_indev_t * lv_gk_kbd_create(void);

void lv_gk_register_inputs();

#endif
