#pragma once
#include "pico/stdlib.h"

typedef enum
{
    UP,
    DOWN,
    LEFT,
    RIGHT,
    A,
    B,
    X,
    Y,
    START,
    SELECT,
    MENU
} pad_button_t;

void controls_init();
void controls_update();
bool controls_is_button_pressed(pad_button_t button);
bool controls_was_button_just_pressed(pad_button_t button);