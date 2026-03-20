#pragma once

#include "types.h"
#include "menu_item.h"
#include "config.h"
#include <stddef.h>
#include "gui_common.h"
#include "controls.h"

// Callback typ pro načtení položek menu
typedef uint8_t (*menu_load_items_cb)(uint8_t page, void *user_data);
typedef key_press_result (*menu_key_press_cb)(pad_button_t button, void *user_data);

typedef struct
{
    uint16_t text_color;        // barva textu
    uint16_t text_bgcolor;      // barva pozadí textu
    uint16_t text_select_color; // barva pozadí vybraného textu
    uint8_t border;             // zda zobrazovat rámeček kolem menu
    uint16_t border_color;      // barva rámečku

} menu_settings;

inline const menu_settings get_default_menu_settings() {
    return (menu_settings){
        .text_color = MENU_TEXT_COLOR,
        .text_bgcolor = MENU_BG_COLOR,
        .text_select_color = MENU_HIGHLIGHT_COLOR,
        .border = 1,
        .border_color = MENU_BORDER_COLOR
    };
}

typedef struct
{
    box box;
    menu_item *items;
    size_t item_count;
    uint8_t actual_page;
    uint8_t selected;
    menu_load_items_cb load_items; // Callback pro načtení položek menu
    menu_key_press_cb key_press;   // Callback pro zpracování stisků kláves v menu
    menu_settings settings; // Nastavení zobrazení menu
    bool exit_menu;         // Flag pro indikaci, zda bylo požadováno opuštění menu (může být nastaveno v callbacku pro zpracování kláves)
} menu;

void gui_menu_show(menu *m);
void gui_menu_draw(menu *m);