#pragma once
#include <stdint.h>
#include "types.h"

// Vykreslení bitmapy na displej. Bitmapa je pole 16-bitových barev v RGB565 formátu.
void display_bmp_draw(uint16_t *bitmap, uint16_t x, uint16_t y, uint16_t width, uint16_t height);