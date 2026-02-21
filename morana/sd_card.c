#include <stdio.h>
#include "pico/stdlib.h"
#include "sd_card.h"
#include "ff.h"
#include "f_util.h"
#include <stdio.h>
#include <string.h>

static FATFS fs;

bool sd_card_init()
{
    // Initialize SD card
    puts("Initializing SD card...");

    // See FatFs - Generic FAT Filesystem Module, "Application Interface",
    // http://elm-chan.org/fsw/ff/00index_e.html

    FRESULT fr = f_mount(&fs, "", 1);
    if (FR_OK != fr)
    {
        printf("f_mount error: (%d)\n", fr);
        // panic("f_mount error: %s (%d)\n", FRESULT_str(fr), fr);
    }

    // sd_card_t *sd_card = sd_get_by_num(0);
    // if (sd_card == NULL)
    // {
    //     printf("Error: No SD card found.\n");
    //     return false;
    // }

    return true;
}

int32_t sd_get_file_size(const char *filename)
{
    FILINFO fno;
    FRESULT fr = f_stat(filename, &fno);
    if (FR_OK != fr)    {
        printf("f_stat error: (%d)\n", fr);
        return -1;
    }

    return fno.fsize;
}

bool sd_read_file(const char *filename, void *buffer, uint32_t buffer_size)
{
    FIL fil;
    FRESULT fr = f_open(&fil, filename, FA_OPEN_EXISTING | FA_READ);
    if (FR_OK != fr)
    {
        printf("f_open error: (%d)\n", fr);
        return false;
    }

    UINT bytes_read;
    fr = f_read(&fil, buffer, buffer_size, &bytes_read);
    if (fr != FR_OK)
    {
        printf("E f_read error: %s (%d)\n", FRESULT_str(fr), fr);
        f_close(&fil);
        return false;
    }

    // Close the file
    fr = f_close(&fil);
    if (FR_OK != fr)
    {
        printf("f_close error: (%d)\n", fr);
        return false;
    }

    return true;
}

bool sd_write_file(const char *filename, const void *data, uint32_t data_size)
{
    FIL fil;
    FRESULT fr = f_open(&fil, filename, FA_OPEN_APPEND | FA_WRITE);
    if (FR_OK != fr && FR_EXIST != fr)
    {
        printf("f_open(%s) error: (%d)\n", filename, fr);
        // panic("f_open(%s) error: %s (%d)\n", filename, FRESULT_str(fr), fr);
        return false;
    }

    UINT bytes_written;
    fr = f_write(&fil, data, data_size, &bytes_written);
    if (fr != FR_OK || bytes_written != data_size)
    {
        printf("E f_write error: %s (%d)\n", FRESULT_str(fr), fr);
        f_close(&fil);
        return false;
    }

    // Close the file
    fr = f_close(&fil);
    if (FR_OK != fr)
    {
        printf("f_close error: (%d)\n", fr);
        return false;
    }

    return true;
}

bool sd_file_exists(const char *filename)
{
    FILINFO fno;
    FRESULT fr = f_stat(filename, &fno);
    if (FR_OK == fr)
        return true;
    else if (FR_NO_FILE == fr)
        return false;
    else
    {
        printf("f_stat error: (%d)\n", fr);
        return false;
    }
}

bool sd_delete_file(const char *filename)
{
    FRESULT fr = f_unlink(filename);
    if (FR_OK != fr)    {
        printf("f_unlink error: (%d)\n", fr);
        return false;
    }
    return true;
}

bool sd_rename_file(const char *old_filename, const char *new_filename)
{
    FRESULT fr = f_rename(old_filename, new_filename);
    if (FR_OK != fr)    {
        printf("f_rename error: (%d)\n", fr);
        return false;
    }
    return true;
}

bool sd_create_directory(const char *dirname)
{
    FRESULT fr = f_mkdir(dirname);
    if (FR_OK != fr)    {
        printf("f_mkdir error: (%d)\n", fr);
        return false;
    }
    return true;
}

bool sd_directory_exists(const char *dirname)
{
    FILINFO fno;
    FRESULT fr = f_stat(dirname, &fno);
    if (FR_OK == fr)
        return (fno.fattrib & AM_DIR) != 0;
    else if (FR_NO_FILE == fr)
        return false;
    else
    {
        printf("f_stat error: (%d)\n", fr);
        return false;
    }
}

bool sd_delete_directory(const char *dirname)
{
    FRESULT fr = f_unlink(dirname);
    if (FR_OK != fr)    {
        printf("f_unlink error: (%d)\n", fr);
        return false;
    }
    return true;
}

bool sd_rename_directory(const char *old_dirname, const char *new_dirname)
{
    FRESULT fr = f_rename(old_dirname, new_dirname);
    if (FR_OK != fr)    {
        printf("f_rename error: (%d)\n", fr);
        return false;
    }
    return true;
}

int sd_list_directory_cb(const char *dirname, sd_entry_cb cb, void *user_data)
{
    DIR dir;
    FILINFO fno;
    int count = 0;

    FRESULT fr = f_opendir(&dir, dirname);
    if (FR_OK != fr) {
        printf("f_opendir error: (%d)\n", fr);
        return -1;
    }

    while (true) {
        fr = f_readdir(&dir, &fno);
        if (fr != FR_OK || fno.fname[0] == 0)
            break;
        cb(&fno, user_data);
        count++;
    }

    f_closedir(&dir);
    return count;
}

int sd_list_directory(const char *dirname, sd_entry_t *entries, int max_entries)
{
    DIR dir;
    FILINFO fno;
    int count = 0;

    FRESULT fr = f_opendir(&dir, dirname);
    if (FR_OK != fr)
    {
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
    return count;
}

int sd_list_directory_pattern(const char *dirname, const char *pattern, sd_entry_t *entries, int skip_entries, int max_entries)
{
    DIR dir;
    FILINFO fno;
    int count = 0;

    FRESULT fr = f_findfirst(&dir, &fno, dirname, pattern);

    // Přeskočit první N položek
    if (skip_entries > 0)
    {
        while (count < skip_entries && fr == FR_OK && fno.fname[0])
        {
            count++;
    		fr=f_findnext(&dir, &fno);
        }
    }

    // Uložit položky do pole s filtrováním podle vzoru a omezením počtu
    count = 0;
    while(count < max_entries && fr == FR_OK && fno.fname[0]) {
        strncpy(entries[count].name, fno.fname, sizeof(entries[count].name) - 1);
        entries[count].name[sizeof(entries[count].name) - 1] = '\0';
        entries[count].is_dir = (fno.fattrib & AM_DIR) ? 1 : 0;
        
        count++;
        fr=f_findnext(&dir, &fno);
    }

    f_closedir(&dir);
    return count;
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