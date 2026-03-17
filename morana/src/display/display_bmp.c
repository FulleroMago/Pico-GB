#include "display_bmp.h"
#include "display.h"
#include "ili9341.h"

// Vykreslení bitmapy na displej. Bitmapa je pole 16-bitových barev v RGB565 formátu.
void display_bmp_draw(const uint16_t *bitmap, uint16_t x, uint16_t y, uint16_t width, uint16_t height)
{
    lcd_write_bitmap(x, y, width, height, bitmap);
}