#include "buttons.h"
#include "controls.h"
#include <pico/stdlib.h>
#include <hardware/pio.h>

void controls_init() {
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
}

void controls_update() {
    // Update button states
}

bool is_button_pressed(hw_button_t button)
{
    return !gpio_get(button);
}