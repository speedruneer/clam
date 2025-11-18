#ifndef TEXT_H
#define TEXT_H

#include <stdint.h>
#include <vesa.h>  // assume this has vesa_mode_info and set_pixel/clear_screen

// Set foreground and background colors for text
void set_text_color(uint8_t r, uint8_t g, uint8_t b, uint8_t br, uint8_t bg, uint8_t bb);

// Draw a single character at pixel coordinates (x, y)
void draw_char(int x, int y, char c);

// Print a string starting at current cursor
void print(const char *s);

// Set cursor position in pixels
void set_cursor(int x, int y);

// Clear the screen and reset cursor
void clear_screen_text();

#endif
