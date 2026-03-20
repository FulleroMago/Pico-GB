#include "setting_menu.h"
#include "game_menu.h"
#include "display/display.h"
#include "../main.h"

menu setting_palette_menu;

key_press_result setting_palette_menu_close(struct menu_item *item, void *user_data)
{
    gui_menu_draw(&setting_menu); // překresli hlavní menu po zavření nastavení
    return KEY_PRESS_EXIT;
}

key_press_result setting_palette_menu_key_press(pad_button_t button, void *data)
{
    switch (button)
    {
    case B:
        return setting_palette_menu_close(NULL, NULL); // zpracování akce pro tlačítko B, pokud je potřeba
        break;
    default:
        return KEY_PRESS_CONTINUE; // zpracování akce pro tlačítko B, pokud je potřeba
        break;
    }
}

key_press_result setting_palette_type(struct menu_item *item, void *user_data)
{
    if ((intptr_t)item->value == -1)
    {
        current_settings.auto_assign_palette = 1;
    }
    else    
    {
        current_settings.auto_assign_palette = 0;
        current_settings.manual_palette = (intptr_t)item->value;
    }

    return setting_palette_menu_close(NULL, NULL); // zavře menu po výběru palety a vrátí se do nastavení
}

void setting_palette_menu_open()
{
    menu_item menu_items[] = {
        {.text = "automaticka", .action = setting_palette_type, .value = (void *)(intptr_t)-1, .selectable = true},
        {.text = "GB DMG zelena", .action = setting_palette_type, .value = (void *)(intptr_t)12, .selectable = true},
        {.text = "GB DMG monochrome", .action = setting_palette_type, .value = (void *)(intptr_t)4, .selectable = true},
        {.text = "paleta 1", .action = setting_palette_type, .value = (void *)(intptr_t)0, .selectable = true},
        {.text = "paleta 2", .action = setting_palette_type, .value = (void *)(intptr_t)1, .selectable = true},
        {.text = "paleta 3", .action = setting_palette_type, .value = (void *)(intptr_t)2, .selectable = true},
        {.text = "paleta 4", .action = setting_palette_type, .value = (void *)(intptr_t)3, .selectable = true},
        {.text = "paleta 5", .action = setting_palette_type, .value = (void *)(intptr_t)5, .selectable = true},
        {.text = "paleta 6", .action = setting_palette_type, .value = (void *)(intptr_t)6, .selectable = true},
        {.text = "paleta 7", .action = setting_palette_type, .value = (void *)(intptr_t)7, .selectable = true},
        {.text = "paleta 8", .action = setting_palette_type, .value = (void *)(intptr_t)8, .selectable = true},
        {.text = "paleta 9", .action = setting_palette_type, .value = (void *)(intptr_t)9, .selectable = true},
        {.text = "paleta 10", .action = setting_palette_type, .value = (void *)(intptr_t)10, .selectable = true},
        {.text = "paleta 11", .action = setting_palette_type, .value = (void *)(intptr_t)11, .selectable = true},

        {.text = NULL, .action = NULL, .value = NULL, .selectable = false}, // prázdný řádek
        {.text = "zpet", .action = setting_palette_menu_close, .value = NULL, .selectable = true},
    };

    uint8_t selected_index = 0;
    uint8_t item_count = sizeof(menu_items) / sizeof(menu_item);

    if(!current_settings.auto_assign_palette)
    {
        for (int i = 0; i < item_count; ++i)
        {
            if ((intptr_t)menu_items[i].value == current_settings.manual_palette)
            {
                selected_index = i;
                break;
            }
        }
    }

    setting_palette_menu = (menu){
        .box = {
            .position = {45, 45},
            .size = {DISPLAY_WIDTH - 90, DISPLAY_HEIGHT - 90}},
        .item_count = (sizeof(menu_items) / sizeof(menu_item)), // bude nastaveno níže podle skutečného počtu souborů
        .items = menu_items,
        .selected = selected_index, // nastaví výchozí výběr na aktuální nastavení palety
        .key_press = setting_palette_menu_key_press, // může být nastaveno pro zpracování specifických kláves v menu
        .settings = {
            .text_color = display_get_color(255, 255, 255),      // bílá
            .text_bgcolor = display_get_color(0, 0, 255),        // modrá
            .text_select_color = display_get_color(255, 255, 0), // žlutá
            .border = 1,
            .border_color = display_get_color(255, 255, 255), // bílá
        }};

    gui_menu_show(&setting_palette_menu);
}