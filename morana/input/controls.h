#pragma once

typedef enum
{
    BUTTON_UP,
    BUTTON_DOWN,
    BUTTON_LEFT,
    BUTTON_RIGHT,
    BUTTON_A,
    BUTTON_B,
    BUTTON_X,
    BUTTON_Y,
    BUTTON_START,
    BUTTON_SELECT,
    BUTTON_MENU,

    BUTTON_COUNT
} hw_button_t;

void controls_init();
void controls_update();
bool is_button_pressed(hw_button_t button);
bool was_button_just_pressed(hw_button_t button);