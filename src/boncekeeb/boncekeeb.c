/**
 * Copyright (c) 2020 Raspberry Pi (Trading) Ltd.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <stdio.h>
#include <stdlib.h>

#include "pico/stdlib.h"
#include "hardware/pio.h"
#include "hardware/clocks.h"
#include "ws2812.pio.h"

static inline void put_pixel(uint32_t pixel_grb) {
    pio_sm_put_blocking(pio0, 0, pixel_grb << 8u);
}

static inline uint32_t urgb_u32(uint8_t r, uint8_t g, uint8_t b) {
    return
            ((uint32_t) (r) << 8) |
            ((uint32_t) (g) << 16) |
            (uint32_t) (b);
}

void pattern_single(uint len, uint t) {
    int pixel_array [20] = { 5, 10, 15, 20, 19,
                            14,  9,  4,  3,  8,
                            13, 18, 17, 12,  7,
                             2,  1,  6, 11, 16 };
    for (int i = 0; i < len; ++i) {
        if (pixel_array[i] >= 0 && pixel_array[i] <=5) {
            put_pixel(urgb_u32(0xff, 0xff, 0xff));
        }
        else if (pixel_array[i] > 5 && pixel_array[i] <=10) {
            put_pixel(urgb_u32(0xff, 0, 0));
        }
        else if (pixel_array[i] > 10 && pixel_array[i] <=15) {
            put_pixel(urgb_u32(0, 0xff, 0));
        }
        else if (pixel_array[i] > 15 && pixel_array[i] <=20) {
            put_pixel(urgb_u32(0, 0, 0xff));
        }
    }
}


typedef void (*pattern)(uint len, uint t);
const struct {
    pattern pat;
    const char *name;
} pattern_table[] = {
        {pattern_single, "Single"},
};

const int PIN_TX = 0;

int main() {
    //set_sys_clock_48();
    stdio_init_all();
    puts("WS2812 Smoke Test");

    // todo get free sm
    PIO pio = pio0;
    int sm = 0;
    uint offset = pio_add_program(pio, &ws2812_program);

    ws2812_program_init(pio, sm, offset, PIN_TX, 800000, false);

    int t = 0;
    while (1) {
        int pat = rand() % count_of(pattern_table);
        int dir = (rand() >> 30) & 1 ? 1 : -1;
        puts(pattern_table[pat].name);
        puts(dir == 1 ? "(forward)" : "(backward)");
        for (int i = 0; i < 1000; ++i) {
            pattern_table[pat].pat(20, 255);
            sleep_ms(10);
            t += dir;
        }
    }
}