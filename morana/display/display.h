#pragma once

void display_init(void);
void display_fill(uint16_t color);
void display_fill_rect(uint8_t x,uint8_t y,uint8_t w,uint8_t h,uint16_t color);
void display_idle(bool idle);
void display_text(char *s, uint8_t x, uint8_t y, uint16_t color, uint16_t bgcolor);
void display_draw_line(uint_fast8_t line, const uint8_t *pixels, const palette_t palette, size_t len);
// void display_set_x(uint8_t x);
// void display_write_pixels(const uint16_t *pixels, uint_fast16_t nmemb);