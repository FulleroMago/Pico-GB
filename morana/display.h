#pragma once

#include <stdio.h>
#include "pico/stdlib.h"

// initializace displeje
void display_init(uint16_t color);

// vymazání displeje
void display_clear();

// deinicializace displeje
void display_deinit();

// flush displeje (vykreslení obsahu frame bufferu na displej)
void display_flush();

// nastavení umístění kurzoru pro text
void display_text_set_cursor(uint8_t new_x, uint8_t new_y);

// zápis řádku textu na aktuální pozici kurzoru a posunutí kurzoru na další řádek
void display_text_write_line(const char *s);

// zápis řádku textu s nastavením délky a posunutí kurzoru na další řádek
void display_text_write_line_len(const char *str, size_t len);

// zápis řádku textu s nastavením barvy textu a pozadí
void display_text_write_line_color(const char *s, uint16_t color, uint16_t bgcolor);

// zápis řádku textu s nastavením délky, barvy textu a pozadí
void display_text_write_line_color_len(const char *str, size_t len, uint16_t color, uint16_t bgcolor);

// nastavení barvy pozadí pro text
void display_text_set_bgcolor(uint16_t color);

// nastavení barvy textu
void display_text_set_color(uint16_t color);

// získání výšky řádku textu
uint8_t display_get_line_height();

// nastavení výšky řádku textu
void display_set_line_height(uint8_t new_line_height);