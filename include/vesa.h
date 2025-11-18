#ifndef VESA_H
#define VESA_H

#include <stdint.h>
#include <klib.h>  // use your mode_info_t

extern mode_info_t vesa_mode_info;

// Basic drawing functions
void set_pixel(int x, int y, uint8_t r, uint8_t g, uint8_t b);
void rectangle(int x, int y, int w, int h, uint8_t r, uint8_t g, uint8_t b);
void line(int x0, int y0, int x1, int y1, uint8_t r, uint8_t g, uint8_t b, int width);
void clear_screen(uint8_t r, uint8_t g, uint8_t b);

// Optional helpers
void draw_hline(int x, int y, int w, uint8_t r, uint8_t g, uint8_t b);
void draw_vline(int x, int y, int h, uint8_t r, uint8_t g, uint8_t b);
void circle(int cx, int cy, int radius, uint8_t r, uint8_t g, uint8_t b);

#endif
