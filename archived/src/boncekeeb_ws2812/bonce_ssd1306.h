#include "ssd1306.h"

#define SLEEPTIME 25
#define SCREEN_MAX_TEXT_LINES 9
#define SCREEN_MAX_TEXT_CHARS 16 

ssd1306_t disp;

void setup_gpios(void);
void animation(void);
void setup_screen_gpios(void);
bool display_text();
bool add_screen_line( char text_line[SCREEN_MAX_TEXT_CHARS] );
void setup_screen_text();
bool add_line_when_full( char text_line[SCREEN_MAX_TEXT_CHARS] );

typedef struct {
    char text_line[SCREEN_MAX_TEXT_CHARS];
} Screen_Text_Line;


typedef struct {
    Screen_Text_Line text_lines[SCREEN_MAX_TEXT_LINES];
    int current_line;
} Screen_Text_Buffer;



Screen_Text_Buffer screen_text_buffer;
