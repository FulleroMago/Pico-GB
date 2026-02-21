#pragma once

#include <stdbool.h>
#include <stdint.h>
#include "ff.h"

// Struktura pro položku adresáře
typedef struct
{
    char name[64];
    uint8_t is_dir;
} sd_entry_t;

// Callback typ pro iteraci adresářem
typedef void (*sd_entry_cb)(const FILINFO *fno, void *user_data);

// Inicializace SD karty
bool sd_card_init(void);

// Získání velikosti souboru
int32_t sd_get_file_size(const char *filename);

// Čtení souboru do bufferu
bool sd_read_file(const char *filename, void *buffer, uint32_t buffer_size);

// Zápis dat do souboru (append)
bool sd_write_file(const char *filename, const void *data, uint32_t data_size);

// Kontrola existence souboru
bool sd_file_exists(const char *filename);

// Smazání souboru
bool sd_delete_file(const char *filename);

// Přejmenování souboru
bool sd_rename_file(const char *old_filename, const char *new_filename);

// Vytvoření adresáře
bool sd_create_directory(const char *dirname);

// Kontrola existence adresáře
bool sd_directory_exists(const char *dirname);

// Smazání adresáře
bool sd_delete_directory(const char *dirname);

// Přejmenování adresáře
bool sd_rename_directory(const char *old_dirname, const char *new_dirname);

// Iterace adresářem pomocí callbacku
int sd_list_directory_cb(const char *dirname, sd_entry_cb cb, void *user_data);

// Získání seznamu položek adresáře do pole
int sd_list_directory(const char *dirname, sd_entry_t *entries, int max_entries);

// Získání seznamu položek adresáře do pole s filtrováním podle vzoru
int sd_list_directory_pattern(const char *dirname, const char *pattern, sd_entry_t *entries, int skip_entries, int max_entries);

// Deinicializace SD karty
void sd_deinit(void);