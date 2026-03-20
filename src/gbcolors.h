/**
 * MIT License
 *
 * Copyright (c) 2022 Vincent Mistler
 * Original source code from deltabeard/Peanut-GB Copyright (c) 2018 Mahyar Koshkouei
 * https://github.com/deltabeard/Peanut-GB/blob/master/examples/sdl2/peanut_sdl.c
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to
 * deal in the Software without restriction, including without limitation the
 * rights to use, copy, modify, merge, publish, distribute, sublicense, and/or
 * sell copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
 * IN THE SOFTWARE.
 */

#pragma once

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

#define NUMBER_OF_MANUAL_PALETTES 13
#define PALETTE_SIZE_IN_BYTES (3 * 4 * sizeof(uint16_t))

typedef uint16_t palette_t[3][4];

void get_colour_palette(palette_t selected_palette, uint8_t table_entry, uint8_t shuffling_flags);
void auto_assign_palette(uint16_t palette[3][4], uint8_t game_checksum, const char *game_title);
void manual_assign_palette(palette_t palette, uint8_t selection);
