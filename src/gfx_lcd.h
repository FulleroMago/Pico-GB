#pragma once

#define PEANUT_GB_HEADER_ONLY
#include "peanut_gb.h"
#include "gbcolors.h"

extern palette_t palette; // Colour palette

void gfx_init();
void gfx_draw_line(struct gb_s *gb, const uint8_t pixels[LCD_WIDTH], const uint_fast8_t line);