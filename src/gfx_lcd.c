#include "gfx_lcd.h"
#include "config.h"
#include "main.h"
#include "display/display.h"

palette_t palette; // Colour palette

static uint8_t scaledLineOffsetTable[LCD_HEIGHT]; // scaled to 240 lines

#define IS_REPEATED(pos) ((pos % 2) || (pos % 6 == 0))

#ifdef USE_BUFFERING
static uint16_t display_buffers[2][DISPLAY_WIDTH * BUFFERED_LINES]; // buffer pro 10 řádků, který se používá pro různé režimy kreslení (stretched, interlace atd.)
static uint8_t active_display_buffer = 0;                           // index aktivního bufferu
#endif

static void calcExtraLineTable()
{
    uint8_t offset = 0;
    for (uint8_t line = 0; line < LCD_HEIGHT; ++line)
    {
        scaledLineOffsetTable[line] = offset;
        offset += 1 + IS_REPEATED(line);
    }
}

#ifdef USE_BUFFERING
void write_display_buffer_line(const uint16_t *pixels, uint16_t x, uint16_t y, uint16_t width)
{
    memcpy(&display_buffers[active_display_buffer][y * DISPLAY_WIDTH + x], pixels, width * sizeof(uint16_t));
}
#endif

void gfx_init()
{
    calcExtraLineTable();
}

void lcd_write_pixels_normal(const uint16_t *pixels, uint8_t line, uint_fast16_t count)
{
    const uint16_t colOffset = (DISPLAY_WIDTH - LCD_WIDTH) / 2;
    const uint16_t screenLineOffset = (DISPLAY_HEIGHT - LCD_HEIGHT) / 2;

    lcd_wait_until_not_busy();
    display_bmp_draw(pixels, colOffset, screenLineOffset + line, LCD_WIDTH, 1);
}

void lcd_write_pixels_stretched(const uint16_t *pixels, uint8_t line, uint_fast16_t count)
{
    const uint8_t lineRepeated = IS_REPEATED(line);
    const uint16_t lineOffset = scaledLineOffsetTable[line];

    static uint16_t doubledPixels[DISPLAY_WIDTH * 2];
    uint16_t pos = 0;

#ifndef USE_BUFFERING
    lcd_wait_until_not_busy(); // počkej, dokud není displej připraven pro další příkazy (např. dokončení DMA přenosu)
#endif

    for (int col = 0; col < count; ++col)
    {
        doubledPixels[pos++] = pixels[col];
        doubledPixels[pos++] = pixels[col];
    }

#ifdef USE_BUFFERING

    static uint16_t filledLines = 0;
    uint16_t bufferLine = lineOffset % BUFFERED_LINES;
    write_display_buffer_line(doubledPixels, 0, bufferLine, DISPLAY_WIDTH);

    filledLines++;
    if (lineRepeated)
    {
        uint16_t nextBufferLine = (bufferLine + 1) % BUFFERED_LINES;
        write_display_buffer_line(doubledPixels, 0, nextBufferLine, DISPLAY_WIDTH);
        filledLines++;
    }

    // Pokud je buffer plný, vypiš na display
    if (filledLines >= BUFFERED_LINES || line == LCD_HEIGHT - 1)
    {
        display_wait_until_not_busy(); // počkej, dokud není displej připraven pro další příkazy (např. dokončení DMA přenosu)
        uint8_t firstLineInBuffer = (lineOffset / BUFFERED_LINES) * BUFFERED_LINES;
        display_bmp_draw(display_buffers[active_display_buffer], 0, firstLineInBuffer, DISPLAY_WIDTH, BUFFERED_LINES);

        // Přepni aktivní buffer
        active_display_buffer = (active_display_buffer + 1) % 2;

        filledLines = 0;
    }
#else

    display_bmp_draw(doubledPixels, 0, lineOffset, DISPLAY_WIDTH, 1);
    if (lineRepeated)
    {
        lcd_wait_until_not_busy();
        display_bmp_draw(doubledPixels, 0, lineOffset + 1, DISPLAY_WIDTH, 1);
    }
#endif
}

void lcd_write_pixels_stretched_keep_aspect(const uint16_t *pixels, uint8_t line, uint_fast16_t count)
{
    static uint16_t doubledPixels[DISPLAY_WIDTH];
    uint16_t pos = 0;

    const uint8_t lineRepeated = IS_REPEATED(line);
    const uint16_t lineOffset = scaledLineOffsetTable[line];

#ifndef USE_BUFFERING
    lcd_wait_until_not_busy(); // počkej, dokud není displej připraven pro další příkazy (např. dokončení DMA přenosu)
#endif

    for (int col = 0; col < count; ++col)
    {
        doubledPixels[pos++] = pixels[col];
        if (IS_REPEATED(col))
        {
            doubledPixels[pos++] = pixels[col];
        }
    }

    const uint16_t stretchedWidth = pos;
    const uint16_t colOffset = (DISPLAY_WIDTH - stretchedWidth) / 2;

#ifdef USE_BUFFERING

    // Cyclic buffer logic
    static uint16_t filledLines = 0;
    uint16_t bufferLine = lineOffset % BUFFERED_LINES;
    write_display_buffer_line(doubledPixels, colOffset, bufferLine, stretchedWidth);
    filledLines++;
    if (lineRepeated)
    {
        uint16_t nextBufferLine = (bufferLine + 1) % BUFFERED_LINES;
        write_display_buffer_line(doubledPixels, colOffset, nextBufferLine, stretchedWidth);
        filledLines++;
    }

    // Pokud je buffer plný, vypiš na display
    if (filledLines >= BUFFERED_LINES || line == LCD_HEIGHT - 1)
    {
        display_wait_until_not_busy(); // počkej, dokud není displej připraven pro další příkazy (např. dokončení DMA přenosu)
        uint8_t firstLineInBuffer = (lineOffset / BUFFERED_LINES) * BUFFERED_LINES;
        display_bmp_draw(display_buffers[active_display_buffer], 0, firstLineInBuffer, DISPLAY_WIDTH, BUFFERED_LINES);

        // Přepni aktivní buffer
        active_display_buffer = (active_display_buffer + 1) % 2;

        filledLines = 0;
    }

#else

    display_bmp_draw(doubledPixels, colOffset, lineOffset, stretchedWidth, 1);
    if (lineRepeated)
    {
        lcd_wait_until_not_busy();
        display_bmp_draw(doubledPixels, colOffset, lineOffset + 1, stretchedWidth, 1);
    }

#endif
}

void lcd_write_pixels_interlace_aspect(const uint16_t *pixels, uint8_t line, uint_fast16_t count)
{
    static uint16_t newPixels[DISPLAY_WIDTH];

    // uint16_t bg_darkest = palette[2][3] / 2;
    // memset(newPixels, 0, sizeof(newPixels)); // vynulování bufferu

    uint16_t pos = 0;
    for (int col = 0; col < count; col++)
    {
        newPixels[pos] = pixels[col];
        pos++;

        if (col % 3 == 0)
        {
            newPixels[pos] = pixels[col]; // nebo jiná barva pozadí z palety
            pos++;
            // newPixels[pos++] = bg_darkest; // nebo jiná barva pozadí z palety
        }
    }

    const uint16_t stretchedWidth = pos - 1;
    const uint16_t colOffset = (DISPLAY_WIDTH - stretchedWidth) / 2;
    const uint16_t screenLineOffset = line * 1.5;

    display_bmp_draw(newPixels, colOffset, screenLineOffset, stretchedWidth, 1);
}

void gfx_draw_line(struct gb_s *gb, const uint8_t pixels[LCD_WIDTH], const uint_fast8_t line)
{
    static uint16_t fb[LCD_WIDTH];

    for (unsigned int x = 0; x < LCD_WIDTH; x++)
    {
        uint16_t color = palette[(pixels[x] & LCD_PALETTE_ALL) >> 4][pixels[x] & 3];
        fb[x] = color;
    }

    switch (current_settings.scaling_mode)
    {
    case STRETCH:
        lcd_write_pixels_stretched(fb, line, LCD_WIDTH);
        break;
    case STRETCH_KEEP_ASPECT:
        lcd_write_pixels_stretched_keep_aspect(fb, line, LCD_WIDTH);
        break;
    // case INTERLACE_KEEP_ASPECT:
    // 	lcd_write_pixels_interlace_aspect(fb, line, LCD_WIDTH);
    // 	break;
    case NORMAL:
    default:
        lcd_write_pixels_normal(fb, line, LCD_WIDTH);
        break;
    }
}