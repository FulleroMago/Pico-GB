#include "display_txt.h"
#include "display.h"
#include "ili9341.h"
#include "fonts/font_basic.h"
#include <string.h>

// Výchozí textový box, který pokrývá celý displej a má výchozí barvy a mezery mezi řádky
text_box default_text_box = {
    {{0, 0}, {0, 0}},     // dimensions x, y, width, height
    2,                    // line_margin
    0xFFFF,               // text_color
    0x0000,               // text_bgcolor
    0,                    // line
    0,                    // column
    DISPLAY_TXT_WRAP_WORD // wrap
};

// Aktuální textový box
text_box current_text_box;

// Spočítá pozici na displeji aktuálního kurzoru pro text a vrátí ji jako vector2d
vector2d display_txt_get_cursor_coordinates()
{
    vector2d pos;
    pos.x = current_text_box.box.position.x + (current_text_box.column * (font_glyph_width));
    pos.y = current_text_box.box.position.y + (current_text_box.line * (font_glyph_height + current_text_box.line_margin));
    return pos;
}

// Zápis jednoho znaku na pozici kurzoru s danou barvou textu a pozadí
void display_txt_put_char(char c, vector2d position, uint16_t color, uint16_t bgcolor)
{
    if (c == '\n')
    {
        current_text_box.line++;
        current_text_box.column = 0;
    }
    else if (c != '\r')
    {
        uint16_t glyph_buffer[font_glyph_height][font_glyph_width];

        // Kontrola, zda je znak v rozsahu a má definovanou glyph
        uint8_t glyph_index = (uint8_t)c;
        const uint8_t *glyph_ptr = NULL;
        if (glyph_index < 128 && font_glyphs[glyph_index][0] != 0)
        {
            glyph_ptr = font_glyphs[glyph_index];
        }
        else
        {
            // Pokud není glyph definován, použijeme mezeru
            glyph_ptr = font_glyphs[' '];
        }

        lcd_wait_until_not_busy(); // počkej, dokud není displej připraven pro další příkazy (např. dokončení DMA přenosu)
        for (int i = 0; i < font_glyph_height; i++)
        {
            uint8_t row = glyph_ptr[i];
            for (int j = 0; j < font_glyph_width; j++)
            {
                // Správné pořadí bitů: nejvyšší bit je vlevo
                glyph_buffer[i][j] = (row & (1 << (7 - j))) ? color : bgcolor;
            }
        }

        lcd_write_bitmap(position.x, position.y, font_glyph_width, font_glyph_height, (uint16_t *)glyph_buffer);
    }
}

// Nastaví nový textový box pro vykreslování textu
void display_txt_set_text_box(text_box *box)
{
    current_text_box = *box;
}

// Nastavení barvy pozadí pro text v aktuálním textovém boxu
void display_txt_set_bgcolor(uint16_t color)
{
    current_text_box.text_bgcolor = color;
}

// Nastavení barvy textu v aktuálním textovém boxu
void display_txt_set_color(uint16_t color)
{
    current_text_box.text_color = color;
}

// Nastavení umístění kurzoru pro text
void display_txt_set_cursor(uint8_t line, uint8_t column)
{
    current_text_box.line = line;
    current_text_box.column = column;
}

// Zápis řádku textu na aktuální pozici kurzoru a posunutí kurzoru na další řádek
void display_txt_write_line(const char *str)
{
    display_txt_write_line_len(str, strlen(str));
}

// Zápis řádku textu s nastavením délky a posunutí kurzoru na další řádek
void display_txt_write_line_len(const char *str, size_t len)
{
    display_txt_write_line_color_len(str, len, current_text_box.text_color, current_text_box.text_bgcolor);
}

// Zápis řádku textu s nastavením barvy textu a pozadí
void display_txt_write_line_color(const char *str, uint16_t color, uint16_t bgcolor)
{
    display_txt_write_line_color_len(str, strlen(str), color, bgcolor);
}

// Zápis řádku textu s nastavením délky, barvy textu a pozadí
void display_txt_write_line_color_len(const char *str, size_t len, uint16_t color, uint16_t bgcolor)
{
    int i = 0;
    while (i < len) {

        vector2d cursor_pos = display_txt_get_cursor_coordinates();

        // Výpočet maximálního počtu znaků na řádek
        int max_chars = (current_text_box.box.size.width) / font_glyph_width;

        if (current_text_box.wrap == DISPLAY_TXT_WRAP_NONE)
        {
            // Vypisuje pouze do konce řádku, pak končí
            for (int j = 0; j < max_chars && i < len; j++, i++) {
                cursor_pos = display_txt_get_cursor_coordinates();
                display_txt_put_char(str[i], cursor_pos, color, bgcolor);
                current_text_box.column++;
            }
            break;
        }
        else if (current_text_box.wrap == DISPLAY_TXT_WRAP_CHAR) {
            // Zalamuje na znak
            if (cursor_pos.x + font_glyph_width > current_text_box.box.position.x + current_text_box.box.size.width) {
                current_text_box.line++;
                current_text_box.column = 0;
                cursor_pos = display_txt_get_cursor_coordinates();
            }
            display_txt_put_char(str[i], cursor_pos, color, bgcolor);
            current_text_box.column++;
            i++;
        }
        else if (current_text_box.wrap == DISPLAY_TXT_WRAP_WORD) {

            // Zalamuje na slovo
            // Najdi délku dalšího slova
            int word_start = i;
            int word_len = 0;
            while (i + word_len < len && str[i + word_len] != ' ' && str[i + word_len] != '\n') {
                word_len++;
            }

            // Pokud se nevejde celé slovo, zalomí
            if (current_text_box.column + word_len > max_chars) {
                current_text_box.line++;
                current_text_box.column = 0;
                cursor_pos = display_txt_get_cursor_coordinates();
            }

            // Vypiš slovo
            for (int k = 0; k < word_len && i < len; k++, i++) {
                cursor_pos = display_txt_get_cursor_coordinates();
                display_txt_put_char(str[i], cursor_pos, color, bgcolor);
                current_text_box.column++;
            }

            // Pokud je mezera, vypiš ji
            if (i < len && str[i] == ' ') {
                cursor_pos = display_txt_get_cursor_coordinates();
                display_txt_put_char(' ', cursor_pos, color, bgcolor);
                current_text_box.column++;
                i++;
            }

            // Pokud je \n, zalomí řádek
            if (i < len && str[i] == '\n') {
                current_text_box.line++;
                current_text_box.column = 0;
                i++;
            }
        }
        else {
            
            // Výchozí: zalamuje na znak
            if (cursor_pos.x + font_glyph_width > current_text_box.box.position.x + current_text_box.box.size.width) {
                current_text_box.line++;
                current_text_box.column = 0;
                cursor_pos = display_txt_get_cursor_coordinates();
            }
            display_txt_put_char(str[i], cursor_pos, color, bgcolor);
            current_text_box.column++;
            i++;
        }
    }
    current_text_box.line++;
    current_text_box.column = 0;
}