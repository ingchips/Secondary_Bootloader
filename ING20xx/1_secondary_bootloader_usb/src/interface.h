#ifndef __INTERFACE_H_
#define __INTERFACE_H_

#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include "ingsoc.h"

#define DEBUG_PRINTF 0

#if (DEBUG_PRINTF == 0)
    #define debug_printf(...)
#else
    #define debug_printf(...) printf(__VA_ARGS__)
#endif

#define IRQn_ITMER1      31
#define IRQn_USB         20

typedef void (*app_timer_callback)(void);

void app_timer_init(void);
uint32_t app_timer_start(app_timer_callback timer_callback, uint16_t ms);
void app_timer_stop(void);
void app_quite_loop(void);
uint32_t app_loop(void);

void wdt_reset(void);

#endif


