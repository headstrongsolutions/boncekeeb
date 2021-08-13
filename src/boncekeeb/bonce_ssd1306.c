#include "pico/stdlib.h"
#include <string.h>
#include "bonce_ssd1306.h"

/**
 * @brief Set up the SSD1306 GPIO's
 * 
 */
void setup_screen_gpios(void) {
    i2c_init(i2c1, 400000);
    gpio_set_function(2, GPIO_FUNC_I2C);
    gpio_set_function(3, GPIO_FUNC_I2C);
    gpio_pull_up(2);
    gpio_pull_up(3);
}

/**
 * @brief Loop through all text lines and print to screen (after clearing it)
 * 
 */
bool display_text() {
    ssd1306_clear(&disp);
    uint8_t y = 0;
    for ( int i = 0; i < SCREEN_MAX_TEXT_LINES; i++ ) {
        char text_line[SCREEN_MAX_TEXT_CHARS];
        strcpy(text_line, screen_text_buffer.text_lines[i].text_line);
        ssd1306_draw_string(&disp, 0, y, 1, screen_text_buffer.text_lines[i].text_line);
        if ( i > 0 ) {
            y = ( y + 8 );
        }
        else {
            y = 8;
        }
    }
    ssd1306_show(&disp);
    return true;
}

/**
 * @brief Adds a line of text to the text lines collection, pop/pushing if
 *        the collection is full
 * 
 * @param char text_line[SCREEN_MAX_TEXT_CHARS] 
 * @return true/false on the display updating without issue
 */
bool add_screen_line( char text_line[SCREEN_MAX_TEXT_CHARS] ) {
    if ( screen_text_buffer.current_line == 0 ) {
        strcpy(screen_text_buffer.text_lines[0].text_line, text_line);
        ++screen_text_buffer.current_line;
    }
    else if ( screen_text_buffer.current_line < SCREEN_MAX_TEXT_LINES - 1 ) {
        strcpy(screen_text_buffer.text_lines[screen_text_buffer.current_line].text_line, text_line);
        ++screen_text_buffer.current_line;
    }
    else {
        add_line_when_full(text_line);
    }
    return display_text();
}

/**
 * @brief Initialises the screen text lines
 * 
 */
void setup_screen_text(){
    add_screen_line("");
    add_screen_line("");
    add_screen_line("   Bonce Keeb");
    add_screen_line("");
    add_screen_line("   Headstrong");
    add_screen_line("   Solutions");
    add_screen_line("");

    add_screen_line("            2021");
}

/**
 * @brief Pushes a text line onto the end of the lines collection, shuffling
 *        back up (replacing first line)
 * 
 * @param char text_line[SCREEN_MAX_TEXT_CHARS]
 * @return true 
 */
bool add_line_when_full( char text_line[SCREEN_MAX_TEXT_CHARS] ) {
    for ( int i = 0; i < SCREEN_MAX_TEXT_LINES-1; i++ ) {
        strcpy(screen_text_buffer.text_lines[i].text_line, screen_text_buffer.text_lines[i+1].text_line);
    }
    strcpy(screen_text_buffer.text_lines[SCREEN_MAX_TEXT_LINES -2].text_line, text_line);
    screen_text_buffer.current_line = SCREEN_MAX_TEXT_LINES;
    return true;
}