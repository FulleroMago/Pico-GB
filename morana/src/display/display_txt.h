#pragma once
#include <stdint.h>
#include <stddef.h>
#include "types.h"

// enum text_align
// {
//     DISPLAY_TXT_ALIGN_LEFT,
//     DISPLAY_TXT_ALIGN_CENTER,
//     DISPLAY_TXT_ALIGN_RIGHT
// };

enum wrap_mode
{
    DISPLAY_TXT_WRAP_NONE,
    DISPLAY_TXT_WRAP_CHAR,
    DISPLAY_TXT_WRAP_WORD
};

// Struktura pro textový box, který určuje oblast pro vykreslování textu, barvu textu a pozadí, a aktuální pozici kurzoru
typedef struct
{
    box box;               // pozice a rozměry textového boxu
    uint8_t line_margin;   // mezera mezi řádky
    uint16_t text_color;   // barva textu
    uint16_t text_bgcolor; // barva pozadí textu
    uint8_t line;          // aktuální řádek v textovém boxu
    uint8_t column;        // aktuální pozice kurzoru v řádku
    enum wrap_mode wrap;   // režim zalamování textu
} text_box;

// Struktura pro uchování informací o aktuálním textovém boxu a pozici kurzoru
extern text_box current_text_box;

// Výchozí textový box, který pokrývá celý displej a má výchozí barvy a mezery mezi řádky
extern text_box default_text_box;

// Nastaví nový textový box pro vykreslování textu
void display_txt_set_text_box(text_box *box);

// nastavení umístění kurzoru pro text
void display_txt_set_cursor(uint8_t line, uint8_t column);

// zápis řádku textu na aktuální pozici kurzoru a posunutí kurzoru na další řádek
void display_txt_write_line(const char *s);

// zápis řádku textu s nastavením délky a posunutí kurzoru na další řádek
void display_txt_write_line_len(const char *str, size_t len);

// zápis řádku textu s nastavením barvy textu a pozadí
void display_txt_write_line_color(const char *s, uint16_t color, uint16_t bgcolor);

// zápis řádku textu s nastavením délky, barvy textu a pozadí
void display_txt_write_line_color_len(const char *str, size_t len, uint16_t color, uint16_t bgcolor);

// nastavení barvy pozadí pro text
void display_txt_set_bgcolor(uint16_t color);

// nastavení barvy textu
void display_txt_set_color(uint16_t color);