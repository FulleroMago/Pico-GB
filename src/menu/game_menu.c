#include "game_menu.h"
#include "setting_menu.h"
#include "display/display.h"
#include "GUI/gui_common.h"
#include "controls.h"
#include "../main.h"

menu game_menu;

key_press_result game_menu_exit(struct menu_item *item, void *user_data)
{
    exit_to_selector = true;
    return KEY_PRESS_EXIT;
}

key_press_result game_menu_close(struct menu_item *item, void *user_data)
{
    display_clear(); // vyčistí obrazovku před návratem do hry, aby se předešlo zobrazení menu přes hru

    return KEY_PRESS_EXIT;
}

key_press_result game_menu_key_press(pad_button_t button, void *data)
{
    switch (button)
    {
    case MENU:
    case B:
        return game_menu_close(NULL, NULL); // zpracování akce pro tlačítko MENU, pokud je potřeba
        break;
    default:
        return KEY_PRESS_CONTINUE; // zpracování akce pro tlačítko B, pokud je potřeba
        break;
    }
}

key_press_result game_menu_open_setting_menu(struct menu_item *item, void *user_data)
{
    setting_menu_open();

    return KEY_PRESS_CONTINUE;
}

void game_menu_open()
{
    char fps_str[10]; // dostatečně velké pole pro uložení stringu "FPS: XX" včetně null terminátoru
    sprintf(fps_str, "FPS: %d", fps);

    menu_item menu_items[9] = {
        {.text = fps_str, .action = NULL, .value = NULL, .selectable = false}, // nezobrazí se jako volitelná položka menu, ale jen jako informace o aktuálním FPS, které se aktualizuje při každém otevření menu
        {.text = NULL, .action = NULL, .value = NULL, .selectable = false},    // prázdný řádek pro oddělení informací od nastavení
        {.text = "pokracovat ve hre", .action = game_menu_close, .value = NULL, .selectable = true},
        {.text = NULL, .action = NULL, .value = NULL, .selectable = false}, // prázdný řádek
        {.text = "ulozit", .action = NULL, .value = NULL, .selectable = true},
        {.text = "nacist", .action = NULL, .value = NULL, .selectable = true},
        {.text = "nastaveni", .action = game_menu_open_setting_menu, .value = NULL, .selectable = true},
        {.text = NULL, .action = NULL, .value = NULL, .selectable = false}, // prázdný řádek
        {.text = "ukoncit hru", .action = game_menu_exit, .value = NULL, .selectable = true},
    };

    game_menu = (menu){
        .box = {
            .position = {30, 30},
            .size = {DISPLAY_WIDTH - 60, DISPLAY_HEIGHT - 60}},
        .item_count = (sizeof(menu_items) / sizeof(menu_item)), // bude nastaveno níže podle skutečného počtu souborů
        .items = menu_items,
        .key_press = game_menu_key_press, // může být nastaveno pro zpracování specifických kláves v menu
        .selected = 2, // nastaví výchozí výběr na "pokracovat ve hre"
        .settings = {
            .text_color = display_get_color(255, 255, 255),      // bílá
            .text_bgcolor = display_get_color(0, 0, 255),        // modrá
            .text_select_color = display_get_color(255, 255, 0), // žlutá
            .border = 1,
            .border_color = display_get_color(255, 255, 255), // bílá
        }};

    gui_menu_show(&game_menu);
}