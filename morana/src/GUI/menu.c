#include "menu.h"
#include "display/display.h"
#include "display/display_txt.h"
#include "display/display_shape.h"
#include "controls.h"
#include "config.h"

void gui_menu_show(menu *m)
{
    if(m->load_items) {
        m->item_count = m->load_items(0, NULL); // načti položky menu pomocí callbacku
    }

    menu_item *selected_item = &m->items[m->selected];

    display_txt_set_cursor(m->selected, 0);
    gui_menu_draw(m);

    while (true)
    {
        controls_update();
        display_txt_set_cursor(m->selected, 0);

        if (controls_is_button_pressed(A))
        {
            if (selected_item->action)
            {
                uint8_t result = selected_item->action(selected_item, selected_item->value);
                if (result)
                    break; // vyskok z menu, pokud akce vrátí nenulovou hodnotu
                else
                    sleep_ms(MENU_SELECT_DELAY_MS); // malý delay pro zamezení vícenásobného spuštění akce při podržení tlačítka
            }
        }
        if (controls_is_button_pressed(DOWN))
        {
            display_txt_write_line_color(selected_item->text, m->settings.text_color, m->settings.text_bgcolor);
            m->selected++;
            if (m->selected >= m->item_count)
                m->selected = 0;
            selected_item = &m->items[m->selected];
            display_txt_set_cursor(m->selected, 0);
            display_txt_write_line_color(selected_item->text, m->settings.text_select_color, m->settings.text_bgcolor);
            sleep_ms(MENU_SCROLL_DELAY_MS);
        }
        if (controls_is_button_pressed(UP))
        {
            /* select the previous rom */
            display_txt_write_line_color(selected_item->text, m->settings.text_color, m->settings.text_bgcolor);
            if (m->selected == 0)
            {
                m->selected = m->item_count - 1;
            }
            else
            {
                m->selected--;
            }

            selected_item = &m->items[m->selected];
            display_txt_set_cursor(m->selected, 0);
            display_txt_write_line_color(selected_item->text, m->settings.text_select_color, m->settings.text_bgcolor);
            sleep_ms(MENU_SCROLL_DELAY_MS);
        }
        if (controls_is_button_pressed(RIGHT))
        {
           if(m->load_items) {
                uint8_t result = m->load_items(m->actual_page + 1, NULL);
                if(result > 0) {
                    m->item_count = result; // aktualizuj počet položek menu podle načtených dat
                    m->actual_page++; // aktualizuj aktuální stránku menu
                    m->selected = 0; // resetuj výběr na první položku nové stránky
                    selected_item = &m->items[m->selected];
                    display_txt_set_cursor(m->selected, 0);
                    gui_menu_draw(m); // překresli menu s novými položkami
                }

                sleep_ms(MENU_SCROLL_DELAY_MS);
            }
        }
        if (controls_is_button_pressed(LEFT))
        {
            if(m->load_items) {
                uint8_t result = m->load_items(m->actual_page - 1, NULL);
                if(result > 0) {
                    m->item_count = result; // aktualizuj počet položek menu podle načtených dat
                    m->actual_page--; // aktualizuj aktuální stránku menu
                    m->selected = 0; // resetuj výběr na první položku nové stránky
                    selected_item = &m->items[m->selected];
                    display_txt_set_cursor(m->selected, 0);
                    gui_menu_draw(m); // překresli menu s novými položkami
                }

                sleep_ms(MENU_SCROLL_DELAY_MS);
            }
        }

        tight_loop_contents();
    }
}

void gui_menu_draw(menu *m)
{
    text_box menu_text_box = {
        .box = {
            .position = {m->box.position.x + 5, m->box.position.y + 5},
            .size = {m->box.size.width - 10, m->box.size.height - 10}},
        .line_margin = 1,
        .text_color = m->settings.text_color,
        .text_bgcolor = m->settings.text_bgcolor,
        .line = 0,
        .column = 0,
        .wrap = DISPLAY_TXT_WRAP_NONE};

    display_txt_set_text_box(&menu_text_box);
    display_shape_rect(m->box.position.x, m->box.position.y, m->box.size.width, m->box.size.height, m->settings.text_bgcolor, true);

    if (m->settings.border)
    {
        display_shape_rect(m->box.position.x, m->box.position.y, m->box.size.width, m->box.size.height, m->settings.border_color, false);
    }

    for (size_t i = 0; i < m->item_count; i++)
    {
        if(m->selected == i)
        {
            display_txt_write_line_color(m->items[i].text, m->settings.text_select_color, m->settings.text_bgcolor);
        }
        else
        {
            display_txt_write_line_color(m->items[i].text, m->settings.text_color, m->settings.text_bgcolor);
        }
    }
}