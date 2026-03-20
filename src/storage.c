#include "storage.h"

#define PEANUT_GB_HEADER_ONLY
#include "peanut_gb.h"

/**
 * Load a save file from the SD card
 */
void storage_read_cart_ram_file(struct gb_s *gb)
{
    // char filename[16];
    // uint_fast32_t save_size;
    // UINT br;

    // gb_get_rom_name(gb, filename);
    // save_size = gb_get_save_size(gb);
    // if (save_size > 0)
    // {
    //     sd_card_t *pSD = sd_get_by_num(0);
    //     FRESULT fr = f_mount(&pSD->fatfs, pSD->pcName, 1);
    //     if (FR_OK != fr)
    //     {
    //         printf("E f_mount error: %s (%d)\n", FRESULT_str(fr), fr);
    //         return;
    //     }

    //     FIL fil;
    //     fr = f_open(&fil, filename, FA_READ);
    //     if (fr == FR_OK)
    //     {
    //         f_read(&fil, ram, f_size(&fil), &br);
    //     }
    //     else
    //     {
    //         printf("E f_open(%s) error: %s (%d)\n", filename, FRESULT_str(fr), fr);
    //     }

    //     fr = f_close(&fil);
    //     if (fr != FR_OK)
    //     {
    //         printf("E f_close error: %s (%d)\n", FRESULT_str(fr), fr);
    //     }
    //     f_unmount(pSD->pcName);
    // }
    // printf("I read_cart_ram_file(%s) COMPLETE (%lu bytes)\n", filename, save_size);
}

/**
 * Write a save file to the SD card
 */
void storage_write_cart_ram_file(struct gb_s *gb)
{
    // char filename[16];
    // uint_fast32_t save_size;
    // UINT bw;

    // gb_get_rom_name(gb, filename);
    // save_size = gb_get_save_size(gb);
    // if (save_size > 0)
    // {
    //     sd_card_t *pSD = sd_get_by_num(0);
    //     FRESULT fr = f_mount(&pSD->fatfs, pSD->pcName, 1);
    //     if (FR_OK != fr)
    //     {
    //         printf("E f_mount error: %s (%d)\n", FRESULT_str(fr), fr);
    //         return;
    //     }

    //     FIL fil;
    //     fr = f_open(&fil, filename, FA_CREATE_ALWAYS | FA_WRITE);
    //     if (fr == FR_OK)
    //     {
    //         f_write(&fil, ram, save_size, &bw);
    //     }
    //     else
    //     {
    //         printf("E f_open(%s) error: %s (%d)\n", filename, FRESULT_str(fr), fr);
    //     }

    //     fr = f_close(&fil);
    //     if (fr != FR_OK)
    //     {
    //         printf("E f_close error: %s (%d)\n", FRESULT_str(fr), fr);
    //     }
    //     f_unmount(pSD->pcName);
    // }
    // printf("I write_cart_ram_file(%s) COMPLETE (%lu bytes)\n", filename, save_size);
}