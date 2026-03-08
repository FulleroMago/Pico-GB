#include "controls.h"
#include "config.h"
#include <pico/stdlib.h>
#include <stdio.h>

static uint16_t button_states = 0;
static uint16_t button_states_prev = 0;

#define PAD_A 0x01
#define PAD_B 0x02
#define PAD_X 0x04
#define PAD_Y 0x08
#define PAD_RIGHT 0x10
#define PAD_LEFT 0x20
#define PAD_UP 0x40
#define PAD_DOWN 0x80

#define PAD_SELECT 0x100
#define PAD_START 0x200
#define PAD_MENU 0x400

void controls_init()
{
    // Initialize button GPIO pins

    /* Initialise GPIO pins. */
    gpio_set_function(GPIO_CONTROL_UP, GPIO_FUNC_SIO);
    gpio_set_function(GPIO_CONTROL_DOWN, GPIO_FUNC_SIO);
    gpio_set_function(GPIO_CONTROL_LEFT, GPIO_FUNC_SIO);
    gpio_set_function(GPIO_CONTROL_RIGHT, GPIO_FUNC_SIO);

    gpio_set_function(GPIO_CONTROL_A, GPIO_FUNC_SIO);
    gpio_set_function(GPIO_CONTROL_B, GPIO_FUNC_SIO);
    gpio_set_function(GPIO_CONTROL_X, GPIO_FUNC_SIO);
    gpio_set_function(GPIO_CONTROL_Y, GPIO_FUNC_SIO);

    gpio_set_function(GPIO_CONTROL_SELECT, GPIO_FUNC_SIO);
    gpio_set_function(GPIO_CONTROL_START, GPIO_FUNC_SIO);
    gpio_set_function(GPIO_CONTROL_MENU, GPIO_FUNC_SIO);

    gpio_set_dir(GPIO_CONTROL_UP, false);
    gpio_set_dir(GPIO_CONTROL_DOWN, false);
    gpio_set_dir(GPIO_CONTROL_LEFT, false);
    gpio_set_dir(GPIO_CONTROL_RIGHT, false);

    gpio_set_dir(GPIO_CONTROL_A, false);
    gpio_set_dir(GPIO_CONTROL_B, false);
    gpio_set_dir(GPIO_CONTROL_X, false);
    gpio_set_dir(GPIO_CONTROL_Y, false);

    gpio_set_dir(GPIO_CONTROL_SELECT, false);
    gpio_set_dir(GPIO_CONTROL_START, false);
    gpio_set_dir(GPIO_CONTROL_MENU, false);

    gpio_pull_up(GPIO_CONTROL_UP);
    gpio_pull_up(GPIO_CONTROL_DOWN);
    gpio_pull_up(GPIO_CONTROL_LEFT);
    gpio_pull_up(GPIO_CONTROL_RIGHT);

    gpio_pull_up(GPIO_CONTROL_A);
    gpio_pull_up(GPIO_CONTROL_B);
    gpio_pull_up(GPIO_CONTROL_X);
    gpio_pull_up(GPIO_CONTROL_Y);

    gpio_pull_up(GPIO_CONTROL_SELECT);
    gpio_pull_up(GPIO_CONTROL_START);
    gpio_pull_up(GPIO_CONTROL_MENU);

    button_states = 0;
    button_states_prev = 0;
}

void controls_update()
{
    button_states_prev = button_states;
    button_states = 0;

    if (!gpio_get(GPIO_CONTROL_UP))
        button_states |= PAD_UP;
    if (!gpio_get(GPIO_CONTROL_DOWN))
        button_states |= PAD_DOWN;
    if (!gpio_get(GPIO_CONTROL_LEFT))
        button_states |= PAD_LEFT;
    if (!gpio_get(GPIO_CONTROL_RIGHT))
        button_states |= PAD_RIGHT;

    if (!gpio_get(GPIO_CONTROL_A))
        button_states |= PAD_A;
    if (!gpio_get(GPIO_CONTROL_B))
        button_states |= PAD_B;
    if (!gpio_get(GPIO_CONTROL_X))
        button_states |= PAD_X;
    if (!gpio_get(GPIO_CONTROL_Y))
        button_states |= PAD_Y;

    if (!gpio_get(GPIO_CONTROL_SELECT))
        button_states |= PAD_SELECT;
    if (!gpio_get(GPIO_CONTROL_START))
        button_states |= PAD_START;
    if (!gpio_get(GPIO_CONTROL_MENU))
        button_states |= PAD_MENU;
}

bool controls_is_button_pressed(pad_button_t button)
{
    if(button < 0 || button > MENU) {
        return false; // Invalid button
    }

    switch (button)
    {
        case UP:
            return (button_states & PAD_UP) != 0;
            break;
        case DOWN:
            return (button_states & PAD_DOWN) != 0;
            break;
        case LEFT:
            return (button_states & PAD_LEFT) != 0;
            break;
        case RIGHT:
            return (button_states & PAD_RIGHT) != 0;
            break;
        case A:
            return (button_states & PAD_A) != 0;
            break;
        case B:
            return (button_states & PAD_B) != 0;
            break;
        case X:
            return (button_states & PAD_X) != 0;
            break;
        case Y:
            return (button_states & PAD_Y) != 0;
            break;
        case START:
            return (button_states & PAD_START) != 0;
            break;
        case SELECT:
            return (button_states & PAD_SELECT) != 0;
            break;
        case MENU:
            return (button_states & PAD_MENU) != 0;
            break;
        default:
            return false;
            break;
    }
}

bool controls_was_button_just_pressed(pad_button_t button)
{
    if(button < 0 || button > MENU) {
        return false; // Invalid button
    }

    uint16_t mask = 0;
    switch (button)
    {        case UP:
            mask = PAD_UP;
            break;
        case DOWN:
            mask = PAD_DOWN;
            break;
        case LEFT:
            mask = PAD_LEFT;
            break;
        case RIGHT:
            mask = PAD_RIGHT;
            break;
        case A:
            mask = PAD_A;
            break;
        case B:
            mask = PAD_B;
            break;
        case X:
            mask = PAD_X;
            break;
        case Y:
            mask = PAD_Y;
            break;
        case START:
            mask = PAD_START;
            break;
        case SELECT:
            mask = PAD_SELECT;
            break;
        case MENU:
            mask = PAD_MENU;
            break;
        default:
            return false;
            break;
    }
    
    return ((button_states & mask) != 0) && ((button_states_prev & mask) == 0);
}