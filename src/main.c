/**
 * Copyright (C) 2022 by Mahyar Koshkouei <mk@deltabeard.com>
 *
 * Permission to use, copy, modify, and/or distribute this software for any
 * purpose with or without fee is hereby granted.
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES WITH
 * REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY
 * AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY SPECIAL, DIRECT,
 * INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM
 * LOSS OF USE, DATA OR PROFITS, WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR
 * OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR
 * PERFORMANCE OF THIS SOFTWARE.
 */

// // Peanut-GB emulator settings
// #define ENABLE_LCD 1
// #define ENABLE_SDCARD 1
// #define PEANUT_GB_HIGH_LCD_ACCURACY 1
// #define PEANUT_GB_USE_BIOS 0

// /* Use DMA for all drawing to LCD. Benefits aren't fully realised at the moment
//  * due to busy loops waiting for DMA completion. */
// #define USE_DMA 1

/* C Headers */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* RP2040 Headers */
#include <hardware/pio.h>
#include <hardware/dma.h>
#include <hardware/spi.h>
#include <hardware/sync.h>
#include <pico/bootrom.h>
#include <pico/stdio.h>
#include <pico/stdlib.h>
#include <pico/multicore.h>
#include <sys/unistd.h>
#include <hardware/irq.h>

#include "morana.h"
#include "GUI/menu.h"

/* Project headers */
#include "hedley.h"
// #include "minigb_apu.h"
#include "peanut_gb.h"
#include "gbcolors.h"
#include "config.h"

#include "main.h"
#include "menu/game_menu.h"
#include "gfx_lcd.h"
#include "storage.h"

#ifndef DEBUG
#include "rom_selector.h"
#endif

#include "ff.h"
#include "f_util.h"

#define putstdio(x) write(1, x, strlen(x))

settings current_settings = {
	.frame_skip = true,
	.interlace = false,
	.auto_assign_palette = true,
	.manual_palette = 0,
	.scaling_mode = STRETCH,
};

bool settings_updated = false;
bool exit_to_selector = false;

#ifndef DEBUG
const uint8_t *rom = (const uint8_t *)(XIP_BASE + FLASH_TARGET_OFFSET);
char rom_title[64];
#endif

uint8_t fps;

void update_fps(void)
{
	static uint_fast32_t frames = 0;
	static uint64_t start_time = 0;

	uint64_t end_time = time_us_64();
	frames++;

	if (start_time == 0)
	{
		start_time = end_time;
		return;
	}

	uint32_t diff = end_time - start_time;
	if (diff >= 1000000)
	{ // každou sekundu
		fps = ((uint64_t)frames * 1000 * 1000) / diff;
		frames = 0;
		start_time = end_time;
	}
}

/**
 * Returns a byte from the ROM file at the given address.
 */
uint8_t gb_rom_read(struct gb_s *gb, const uint_fast32_t addr)
{
	(void)gb;
	if (addr < sizeof(rom_bank0))
		return rom_bank0[addr];

	return rom[addr];
}

/**
 * Returns a byte from the cartridge RAM at the given address.
 */
uint8_t gb_cart_ram_read(struct gb_s *gb, const uint_fast32_t addr)
{
	(void)gb;
	return ram[addr];
}

/**
 * Writes a given byte to the cartridge RAM at the given address.
 */
void gb_cart_ram_write(struct gb_s *gb, const uint_fast32_t addr, const uint8_t val)
{
	ram[addr] = val;
}

/**
 * Ignore all errors.
 */
void gb_error(struct gb_s *gb, const enum gb_error_e gb_err, const uint16_t addr)
{
#if 1
	const char *gb_err_str[4] = {
		"UNKNOWN",
		"INVALID OPCODE",
		"INVALID READ",
		"INVALID WRITE"};
	printf("Error %d occurred: %s at %04X\n.\n", gb_err, gb_err_str[gb_err], addr);
//	abort();
#endif
}

void update_gb_joypad(struct gb_s *gb)
{
	controls_update();

	gb->direct.joypad_bits.up = !controls_is_button_pressed(UP);
	gb->direct.joypad_bits.down = !controls_is_button_pressed(DOWN);
	gb->direct.joypad_bits.left = !controls_is_button_pressed(LEFT);
	gb->direct.joypad_bits.right = !controls_is_button_pressed(RIGHT);
	gb->direct.joypad_bits.a = !controls_is_button_pressed(B);
	gb->direct.joypad_bits.b = !controls_is_button_pressed(A);

	gb->direct.joypad_bits.select = !controls_is_button_pressed(SELECT);
	gb->direct.joypad_bits.start = !controls_is_button_pressed(START);

	if (controls_is_button_pressed(MENU))
	{
		game_menu_open();
	}
}

void update_serial_input(struct gb_s *gb)
{
	/* Serial monitor commands */
	int input = getchar_timeout_us(0);
	if (input == PICO_ERROR_TIMEOUT)
		return;

	switch (input)
	{
	case 'i':
		gb->direct.interlace = !gb->direct.interlace;
		break;

	case 'f':
		gb->direct.frame_skip = !gb->direct.frame_skip;
		break;

	case 'b':
	{
		// uint64_t end_time;
		// uint32_t diff;

		// end_time = time_us_64();
		// diff = end_time - start_time;
		// fps = ((uint64_t)frames * 1000 * 1000) / diff;
		// printf("Frames: %u\n"
		// 	   "Time: %lu us\n"
		// 	   "FPS: %lu\n",
		// 	   frames, diff, fps);
		// stdio_flush();
		// frames = 0;
		// start_time = time_us_64();
		break;
	}

	default:
	{
		break;
	}
	}
}

void update_settings(struct gb_s *gb)
{
	gb->direct.frame_skip = current_settings.frame_skip; // zapnutí frame_skip může zlepšit výkon u náročnějších her, ale může způsob
	gb->direct.interlace = current_settings.interlace;	 // zapnutí interlace módu může zlepšit výkon u náročnějších her, ale může způsobit blikání obrazu. Můžete ho vypnout v menu (select + up) pro stabilnější obraz, ale s možným snížením FPS u náročnějších her.

	if (current_settings.auto_assign_palette)
		auto_assign_palette(palette, gb_colour_hash(gb), gb_get_rom_name(gb, rom_title));
	else
		manual_assign_palette(palette, current_settings.manual_palette);
}

int main(void)
{
	static struct gb_s gb;
	enum gb_init_error_e ret;

	// Inicializace displeje, DMA, PIO a dalších periferií
	stdio_init_all();
	morana_init_all();

	gfx_init();

	while (true)
	{
#ifndef DEBUG
		rom_selector();
#endif

		display_clear();

		/* Initialise GB context. */
		memcpy(rom_bank0, rom, sizeof(rom_bank0));
		ret = gb_init(&gb, &gb_rom_read, &gb_cart_ram_read, &gb_cart_ram_write, &gb_error, NULL);

		if (ret != GB_INIT_NO_ERROR)
		{
			printf("Error: %d\n", ret);
			break;
			// goto out;
		}

		gb_init_lcd(&gb, &gfx_draw_line);

		// #if ENABLE_SDCARD
		// 	/* Load Save File. */
		// 	read_cart_ram_file(&gb);
		// #endif

		update_settings(&gb);

		while (1)
		{
			update_fps();

			gb.gb_frame = 0;

			do
			{
				__gb_step_cpu(&gb);
				tight_loop_contents();
			} while (HEDLEY_LIKELY(gb.gb_frame == 0));

			if (settings_updated)
			{
				update_settings(&gb);
				settings_updated = false;
			}

			update_gb_joypad(&gb);

			if (exit_to_selector)
			{
				exit_to_selector = false;
				break;
			}

			// update_serial_input(&gb);
		}
	}

	morana_deinit_all();
}