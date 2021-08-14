/**
 * Copyright (c) 2020 Raspberry Pi (Trading) Ltd.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <stdio.h>
#include <stdlib.h>

#include "pico/stdlib.h"
#include "hardware/uart.h"
#include "hardware/pio.h"
#include "hardware/gpio.h"
#include "hardware/clocks.h"
#include "generated/ws2812.pio.h"
#include "hardware/i2c.h"

#include "bsp/board.h"
#include "tusb.h"
#include "tusb_config.h"
#include "usb_descriptors.h"
#include "bonce_ssd1306.h"

#define total_key_count 20

#define key_col_count 5
#define key_row_count 4

#define key_col0 13
#define key_col1 12
#define key_col2 11
#define key_col3 10
#define key_col4 9

#define key_row0 22
#define key_row1 26
#define key_row2 27
#define key_row3 28

#define key_delay_ms 100
#define PIN_TX 6

int key_cols[5] = { key_col0, key_col1, key_col2, key_col3, key_col4 };
int key_rows[4] = { key_row0, key_row1, key_row2, key_row3 };

int key_xy[key_row_count][key_col_count] = {
    { 0, 1, 2, 3, 4}, 
    { 5, 6, 7, 8, 9}, 
    {10,11,12,13,14}, 
    {15,16,17,18,19}
};

typedef struct{
    int key_index;
    int led_index;
    int rgb[3];
    int state;
    uint32_t delay_timeout;
    bool issued;
    char name[4];
    uint8_t keycode;
} Keeb_Key;

// HID Key definitions taken from tinyusb::hid.h
Keeb_Key keeb_keys[total_key_count] = {
    {0, 16, {0xff, 0x00, 0x00}, 0, 0, false, "ESC", HID_KEY_ESCAPE},
    {1, 15, {0xff, 0x00, 0x00}, 0, 0, false, "F10", HID_KEY_F10},
    {2,  8, {0xff, 0x00, 0x00}, 0, 0, false, "F9",  HID_KEY_F9},
    {3,  7, {0xff, 0x00, 0x00}, 0, 0, false, "F5", HID_KEY_F5},
    {4,  0, {0xff, 0x00, 0x00}, 0, 0, false, "F6",  HID_KEY_F6},

    {5, 17, {0x00, 0xff, 0x00}, 0, 0, false, "[",   HID_KEY_BRACKET_LEFT},
    {6, 14, {0x00, 0xff, 0x00}, 0, 0, false, "]",   HID_KEY_BRACKET_RIGHT},
    {7,  9, {0x00, 0xff, 0x00}, 0, 0, false, "T",   HID_KEY_CUT},
    {8,  6, {0x00, 0xff, 0x00}, 0, 0, false, "O",   HID_KEY_F12},
    {9,  1, {0x00, 0xff, 0x00}, 0, 0, false, "I",   HID_KEY_PASTE},

    {10, 18, {0x00, 0x00, 0xff}, 0, 0, false, "L",  KEYBOARD_MODIFIER_LEFTCTRL},
    {11, 13, {0x00, 0x00, 0xff}, 0, 0, false, ";",  HID_KEY_PAGE_UP},
    {12, 10, {0x00, 0x00, 0xff}, 0, 0, false, "H",  HID_KEY_HOME},
    {13,  5, {0x00, 0x00, 0xff}, 0, 0, false, "W",  HID_KEY_ARROW_UP},
    {14,  2, {0x00, 0x00, 0xff}, 0, 0, false, "E",  HID_KEY_END},

    {15, 19, {0xff, 0xff, 0xff}, 0, 0, false, ".",  KEYBOARD_MODIFIER_LEFTALT},
    {16, 12, {0xff, 0xff, 0xff}, 0, 0, false, "\'", HID_KEY_PAGE_DOWN},
    {17, 11, {0xff, 0xff, 0xff}, 0, 0, false, "A",  HID_KEY_ARROW_LEFT},
    {18,  4, {0xff, 0xff, 0xff}, 0, 0, false, "S",  HID_KEY_ARROW_DOWN},
    {19,  3, {0xff, 0xff, 0xff}, 0, 0, false, "D",  HID_KEY_ARROW_RIGHT}
};

void setup_rows()
{
    // For each row, set gpio to read
    for ( int i = 0; i < key_row_count; ++i ){
        gpio_init(key_rows[i]);
        gpio_set_input_enabled (key_rows[i],1);
        gpio_set_dir (key_rows[i], GPIO_IN);
    }
}

void setup_cols()
{
    // For each column, set gpio to write
    for ( int i = 0; i <= key_col_count; ++i ){
        gpio_init(key_cols[i]);
        gpio_set_dir(key_cols[i], true);
        gpio_put(key_cols[i], false);
    }
}

int find_key_by_led_index(int led_index){

    for(int i = 0; i < sizeof(keeb_keys) / sizeof(Keeb_Key); ++i){
        if(keeb_keys[i].led_index == led_index){
            return keeb_keys[i].key_index;
        }
    }
    return -1;
}

Keeb_Key find_key_by_gpio(int col, int row_gpio){
    int row;
    int key_index = -1;

    switch (row_gpio){
        case key_row0:
            row = 0;
            break;
        case key_row1:
            row = 1;
            break;
        case key_row2:
            row = 2;
            break;
        case key_row3:
            row = 3;
            break;
        default:
            row = -1;
            break;
    }
    if (col > -1 && row > -1){
        key_index = key_xy[row][col];
    }
    return keeb_keys[key_index];
}
 
uint32_t delay_timeout(){
    // Sets a timestamp in the future, cast out by value in key_delay_ms
    uint32_t now = to_ms_since_boot(get_absolute_time());
    return now + (key_delay_ms);
}

uint32_t get_now(){
    return to_ms_since_boot(get_absolute_time());
}

void setup_keys() {
    for ( int i = 0; i < total_key_count; ++i ) {
        keeb_keys[i].issued = false;
        keeb_keys[i].state = 0;
        keeb_keys[i].delay_timeout = delay_timeout();
    }
}

void test_rows(uint col) {
    // For each row in this row, test if high
    for (int i = 0; i  < key_row_count; ++i){
        // test if this row is high
        bool row_state = gpio_get(key_rows[i]);
        sleep_ms(1);
        Keeb_Key keeb_key = find_key_by_gpio(col, key_rows[i]);
        
        keeb_keys[keeb_key.key_index].state = row_state;
        if (keeb_keys[keeb_key.key_index].state == 1) {
            uint32_t now = get_now();
            int time_delta = keeb_keys[keeb_key.key_index].delay_timeout - now;
            bool button_triggered = keeb_keys[keeb_key.key_index].state == 1? true : false;
            bool delay_time_expired = (time_delta < 0) ? true : false;
            bool key_unused = keeb_keys[keeb_key.key_index].issued ? false: true;

            // initially set the button timeout to now if set to 0
            if ( (button_triggered) && ((delay_time_expired) && (key_unused)) ) {
                // this only triggers when the key repeat delay has expired
                keeb_keys[keeb_key.key_index].rgb[0] = 0;
                keeb_keys[keeb_key.key_index].rgb[1] = 0;
                keeb_keys[keeb_key.key_index].rgb[2] = 0;
                keeb_keys[keeb_key.key_index].issued = false;
                keeb_keys[keeb_key.key_index].delay_timeout = delay_timeout();
                // TODO - trigger the key press, the HID report should set the .issued to true, not here
                char text[32];
                sprintf(text, "%s", keeb_keys[keeb_key.key_index].name); 
                add_screen_line(text);
                keeb_keys[keeb_key.key_index].issued = true;
            }
            else if ( (button_triggered) && (!delay_time_expired) && (!key_unused) ) {
                // persist the rolling state
                keeb_keys[keeb_key.key_index].issued = true;
            }
            else if ( (button_triggered) && (!delay_time_expired) && (key_unused) ) {
                // update the delay timeout
                keeb_keys[keeb_key.key_index].delay_timeout = delay_timeout();
            }
            else {
                // clear the states
                keeb_keys[keeb_key.key_index].delay_timeout = get_now();
                keeb_keys[keeb_key.key_index].state = 0;
                keeb_keys[keeb_key.key_index].issued = false;
            }
        }
    }
}

void scan_cols() {
    // set each col high, then test underlying rows
    for (int i = 0; i < key_col_count; ++i) {
        //set col high
        gpio_put(key_cols[i], true);

        // test rows
        test_rows(i);

        // set col low
        gpio_put(key_cols[i], false);
        sleep_ms(1);
    }
}

static inline void put_pixel(uint32_t pixel_grb) {
    // send the RGB value for a LED pixel
    pio_sm_put_blocking(pio0, 0, pixel_grb << 8u);
}

static inline uint32_t urgb_u32(uint8_t r, uint8_t g, uint8_t b) {
    // create a uint32 from 3 hex values for a hex based RGB colour
    return
            ((uint32_t) (r) << 8) |
            ((uint32_t) (g) << 16) |
            (uint32_t) (b);
}

void set_key_leds() {
    // Loops through all keeb_keys
    for (int i = 0; i < sizeof(keeb_keys) / sizeof(Keeb_Key); ++i) {
        // use the i incremented index as a point of reference to get the keeb_key
        //add_screen_line("in lights");        
        int keeb_key_index = find_key_by_led_index(i);
        // then use that keys RGB values to set the LED colour
        put_pixel(
            urgb_u32(
                keeb_keys[keeb_key_index].rgb[0],
                keeb_keys[keeb_key_index].rgb[1],
                keeb_keys[keeb_key_index].rgb[2]
            )
        );
    }
}

// Invoked when device is mounted
void tud_mount_cb(void) {
    add_screen_line("Mounted");
}

// Invoked when device is unmounted
void tud_umount_cb(void) {
    add_screen_line("Unmounted");
}

// Invoked when usb bus is suspended
// remote_wakeup_en : if host allow us  to perform remote wakeup
// Within 7ms, device must draw an average of current less than 2.5 mA from bus
void tud_suspend_cb(bool remote_wakeup_en) {
    (void) remote_wakeup_en;
    add_screen_line("Suspended");
}

// Invoked when usb bus is resumed
void tud_resume_cb(void) {
    add_screen_line("Resumed Mounted");
}

// USB HID Task
bool hid_task(void) {

    
    // Create an empty set of keycodes ready to hold up to 6 key presses
    uint8_t keycodes[6];

    // Iterate over buttons and collect their states (max 6 keypresses)
    for (int i = 0; i < total_key_count; ++i) {
        uint8_t hid_keycode_counter = 0;
        if(hid_keycode_counter < 6) {
            // If button state is true and not already issued, add to the keycodes
            Keeb_Key keeb_key = keeb_keys[i];
            if (keeb_key.state == 1){
                char text [32];
                sprintf(text, "%i %s", keeb_key.issued, keeb_key.keycode);
                add_screen_line(text);
                keycodes[hid_keycode_counter] = keeb_key.keycode;
                printf("issued: %s, key counter: %i, keycode: %s", (!keeb_key.issued == true ? "true": "false"), hid_keycode_counter, keeb_key.keycode);
                ++hid_keycode_counter;
            } 
        }
    }
    // Issue the report 
    bool hid_report = tud_hid_keyboard_report(REPORT_ID_KEYBOARD, 0 , keycodes);
    return hid_report;
}


// Invoked when received GET_REPORT control request
// Application must fill buffer report's content and return its length.
// Return zero will cause the stack to STALL request
uint16_t tud_hid_get_report_cb(uint8_t itf, uint8_t report_id, hid_report_type_t report_type, uint8_t *buffer, uint16_t reqlen) {
    printf("tud_hid_get_report_cb triggered\n");
    (void) report_id;
    (void) report_type;
    (void) buffer;
    (void) reqlen;

    return 0;
}

// Invoked when received SET_REPORT control request or
// received data on OUT endpoint ( Report ID = 0, Type = 0 )
void tud_hid_set_report_cb(uint8_t itf, uint8_t report_id, hid_report_type_t report_type, uint8_t const *buffer, uint16_t bufsize) {
    printf("tud_hid_set_report_cb triggered\n");
    printf("report_id: %X\n", report_id);
    printf("report_type: %X\n", report_type);
    printf("bufsize: %d\n", bufsize);

    printf("buffer content:\n");
    for (int i = 0; i < bufsize; i++) {
        printf("%02X ", buffer[i]);
    }
    printf(" - End \n");
    (void) report_id;
    (void) report_type;
    (void) buffer;
    (void) bufsize;
}



int main() {
    stdio_init_all();

    //Init oled
    setup_screen_gpios();
    disp.external_vcc=false;
    ssd1306_init(&disp, 128, 64, 0x3C, i2c1);
    setup_screen_text();

    // Init Board
    board_init();
    add_screen_line("Board init");

    // Init TinyUSB
    tusb_init();
    add_screen_line("TUSB init");

    // Init key watches events
    setup_cols();
    add_screen_line("Cols init");
    setup_rows();
    add_screen_line("Rows init");
    setup_keys();
    add_screen_line("Keys init");

    // Init WS2812
    PIO pio = pio0;
    int sm = 0;
    uint offset = pio_add_program(pio, &ws2812_program);
    //add_screen_line("PIO added");
    ws2812_program_init(pio, sm, offset, PIN_TX, 800000, false);
    set_key_leds();
    add_screen_line("LEDs init");
    
    while (1) {
        tud_task();     // TinyUSB device task
        scan_cols();    // Get key presses
        //hid_task();     //HID Task
        // char text[32];
        // bool usb_inited = tusb_inited();
        // sprintf(text, "Latest: %i, %i", t, usb_inited); 
        //add_screen_line(text);
        // int pat = rand() % count_of(pattern_table);
        // int dir = (rand() >> 30) & 1 ? 1 : -1;
        // puts(pattern_table[pat].name);
        // puts(dir == 1 ? "(forward)" : "(backward)");
        // for (int i = 0; i < 1000; ++i) {
        //     pattern_table[pat].pat(20, 255);
        //     slvoideep_ms(10);
        //     t += dir;
        // }
        // sleep_ms(500);
        // char text[32];
        // sprintf(text, "Latest: %i", get_now()); 
        // add_screen_line(text);
        //sleep_ms(50);
    }
    return 1;
}