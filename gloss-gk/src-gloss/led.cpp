#include "syscalls.h"
#include "deferred.h"
#include "gk.h"

int GK_SetLED(int led_id, uint32_t color)
{
    __syscall_set_led_params p { .led_id = led_id, .color = color };
    return deferred_call(__syscall_set_leds, &p);
}
