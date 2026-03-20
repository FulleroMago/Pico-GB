#pragma once
#include <stdbool.h>
#include "gui_common.h"

struct menu_item; // deklarace struktury menu_item pro použití v callbacku

// Callback typ pro akci položky menu
typedef key_press_result (*menu_item_action_cb)(struct menu_item *item, void *user_data);

typedef struct menu_item
{
    const char *text;           // text položky menu
    const char *description;    // popis položky menu
    bool selectable;            // zda je položka menu vyberatelná (může být použita pro zobrazení neaktivních položek)
    menu_item_action_cb action; // ukazatel na funkci, která se má vykonat po výběru položky
    void *value;                // volitelná hodnota položky menu, která může být použita v akci (např. pro nastavení hodnoty)
} menu_item;

