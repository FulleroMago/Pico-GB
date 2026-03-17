#pragma once

#include <stdio.h>
#include "pico/stdlib.h"
#include "display_bmp.h"
#include "display_txt.h"
#include "display_shape.h"
#include "ili9341.h"
#include "types.h"

#define DISPLAY_WIDTH 320  // rotated by 90 degrees
#define DISPLAY_HEIGHT 240 // rotated by 90 degrees

// Převod RGB888 na RGB565
static inline uint16_t display_get_color(uint8_t R, uint8_t G, uint8_t B)
{
    uint16_t r = (R >> 3) & 0x1F;
    uint16_t g = (G >> 2) & 0x3F;
    uint16_t b = (B >> 3) & 0x1F;
    return (r << 11) | (g << 5) | b;
}

// Funkce pro zjištění, zda je displej stále zaneprázdněný (např. probíhá DMA přenos)
bool display_is_busy();

// initializace displeje
void display_init();

// vymazání displeje
void display_clear();

// deinicializace displeje
void display_deinit();

// flush displeje (vykreslení obsahu frame bufferu na displej)
void display_flush();

// čeká, dokud není displej připraven pro další příkazy (např. dokončení DMA přenosu)
void display_wait_until_not_busy();