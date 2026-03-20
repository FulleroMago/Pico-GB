#include "main.h"
#include "rom_selector.h"
#include <pico/stdio.h>
#include <stdio.h>
#include "morana.h"
#include "GUI/menu.h"
#include "GUI/gui_common.h"
#include "config.h"
#include <string.h>

char rom_files[FILES_PER_PAGE][64];
menu_item menu_items[FILES_PER_PAGE];

void loading_cart_to_flash_block(const void *block, uint32_t block_size, void *user_data)
{
    flash_write_state_t *state = (flash_write_state_t *)user_data;

    printf("I Erasing target region...\n");
    flash_range_erase(state->flash_target_offset, block_size);
    printf("I Programming target region...\n");
    flash_range_program(state->flash_target_offset, block, block_size);

    /* Read back target region and check programming */
    printf("I Done. Reading back target region...\n");
    for (uint32_t i = 0; i < block_size; i++)
    {
        if (rom[state->flash_target_offset + i] != ((uint8_t *)block)[i])
        {
            state->mismatch = true;
        }
    }

    /* Next sector */
    state->flash_target_offset += FLASH_SECTOR_SIZE;
}

/**
 * Load a .gb rom file in flash from the SD card
 */
void load_cart_rom_file(char *filename)
{
    flash_write_state_t state = {
        .flash_target_offset = FLASH_TARGET_OFFSET,
        .mismatch = false};

    if (!sd_read_file_sequential(filename, FLASH_SECTOR_SIZE, loading_cart_to_flash_block, (void *)&state))
    {
        printf("E Failed to read file %s from SD card.\n", filename);
        return;
    }

    if (state.mismatch)
    {
        strcpy(rom_title, filename);
        printf("I Programming successful!\n");
    }
    else
    {
        printf("E Programming failed!\n");
    }
}

key_press_result load_selected_rom(menu_item *item, void *user_data)
{
    printf("Selected ROM: %s\n", (char *)item->value);
    load_cart_rom_file((char *)item->value);
    return KEY_PRESS_EXIT;
}

void rom_selector_clear_menu_items()
{
    for (uint8_t i = 0; i < FILES_PER_PAGE; i++)
    {
        menu_items[i].text = NULL;
        menu_items[i].action = NULL;
        menu_items[i].value = NULL;
    }
}

// Funkce pro naplnění menu položkami s ROMkami z aktuální stránky
uint8_t rom_selector_show_page(uint8_t page, void *user_data)
{
    // /* clear the filenames array */
    for (uint8_t ifile = 0; ifile < FILES_PER_PAGE; ifile++)
    {
        strcpy(rom_files[ifile], "");
    }

    /* search *.gb files */
    int files_to_skip = page * FILES_PER_PAGE;

    sd_entry_t entries[FILES_PER_PAGE];
    int found = sd_list_directory_pattern("", "*.gb", entries, files_to_skip, FILES_PER_PAGE);

    if(found == 0) {
        return 0;
    }

    // clear menu items
    rom_selector_clear_menu_items();

    for (int i = 0; i < found; i++)
    {
        strcpy(rom_files[i], entries[i].name);
        menu_items[i].text = rom_files[i];
        menu_items[i].value = rom_files[i];
        menu_items[i].action = load_selected_rom;
        menu_items[i].selectable = true;
    }

    return found;
}

void rom_selector()
{
    menu m = {
        .box = {
            .position = {5, 5},
            .size = {DISPLAY_WIDTH - 10, DISPLAY_HEIGHT - 10}},
        .item_count = 0, // bude nastaveno níže podle skutečného počtu souborů
        .items = menu_items,
        .settings = {
            .text_color = display_get_color(255, 255, 255),      // bílá
            .text_bgcolor = display_get_color(0, 0, 0),          // černá
            .text_select_color = display_get_color(255, 255, 0), // žlutá
            .border = 1,
            .border_color = display_get_color(0, 0, 255), // modrá
        },
        .load_items = rom_selector_show_page};

    gui_menu_show(&m);
}