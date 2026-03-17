#pragma once
#include <stdint.h>
#include "types.h"

// Vykreslení čáry na displej. Barva je 16-bitová v RGB565 formátu.
void display_bmp_draw_line(uint16_t color, uint16_t x, uint16_t y, uint16_t width);

// Vykreslení pixelu na displej. Barva je 16-bitová v RGB565 formátu.
void display_bmp_draw_pixel(uint16_t color, uint16_t x, uint16_t y);

// Vykreslení obdélníku na displej.
void display_shape_rect(uint16_t x, uint16_t y, uint16_t width, uint16_t height, uint16_t color, bool filled);