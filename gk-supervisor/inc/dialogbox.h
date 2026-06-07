#ifndef DIALOGBOX_H
#define DIALOGBOX_H

#include <lvgl.h>
#include <vector>
#include <string>

struct dialogbox_button
{
    std::string text;
    void (*func)(void *, void *);
    void *userptr;
    void *ctxptr;
};

void init_dialogbox();

int dialogbox_show(const std::string &msg, const std::vector<dialogbox_button> btns);
bool dialog_visible();

#endif
