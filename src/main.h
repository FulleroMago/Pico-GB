#pragma once
#include <stdint.h>
#include <hardware/flash.h>

// /** Definition of ROM data
//  * We're going to erase and reprogram a region 1Mb from the start of the flash
//  * Once done, we can access this at XIP_BASE + 1Mb.
//  * Game Boy DMG ROM size ranges from 32768 bytes (e.g. Tetris) to 1,048,576 bytes (e.g. Pokemod Red)
//  */
#define FLASH_TARGET_OFFSET (1024 * 1024)

#if DEBUG
extern const unsigned char rom[];
#else
extern const uint8_t *rom;
#endif

extern char rom_title[64];

static unsigned char rom_bank0[65536];
static uint8_t ram[32768];

enum ScalingMode
{
    NORMAL = 0,
    STRETCH,
    STRETCH_KEEP_ASPECT,
    // INTERLACE,
    // INTERLACE_KEEP_ASPECT,
    COUNT
};