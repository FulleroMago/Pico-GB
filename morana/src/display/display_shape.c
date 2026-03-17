#include "display.h"
#include "config.h"

void display_bmp_draw_line(uint16_t color, uint16_t x, uint16_t y, uint16_t width)
{
    uint16_t lineBuffer[DISPLAY_WIDTH];

    display_wait_until_not_busy(); // počkej, dokud není displej připraven pro další příkazy (např. dokončení DMA přenosu)
    for (uint16_t i = 0; i < width; i++)
    {
        lineBuffer[i] = color;
    }

    display_bmp_draw(lineBuffer, x, y, width, 1);
}

void display_bmp_draw_pixel(uint16_t color, uint16_t x, uint16_t y)
{
    display_bmp_draw_line(color, x, y, 1);
}

void display_shape_rect(uint16_t x, uint16_t y, uint16_t width, uint16_t height, uint16_t color, bool filled)
{
    if (filled)
    {
        for (uint16_t i = 0; i < height; i++)
        {
            display_bmp_draw_line(color, x, y + i, width);
        }
    }
    else
    {
        // Top edge
        display_bmp_draw_line(color, x, y, width);
        // Bottom edge
        display_bmp_draw_line(color, x, y + height - 1, width);
        // Left edge
        for (uint16_t i = 0; i < height; i++)
        {
            display_bmp_draw_pixel(color, x, y + i);
        }
        // Right edge
        for (uint16_t i = 0; i < height; i++)
        {
            display_bmp_draw_pixel(color, x + width - 1, y + i);
        }
    }
}