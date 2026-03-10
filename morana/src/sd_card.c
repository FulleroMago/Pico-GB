#include <stdio.h>
#include "pico/stdlib.h"
#include "sd_card.h"
#include "ff.h"
#include "f_util.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "sd_hw_config.h"

bool sd_card_init()
{
    // Initialize SD card
    puts("Initializing SD card...");

    // See FatFs - Generic FAT Filesystem Module, "Application Interface",
    // http://elm-chan.org/fsw/ff/00index_e.html

    // sd_card_t *sd_card = sd_get_by_num(0);
    // if (sd_card == NULL)
    // {
    //     printf("Error: No SD card found.\n");
    //     return false;
    // }

    return true;
}

bool sd_mount()
{
    sd_card_t *pSD = sd_get_by_num(0);
    FRESULT fr = f_mount(&pSD->fatfs, pSD->pcName, 1);

    if (FR_OK != fr)
    {
        printf("f_mount error: (%d)\n", fr);
        return false;
    }

    return true;
}

bool sd_unmount()
{
    sd_card_t *pSD = sd_get_by_num(0);
    FRESULT fr = f_unmount(pSD->pcName);

    if (FR_OK != fr)
    {
        printf("f_unmount error: (%d)\n", fr);
        return false;
    }

    return true;
}

int32_t sd_get_file_size(const char *filename)
{
    sd_mount();

    FILINFO fno;
    FRESULT fr = f_stat(filename, &fno);
    if (FR_OK != fr)
    {
        sd_unmount();
        printf("f_stat error: (%d)\n", fr);
        return -1;
    }

    sd_unmount();

    return fno.fsize;
}

bool sd_read_file(const char *filename, void *buffer, uint32_t buffer_size)
{
    sd_mount();

    FIL fil;
    FRESULT fr = f_open(&fil, filename, FA_OPEN_EXISTING | FA_READ);
    if (FR_OK != fr)
    {
        sd_unmount();
        printf("f_open error: (%d)\n", fr);
        return false;
    }

    int bytes_read;
    fr = f_read(&fil, buffer, buffer_size, &bytes_read);
    if (fr != FR_OK)
    {
        sd_unmount();
        printf("E f_read error: %s (%d)\n", FRESULT_str(fr), fr);
        f_close(&fil);
        return false;
    }

    // Close the file
    fr = f_close(&fil);
    if (FR_OK != fr)
    {
        sd_unmount();
        printf("f_close error: (%d)\n", fr);
        return false;
    }

    sd_unmount();

    return true;
}

bool sd_write_file(const char *filename, const void *data, uint32_t data_size)
{
    sd_mount();

    FIL fil;
    FRESULT fr = f_open(&fil, filename, FA_OPEN_APPEND | FA_WRITE);
    if (FR_OK != fr && FR_EXIST != fr)
    {
        sd_unmount();
        printf("f_open(%s) error: (%d)\n", filename, fr);
        // panic("f_open(%s) error: %s (%d)\n", filename, FRESULT_str(fr), fr);
        return false;
    }

    int bytes_written;
    fr = f_write(&fil, data, data_size, &bytes_written);
    if (fr != FR_OK || bytes_written != data_size)
    {
        sd_unmount();
        printf("E f_write error: %s (%d)\n", FRESULT_str(fr), fr);
        f_close(&fil);
        return false;
    }

    // Close the file
    fr = f_close(&fil);
    if (FR_OK != fr)
    {
        sd_unmount();
        printf("f_close error: (%d)\n", fr);
        return false;
    }

    sd_unmount();

    return true;
}

bool sd_file_exists(const char *filename)
{
    sd_mount();

    FILINFO fno;
    FRESULT fr = f_stat(filename, &fno);
    if (FR_OK == fr)
    {
        sd_unmount();
        return true;
    }
    else if (FR_NO_FILE == fr)
    {
        sd_unmount();
        return false;
    }
    else
    {
        sd_unmount();
        printf("f_stat error: (%d)\n", fr);
        return false;
    }

    sd_unmount();
}

bool sd_delete_file(const char *filename)
{
    sd_mount();

    FRESULT fr = f_unlink(filename);
    if (FR_OK != fr)
    {
        sd_unmount();
        printf("f_unlink error: (%d)\n", fr);
        return false;
    }

    sd_unmount();

    return true;
}

bool sd_rename_file(const char *old_filename, const char *new_filename)
{
    sd_mount();

    FRESULT fr = f_rename(old_filename, new_filename);
    if (FR_OK != fr)
    {
        sd_unmount();
        printf("f_rename error: (%d)\n", fr);
        return false;
    }

    sd_unmount();

    return true;
}

bool sd_create_directory(const char *dirname)
{
    sd_mount();

    FRESULT fr = f_mkdir(dirname);
    if (FR_OK != fr)
    {
        sd_unmount();
        printf("f_mkdir error: (%d)\n", fr);
        return false;
    }

    sd_unmount();

    return true;
}

bool sd_directory_exists(const char *dirname)
{
    sd_mount();

    FILINFO fno;
    FRESULT fr = f_stat(dirname, &fno);
    if (FR_OK == fr)
    {
        sd_unmount();
        return (fno.fattrib & AM_DIR) != 0;
    }
    else if (FR_NO_FILE == fr)
    {
        sd_unmount();
        return false;
    }
    else
    {
        sd_unmount();
        printf("f_stat error: (%d)\n", fr);
        return false;
    }
}

bool sd_delete_directory(const char *dirname)
{
    sd_mount();

    FRESULT fr = f_unlink(dirname);
    if (FR_OK != fr)
    {
        sd_unmount();
        printf("f_unlink error: (%d)\n", fr);
        return false;
    }

    sd_unmount();

    return true;
}

bool sd_rename_directory(const char *old_dirname, const char *new_dirname)
{
    sd_mount();

    FRESULT fr = f_rename(old_dirname, new_dirname);
    if (FR_OK != fr)
    {
        sd_unmount();
        printf("f_rename error: (%d)\n", fr);
        return false;
    }

    sd_unmount();
    return true;
}

uint8_t sd_list_directory_cb(const char *dirname, sd_entry_cb cb, void *user_data)
{
    DIR dir;
    FILINFO fno;
    uint8_t count = 0;

    sd_mount();

    FRESULT fr = f_opendir(&dir, dirname);
    if (FR_OK != fr)
    {
        sd_unmount();
        printf("f_opendir error: (%d)\n", fr);
        return -1;
    }

    while (true)
    {
        fr = f_readdir(&dir, &fno);
        if (fr != FR_OK || fno.fname[0] == 0)
            break;
        cb(&fno, user_data);
        count++;
    }

    f_closedir(&dir);

    sd_unmount();

    return count;
}

uint8_t sd_list_directory(const char *dirname, sd_entry_t *entries, uint8_t max_entries)
{
    DIR dir;
    FILINFO fno;
    uint8_t count = 0;

    sd_mount();

    FRESULT fr = f_opendir(&dir, dirname);
    if (FR_OK != fr)
    {
        sd_unmount();
        printf("f_opendir error: (%d)\n", fr);
        return -1;
    }

    while (count < max_entries)
    {
        fr = f_readdir(&dir, &fno);
        if (fr != FR_OK || fno.fname[0] == 0)
            break;
        strncpy(entries[count].name, fno.fname, sizeof(entries[count].name) - 1);
        entries[count].name[sizeof(entries[count].name) - 1] = '\0';
        entries[count].is_dir = (fno.fattrib & AM_DIR) ? 1 : 0;
        count++;
    }

    f_closedir(&dir);

    sd_unmount();

    return count;
}

uint8_t sd_list_directory_pattern(const char *dirname, const char *pattern, sd_entry_t *entries, uint8_t skip_entries, uint8_t max_entries)
{
    sd_mount();

    DIR dir;
    FILINFO fno;
    uint8_t count = 0;

    FRESULT fr = f_findfirst(&dir, &fno, dirname, pattern);

    // Přeskočit první N položek
    if (skip_entries > 0)
    {
        while (count < skip_entries && fr == FR_OK && fno.fname[0])
        {
            count++;
            fr = f_findnext(&dir, &fno);
        }
    }

    // Uložit položky do pole s filtrováním podle vzoru a omezením počtu
    count = 0;
    while (count < max_entries && fr == FR_OK && fno.fname[0])
    {
        strncpy(entries[count].name, fno.fname, sizeof(entries[count].name) - 1);
        entries[count].name[sizeof(entries[count].name) - 1] = '\0';
        entries[count].is_dir = (fno.fattrib & AM_DIR) ? 1 : 0;

        count++;
        fr = f_findnext(&dir, &fno);
    }

    f_closedir(&dir);
    sd_unmount();

    return count;
}

bool sd_read_file_sequential(const char *filename, uint32_t block_size, sd_read_block_cb cb, void *user_data)
{
    bool result = true;

    sd_mount();

    FIL fil;
    FRESULT fr = f_open(&fil, filename, FA_READ);
    if (FR_OK != fr)
    {
        sd_unmount();
        printf("f_open error: (%d)\n", fr);
        return false;
    }

    void *buffer = malloc(block_size);
    if (!buffer)
    {
        printf("malloc error\n");
        f_close(&fil);
        sd_unmount();
        return false;
    }

    int bytes_read;

    while (1)
    {
        fr = f_read(&fil, buffer, block_size, &bytes_read);
        if (fr != FR_OK)
        {
            printf("E f_read error: %s (%d)\n", FRESULT_str(fr), fr);
            result = false;
            break;
        }
        if (bytes_read == 0)
            break; // EOF

        cb(buffer, bytes_read, user_data);

        if (bytes_read < block_size)
            break; // poslední blok
    }

    free(buffer);

    fr = f_close(&fil);
    if (FR_OK != fr)
    {
        sd_unmount();
        printf("f_close error: (%d)\n", fr);
        result = false;
    }

    sd_unmount();

    return result;
}

// bool sd_find_file(const char *dirname, const char *pattern, sd_entry_t *entry)
// {
//     DIR dir;
//     FILINFO fno;
//     FRESULT fr = f_findfirst(&dir, &fno, dirname, pattern);
//     if (fr != FR_OK) {
//         printf("f_findfirst error: (%d)\n", fr);
//         return false;
//     }

//     bool found = false;
//     if (fno.fname[0] != 0) {
//         strncpy(entry->name, fno.fname, sizeof(entry->name) - 1);
//         entry->name[sizeof(entry->name) - 1] = '\0';
//         entry->is_dir = (fno.fattrib & AM_DIR) ? 1 : 0;
//         found = true;
//     }

//     f_findclose(&dir);
//     return found;
// }

void sd_deinit()
{
    // Unmount the SD card
    f_unmount("");
}