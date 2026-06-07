#ifndef TOASTS_H
#define TOASTS_H

#include <string>
#include <chrono>
#include <cstdint>

int init_toasts();
int toast_message(const std::string &msg, const std::chrono::milliseconds msg_time = std::chrono::milliseconds(500));
bool toast_visible();

#endif
