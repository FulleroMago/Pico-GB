#pragma once

#include <stdio.h>
#include "pico/stdlib.h"
#include "display_bmp.h"
#include "display_txt.h"
#include "ili9341.h"
#include "types.h"

// initializace displeje
void display_init();

// vymazání displeje
void display_clear();

// deinicializace displeje
void display_deinit();

// flush displeje (vykreslení obsahu frame bufferu na displej)
void display_flush();