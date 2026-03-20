#include "setting_menu.h"
#include "game_menu.h"
#include "palette_menu.h"
#include "display/display.h"
#include "../main.h"

menu setting_menu;

key_press_result setting_menu_close(struct menu_item *item, void *user_data)
{
    settings_updated = true; // nastaví flag pro aktualizaci nastavení v hlavní smyčce

    if (!game_menu.exit_menu)
        gui_menu_draw(&game_menu); // překresli hlavní menu po zavření nastavení

    return KEY_PRESS_EXIT;
}

key_press_result setting_menu_key_press(pad_button_t button, void *data)
{
    switch (button)
    {
    case MENU:
        game_menu.exit_menu = true;            // nastaví flag pro opuštění hlavního menu, který bude zkontrolován v hlavní smyčce
        return setting_menu_close(NULL, NULL); // zpracování akce pro tlačítko MENU, pokud je potřeba
        break;
    case B:
        return setting_menu_close(NULL, NULL); // zpracování akce pro tlačítko B, pokud je potřeba
        break;
    default:
        return KEY_PRESS_CONTINUE; // zpracování akce pro tlačítko B, pokud je potřeba
        break;
    }
}

const char *setting_name_scaling_mode()
{
    const char *mode_name;
    
    if (current_settings.scaling_mode == NORMAL)
    {
        mode_name = "originalni";
    }
    else if (current_settings.scaling_mode == STRETCH)
    {
        mode_name = "cely displej";
    }
    else if (current_settings.scaling_mode == STRETCH_KEEP_ASPECT)
    {
        mode_name = "zachovat pomer stran";
    }
    else
    {
        mode_name = "neznama hodnota";
    }

    static char buffer[30];
    snprintf(buffer, sizeof(buffer), "rezim zobrazeni: %s", mode_name);

    return buffer;
}

key_press_result setting_scaling_mode(struct menu_item *item, void *user_data)
{
    current_settings.scaling_mode = (current_settings.scaling_mode + 1) % COUNT;
    item->text = setting_name_scaling_mode();
    settings_updated = true; // nastaví flag pro aktualizaci nastavení v hlavní smyčce

    gui_menu_draw(&setting_menu); // překresli menu pro aktualizaci zobrazení aktuálního nastavení

    return KEY_PRESS_CONTINUE;
}

const char *setting_name_frame_skip()
{
    const char *mode_name;

    if (current_settings.frame_skip)
    {
        mode_name = "zapnuto";
    }
    else
    {
        mode_name = "vypnuto";
    }

    static char buffer[20];
    snprintf(buffer, sizeof(buffer), "frame skip: %s", mode_name);
    return buffer;
}

key_press_result setting_frame_skip(struct menu_item *item, void *user_data)
{
    current_settings.frame_skip = !current_settings.frame_skip;
    item->text = setting_name_frame_skip();

    gui_menu_draw(&setting_menu); // překresli menu pro aktualizaci zobrazení aktuálního nastavení

    return KEY_PRESS_CONTINUE;
}

const char *setting_name_interlace()
{
    const char *mode_name;

    if (current_settings.interlace)
    {
        mode_name = "zapnuto";
    }
    else
    {
        mode_name = "vypnuto";
    }

    static char buffer[20];
    snprintf(buffer, sizeof(buffer), "interlace: %s", mode_name);

    return buffer;
}

key_press_result setting_interlace(struct menu_item *item, void *user_data)
{
    current_settings.interlace = !current_settings.interlace;
    item->text = setting_name_interlace();

    gui_menu_draw(&setting_menu); // překresli menu pro aktualizaci zobrazení aktuálního nastavení

    return KEY_PRESS_CONTINUE;
}

key_press_result setting_palette(struct menu_item *item, void *user_data)
{
    setting_palette_menu_open();

    return KEY_PRESS_CONTINUE;
}

void setting_menu_open()
{
    menu_item menu_items[6] = {
        {.text = setting_name_scaling_mode(), .action = setting_scaling_mode, .value = NULL, .selectable = true},
        {.text = setting_name_frame_skip(), .action = setting_frame_skip, .value = NULL, .selectable = true},
        {.text = setting_name_interlace(), .action = setting_interlace, .value = NULL, .selectable = true},
        {.text = "paleta", .action = setting_palette, .value = NULL, .selectable = true},
        {.text = NULL, .action = NULL, .value = NULL, .selectable = false}, // prázdný řádek
        {.text = "zpet", .action = setting_menu_close, .value = NULL, .selectable = true},
    };

    setting_menu = (menu){
        .box = {
            .position = {40, 40},
            .size = {DISPLAY_WIDTH - 80, DISPLAY_HEIGHT - 80}},
        .item_count = (sizeof(menu_items) / sizeof(menu_item)), // bude nastaveno níže podle skutečného počtu souborů
        .items = menu_items,
        .key_press = setting_menu_key_press, // může být nastaveno pro zpracování specifických kláves v menu
        .settings = {
            .text_color = display_get_color(255, 255, 255),      // bílá
            .text_bgcolor = display_get_color(0, 0, 255),        // modrá
            .text_select_color = display_get_color(255, 255, 0), // žlutá
            .border = 1,
            .border_color = display_get_color(255, 255, 255), // bílá
        }};

    gui_menu_show(&setting_menu);
}