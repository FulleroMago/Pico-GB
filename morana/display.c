#include "display.h"
#include "ili9341.h"
#include "config.h"
#include "gfx.h"
#include "hardware/clocks.h"

void display_init(uint16_t color)
{
    // Initialize display
    puts("Initializing display...");
    LCD_setPins(TFT_DC, TFT_CS, TFT_RST, TFT_SCLK, TFT_MOSI);

    // Nastavení SPI pro komunikaci s displejem
    clock_configure(clk_peri, 0,
                    CLOCKS_CLK_PERI_CTRL_AUXSRC_VALUE_CLK_SYS,
                    125 * 1000 * 1000, 125 * 1000 * 1000);

    gpio_set_slew_rate(TFT_SCLK, GPIO_SLEW_RATE_FAST);
    gpio_set_slew_rate(TFT_MOSI, GPIO_SLEW_RATE_FAST);

    LCD_initDisplay();
    LCD_setRotation(TFT_ROTATION);
    // GFX_createFramebuf(); // Frame buffer spotřebuje 320*240*2 = 150kB RAM, což je víc než máme, takže ji nebudeme používat a budeme kreslit přímo do LCD
    GFX_setClearColor(color);
    // GFX_setTextBack(BACKGROUND);
    // GFX_setTextColor(FOREGROUND);
    GFX_clearScreen();

    display_flush();
}

void display_deinit()
{
    puts("Deinitializing display...");
    spi_deinit(TFT_SPI);
    // GFX_destroyFramebuf(); // Frame buffer spotřebuje 320*240*2 = 150kB RAM, což je víc než máme, takže ji nebudeme používat a budeme kreslit přímo do LCD
}

void display_clear()
{
    GFX_clearScreen();
}

void display_text_set_bgcolor(uint16_t color)
{
    GFX_setTextBack(color);
}

void display_text_set_color(uint16_t color)
{
    GFX_setTextColor(color);
}

uint8_t line = 0;
uint8_t text_box_x = 0;
uint8_t text_box_y = 0;
uint8_t line_height = 8;

void display_text_set_cursor(uint8_t new_x, uint8_t new_y)
{
    text_box_x = new_x;
    text_box_y = new_y;
    GFX_setCursor(text_box_x, text_box_y);
    line = 0;
}

void display_text_write_line(const char *str)
{
    GFX_setCursor(text_box_x, text_box_y + line * line_height);
    GFX_printf(str);

    line++;
}

void display_text_write_line_len(const char *str, size_t len)
{
    GFX_setCursor(text_box_x, text_box_y + line * line_height);

    GFX_printf("%.*s", (int)len, str);

    line++;
}

void display_text_write_line_color(const char *str, uint16_t color, uint16_t bgcolor)
{
    GFX_setCursor(text_box_x, text_box_y + line * line_height);
    GFX_setTextColor(color);
    GFX_setTextBack(bgcolor);
    GFX_printf(str);

    line++;
}

void display_text_write_line_color_len(const char *str, size_t len, uint16_t color, uint16_t bgcolor)
{
    GFX_setCursor(text_box_x, text_box_y + line * line_height);
    GFX_setTextColor(color);
    GFX_setTextBack(bgcolor);
    GFX_printf("%.*s", (int)len, str);

    line++;
}

void display_set_line_height(uint8_t new_line_height)
{
    line_height = new_line_height;
}

uint8_t display_get_line_height()
{
    return line_height;
}

void display_draw_bitmap(const uint16_t *bitmap, uint16_t x, uint16_t y, uint16_t width, uint16_t height)
{
    LCD_WriteBitmap(x, y, width, height, bitmap);
}

void display_draw_line(uint16_t color, uint16_t x, uint16_t y, uint16_t width)
{
    GFX_drawFastHLine(x, y, width, color);
}

void display_flush()
{
    GFX_flush();
}