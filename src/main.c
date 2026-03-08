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
#define ENABLE_LCD 1
#define ENABLE_SDCARD 1
#define PEANUT_GB_HIGH_LCD_ACCURACY 1
#define PEANUT_GB_USE_BIOS 0

// /* Use DMA for all drawing to LCD. Benefits aren't fully realised at the moment
//  * due to busy loops waiting for DMA completion. */
#define USE_DMA 1

/* C Headers */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* RP2040 Headers */
#include <hardware/pio.h>
#include <hardware/dma.h>
#include <hardware/spi.h>
#include <hardware/sync.h>
#include <hardware/flash.h>
#include <pico/bootrom.h>
#include <pico/stdio.h>
#include <pico/stdlib.h>
#include <pico/multicore.h>
#include <sys/unistd.h>
#include <hardware/irq.h>

#include "morana.h"

/* Project headers */
#include "hedley.h"
// #include "minigb_apu.h"
#include "peanut_gb.h"
#include "gbcolors.h"
#include "config.h"

#include "ff.h"
#include "f_util.h"

union core_cmd
{
	struct
	{
		/* Does nothing. */
#define CORE_CMD_NOP 0
		/* Set line "data" on the LCD. Pixel data is in pixels_buffer. */
#define CORE_CMD_LCD_LINE 1
		/* Control idle mode on the LCD. Limits colours to 2 bits. */
#define CORE_CMD_IDLE_SET 2
		/* Set a specific pixel. For debugging. */
#define CORE_CMD_SET_PIXEL 3
		uint8_t cmd;
		uint8_t unused1;
		uint8_t unused2;
		uint8_t data;
	};
	uint32_t full;
};

enum ScalingMode
{
	NORMAL = 0,
	STRETCH,
	STRETCH_KEEP_ASPECT,
	INTERLACE,
	INTERLACE_KEEP_ASPECT,
	COUNT
};

volatile enum ScalingMode scalingMode = NORMAL;

// /** Definition of ROM data
//  * We're going to erase and reprogram a region 1Mb from the start of the flash
//  * Once done, we can access this at XIP_BASE + 1Mb.
//  * Game Boy DMG ROM size ranges from 32768 bytes (e.g. Tetris) to 1,048,576 bytes (e.g. Pokemod Red)
//  */
#define FLASH_TARGET_OFFSET (1024 * 1024)
const uint8_t *rom = (const uint8_t *)(XIP_BASE + FLASH_TARGET_OFFSET);
static unsigned char rom_bank0[65536];

static uint8_t ram[32768];
static palette_t palette; // Colour palette
static uint8_t manual_palette_selected = 0;

/* Pixel data is stored in here. */
static uint8_t pixels_buffer[LCD_WIDTH];
static int lcd_line_busy = 0;

#define putstdio(x) write(1, x, strlen(x))

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
void gb_cart_ram_write(struct gb_s *gb, const uint_fast32_t addr,
					   const uint8_t val)
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

// void lcd_draw_line(struct gb_s *gb, const uint8_t pixels[LCD_WIDTH], const uint_fast8_t line)
// {
// 	// static uint16_t fb[DISPLAY_WIDTH] __attribute__((aligned(4)));
// 	static uint16_t fb[LCD_WIDTH];

// 	for (unsigned int x = 0; x < LCD_WIDTH; x++)
// 	{
// 		uint16_t color = palette[(pixels[x] & LCD_PALETTE_ALL) >> 4][pixels[x] & 3];
// 		fb[x] = color;
// 		// fb[2 * x + 1] = color;
// 		// fb[2 * x + LCD_WIDTH * 2] = color;
// 		// fb[2 * x + LCD_WIDTH * 2 + 1] = color;
// 	}

// 	// uint16_t line_double = line * 2; // Interlace mode: duplicate each line to reduce flickering
// 	// display_bmp_draw(fb, 0, line_double, DISPLAY_WIDTH * 2, 2);
// 	// display_bmp_draw(fb, 0, line, DISPLAY_WIDTH, 1);
// 	display_bmp_draw(fb, 0, line, LCD_WIDTH, 1);

// 	// // Pokud je další řádek "vynechaný", vykresli tam alternativní obsah
// 	// if (((int)(line * 2) % 3) != 0)
// 	// {
// 	// 	uint16_t bgColor = palette[2][0];										  // Barva pozadí z palety
// 	// 	display_draw_line(bgColor, 0, (uint16_t)(line * 1.5 + 1), LCD_WIDTH * 2); // vykreslíme znovu stejný řádek, aby se snížilo blikání při zapnutém interlace módu
// 	// }
// }

// #if ENABLE_SDCARD
// /**
//  * Load a save file from the SD card
//  */
// void read_cart_ram_file(struct gb_s *gb) {
// char filename[16];
// 	uint_fast32_t save_size;
// 	UINT br;

// 	gb_get_rom_name(gb,filename);
// 	save_size=gb_get_save_size(gb);
// 	if(save_size>0) {
// 		sd_card_t *pSD=sd_get_by_num(0);
// 		FRESULT fr=f_mount(&pSD->fatfs,pSD->pcName,1);
// 		if (FR_OK!=fr) {
// 			printf("E f_mount error: %s (%d)\n",FRESULT_str(fr),fr);
// 			return;
// 		}

// 		FIL fil;
// 		fr=f_open(&fil,filename,FA_READ);
// 		if (fr==FR_OK) {
// 			f_read(&fil,ram,f_size(&fil),&br);
// 		} else {
// 			printf("E f_open(%s) error: %s (%d)\n",filename,FRESULT_str(fr),fr);
// 		}

// 		fr=f_close(&fil);
// 		if(fr!=FR_OK) {
// 			printf("E f_close error: %s (%d)\n", FRESULT_str(fr), fr);
// 		}
// 		f_unmount(pSD->pcName);
// 	}
// 	printf("I read_cart_ram_file(%s) COMPLETE (%lu bytes)\n",filename,save_size);
// }

// /**
//  * Write a save file to the SD card
//  */
// void write_cart_ram_file(struct gb_s *gb) {
// 	char filename[16];
// 	uint_fast32_t save_size;
// 	UINT bw;

// 	gb_get_rom_name(gb,filename);
// 	save_size=gb_get_save_size(gb);
// 	if(save_size>0) {
// 		sd_card_t *pSD=sd_get_by_num(0);
// 		FRESULT fr=f_mount(&pSD->fatfs,pSD->pcName,1);
// 		if (FR_OK!=fr) {
// 			printf("E f_mount error: %s (%d)\n",FRESULT_str(fr),fr);
// 			return;
// 		}

// 		FIL fil;
// 		fr=f_open(&fil,filename,FA_CREATE_ALWAYS | FA_WRITE);
// 		if (fr==FR_OK) {
// 			f_write(&fil,ram,save_size,&bw);
// 		} else {
// 			printf("E f_open(%s) error: %s (%d)\n",filename,FRESULT_str(fr),fr);
// 		}

// 		fr=f_close(&fil);
// 		if(fr!=FR_OK) {
// 			printf("E f_close error: %s (%d)\n", FRESULT_str(fr), fr);
// 		}
// 		f_unmount(pSD->pcName);
// 	}
// 	printf("I write_cart_ram_file(%s) COMPLETE (%lu bytes)\n",filename,save_size);
// }

typedef struct
{
	uint32_t flash_target_offset;
	bool mismatch;
} flash_write_state_t;

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
		printf("I Programming successful!\n");
	}
	else
	{
		printf("E Programming failed!\n");
	}

	// printf("I load_cart_rom_file(%s) COMPLETE (%lu bytes)\n", filename, state.flash_target_offset - FLASH_TARGET_OFFSET);
}

/**
 * Function used by the rom file selector to display one page of .gb rom files
 */
uint16_t rom_file_selector_display_page(char filename[FILES_PER_PAGE][64], uint16_t num_page)
{
	// /* clear the filenames array */
	for (uint8_t ifile = 0; ifile < FILES_PER_PAGE; ifile++)
	{
		strcpy(filename[ifile], "");
	}

	/* search *.gb files */
	int files_to_skip = num_page * FILES_PER_PAGE;

	sd_entry_t entries[FILES_PER_PAGE];
	int found = sd_list_directory_pattern("", "*.gb", entries, files_to_skip, FILES_PER_PAGE);

	/* store the filenames of this page */
	display_clear();
	display_txt_set_cursor(0, 0);

	for (int i = 0; i < found; i++)
	{
		strcpy(filename[i], entries[i].name);
		display_txt_write_line(filename[i]);
	}

	return found;
}

/**
 * The ROM selector displays pages of up to FILES_PER_PAGE rom files
 * allowing the user to select which rom file to start
 * Copy your *.gb rom files to the root directory of the SD card
 */
void rom_file_selector()
{

	uint16_t num_page;
	char filename[FILES_PER_PAGE][64];
	uint16_t num_file;

	text_box box = {
		.box.position = {5, 5},
		.box.size = {DISPLAY_WIDTH - 10, DISPLAY_HEIGHT - 10},
		.line_margin = 1,
		.text_color = SELECTOR_TEXT_COLOR,
		.text_bgcolor = SELECTOR_BG_COLOR,
		.line = 0,
		.column = 0,
		.wrap = DISPLAY_TXT_WRAP_NONE};

	display_txt_set_text_box(&box);

	/* display the first page with up to 22 rom files */
	num_file = rom_file_selector_display_page(filename, num_page);

	// /* select the first rom */
	uint8_t selected = 0;
	display_txt_set_cursor(selected, 0);
	display_txt_write_line_color(filename[selected], SELECTOR_HIGHLIGHT_COLOR, SELECTOR_BG_COLOR);

	while (true)
	{

		controls_update();
		display_txt_set_cursor(selected, 0);

		if (controls_is_button_pressed(START))
		{
			/* re-start the last game (no need to reprogram flash) */
			break;
		}
		if (controls_is_button_pressed(A) || controls_is_button_pressed(B))
		{
			/* copy the rom from the SD card to flash and start the game */
			load_cart_rom_file(filename[selected]);
			break;
		}
		if (controls_is_button_pressed(DOWN))
		{
			/* select the next rom */
			display_txt_write_line_color(filename[selected], SELECTOR_TEXT_COLOR, SELECTOR_BG_COLOR);
			selected++;
			if (selected >= num_file)
				selected = 0;
			display_txt_set_cursor(selected, 0);
			display_txt_write_line_color(filename[selected], SELECTOR_HIGHLIGHT_COLOR, SELECTOR_BG_COLOR);
			sleep_ms(SELECTOR_SCROLL_DELAY_MS);
		}
		if (controls_is_button_pressed(UP))
		{
			/* select the previous rom */
			display_txt_write_line_color(filename[selected], SELECTOR_TEXT_COLOR, SELECTOR_BG_COLOR);
			if (selected == 0)
			{
				selected = num_file - 1;
			}
			else
			{
				selected--;
			}
			display_txt_set_cursor(selected, 0);
			display_txt_write_line_color(filename[selected], SELECTOR_HIGHLIGHT_COLOR, SELECTOR_BG_COLOR);
			sleep_ms(SELECTOR_SCROLL_DELAY_MS);
		}
		if (controls_is_button_pressed(RIGHT))
		{
			/* select the next page */
			num_page++;
			num_file = rom_file_selector_display_page(filename, num_page);
			if (num_file == 0)
			{
				/* no files in this page, go to the previous page */
				num_page--;
				num_file = rom_file_selector_display_page(filename, num_page);
			}
			/* select the first file */
			selected = 0;
			display_txt_set_cursor(selected, 0);
			display_txt_write_line_color(filename[selected], SELECTOR_HIGHLIGHT_COLOR, SELECTOR_BG_COLOR);
			sleep_ms(SELECTOR_SCROLL_DELAY_MS);
		}
		if (controls_is_button_pressed(LEFT) && num_page > 0)
		{
			/* select the previous page */
			num_page--;
			num_file = rom_file_selector_display_page(filename, num_page);
			/* select the first file */
			selected = 0;
			display_txt_set_cursor(selected, 0);
			display_txt_write_line_color(filename[selected], SELECTOR_HIGHLIGHT_COLOR, SELECTOR_BG_COLOR);
			sleep_ms(SELECTOR_SCROLL_DELAY_MS);
		}

		tight_loop_contents();
	}
};

void update_gb_joypad(struct gb_s *gb)
{
	controls_update();

	gb->direct.joypad_bits.up = !controls_is_button_pressed(UP);
	gb->direct.joypad_bits.down = !controls_is_button_pressed(DOWN);
	gb->direct.joypad_bits.left = !controls_is_button_pressed(LEFT);
	gb->direct.joypad_bits.right = !controls_is_button_pressed(RIGHT);
	gb->direct.joypad_bits.a = !controls_is_button_pressed(A);
	gb->direct.joypad_bits.b = !controls_is_button_pressed(B);

	gb->direct.joypad_bits.select = !controls_is_button_pressed(SELECT);
	gb->direct.joypad_bits.start = !controls_is_button_pressed(START);
}

void update_serial_input(struct gb_s *gb)
{
	static uint_fast32_t frames = 0;
	static uint64_t start_time = 0;

	if (start_time == 0)
		start_time = time_us_64();

	frames++;

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
		uint64_t end_time;
		uint32_t diff;
		uint32_t fps;

		end_time = time_us_64();
		diff = end_time - start_time;
		fps = ((uint64_t)frames * 1000 * 1000) / diff;
		printf("Frames: %u\n"
			   "Time: %lu us\n"
			   "FPS: %lu\n",
			   frames, diff, fps);
		stdio_flush();
		frames = 0;
		start_time = time_us_64();
		break;
	}

	default:
	{
		break;
	}
	}
}

bool update_menu_combos(struct gb_s *gb)
{
	while (controls_is_button_pressed(MENU))
	{
		controls_update();

		if (controls_is_button_pressed(RIGHT))
		{
			/* select + right: select the next manual color palette */
			if (manual_palette_selected < 12)
			{
				manual_palette_selected++;
				manual_assign_palette(palette, manual_palette_selected);
			}

			sleep_ms(MENU_COMBO_DELAY_MS);
		}
		if (controls_is_button_pressed(LEFT))
		{
			/* select + left: select the previous manual color palette */
			if (manual_palette_selected > 0)
			{
				manual_palette_selected--;
				manual_assign_palette(palette, manual_palette_selected);
			}

			sleep_ms(MENU_COMBO_DELAY_MS);
		}
		// if (controls_is_button_pressed(X))
		// {
		// 	/* select + start: save ram and resets to the game selection menu */
		// 	write_cart_ram_file(gb);
		// 	// rom_file_selector();
		// }
		// if(controls_is_button_pressed(Y))
		// {
		// 	read_cart_ram_file(gb);
		// }
		if (controls_is_button_pressed(UP))
		{
			/* select + up: toggle interlace mode */
			// gb->direct.interlace = !gb->direct.interlace;
			scalingMode = (scalingMode + 1) % COUNT;

			sleep_ms(MENU_COMBO_DELAY_MS);
		}
		if (controls_is_button_pressed(DOWN))
		{
			/* select + Y: toggle auto-assign palette on/off */
			if (manual_palette_selected == 0)
			{
				manual_palette_selected = 1;
				manual_assign_palette(palette, manual_palette_selected);
			}
			else
			{
				manual_palette_selected = 0;
				auto_assign_palette(palette, gb_colour_hash(gb), NULL);
			}

			sleep_ms(MENU_COMBO_DELAY_MS);
		}
		if (controls_is_button_pressed(A))
		{
			/* select + A: toggle frame-skip on/off */
			gb->direct.frame_skip = !gb->direct.frame_skip;

			sleep_ms(MENU_COMBO_DELAY_MS);
		}
		if (controls_is_button_pressed(SELECT))
		{
			/* select + start: save ram and resets to the game selection menu */
			// write_cart_ram_file(&gb);
			// rom_file_selector();

			// prozatím jen návrat do výběru rom bez uložení, protože zatím není implementováno načítaní uložené pozice z ramky, takže by to stejně nemělo efekt
			return true;
		}

		// 	if (!gb.direct.joypad_bits.a && prev_joypad_bits.a)
		// 	{
		// 		/* select + A: enable/disable frame-skip => fast-forward */
		// 		gb.direct.frame_skip = !gb.direct.frame_skip;
		// 		printf("I gb.direct.frame_skip = %d\n", gb.direct.frame_skip);
		// 	}
	}

	return false;
}

static uint8_t scaledLineOffsetTable[LCD_HEIGHT]; // scaled to 240 lines

#define IS_REPEATED(pos) ((pos % 2) || (pos % 6 == 0))

static void calcExtraLineTable()
{
	uint8_t offset = 0;
	for (uint8_t line = 0; line < LCD_HEIGHT; ++line)
	{
		scaledLineOffsetTable[line] = offset;
		offset += 1 + IS_REPEATED(line);
	}
}

void lcd_write_pixels_normal(const uint16_t *pixels, uint8_t line, uint_fast16_t count)
{
	const uint16_t colOffset = (DISPLAY_WIDTH - LCD_WIDTH) / 2;
	const uint16_t screenLineOffset = (DISPLAY_HEIGHT - LCD_HEIGHT) / 2;

	display_bmp_draw(pixels, colOffset, screenLineOffset + line, LCD_WIDTH, 1);
}

void lcd_write_pixels_stretched(const uint16_t *pixels, uint8_t line, uint_fast16_t count)
{
	static uint16_t doubledPixels[DISPLAY_WIDTH * 2];
	const uint8_t lineRepeated = IS_REPEATED(line);

	uint16_t pos = 0;

	for (int col = 0; col < count; ++col)
	{
		doubledPixels[pos] = pixels[col];
		if (lineRepeated)
		{
			doubledPixels[pos + DISPLAY_WIDTH] = pixels[col];
		}
		pos++;
		doubledPixels[pos] = pixels[col];
		if (lineRepeated)
		{
			doubledPixels[pos + DISPLAY_WIDTH] = pixels[col];
		}
		pos++;
	}

	const uint16_t stretchedWidth = count * 2;
	const uint16_t lineOffset = scaledLineOffsetTable[line];

	// display_bmp_draw(doubledPixels, 0, lineOffset, stretchedWidth, lineRepeated ? 2 : 1);
	display_bmp_draw(doubledPixels, 0, lineOffset, stretchedWidth, 1);
}

void lcd_write_pixels_stretched_keep_aspect(const uint16_t *pixels, uint8_t line, uint_fast16_t count)
{
	static uint16_t doubledPixels[DISPLAY_WIDTH];
	uint16_t pos = 0;
	for (int col = 0; col < count; ++col)
	{
		doubledPixels[pos++] = pixels[col];
		if (IS_REPEATED(col))
		{
			doubledPixels[pos++] = pixels[col];
		}
	}

	const uint16_t stretchedWidth = pos;

	const uint16_t colOffset = (DISPLAY_WIDTH - stretchedWidth) / 2;

	const uint8_t lineRepeated = IS_REPEATED(line);
	const uint16_t lineOffset = scaledLineOffsetTable[line];

	display_bmp_draw(doubledPixels, colOffset, lineOffset, stretchedWidth, 1);
	if (lineRepeated)
	{
		sleep_us(5);
		display_bmp_draw(doubledPixels, colOffset, lineOffset + 1, stretchedWidth, 1);
	}
}

void lcd_write_pixels_interlace_aspect(const uint16_t *pixels, uint8_t line, uint_fast16_t count)
{
	static uint16_t newPixels[DISPLAY_WIDTH];

	// uint16_t bg_darkest = palette[2][3] / 2;
	// memset(newPixels, 0, sizeof(newPixels)); // vynulování bufferu

	uint16_t pos = 0;
	for (int col = 0; col < count; col++)
	{
		newPixels[pos] = pixels[col];
		pos++;

		if (col % 3 == 0)
		{
			newPixels[pos] = pixels[col]; // nebo jiná barva pozadí z palety
			pos++;
			// newPixels[pos++] = bg_darkest; // nebo jiná barva pozadí z palety
		}
	}

	const uint16_t stretchedWidth = pos - 1;
	const uint16_t colOffset = (DISPLAY_WIDTH - stretchedWidth) / 2;
	const uint16_t screenLineOffset = line * 1.5;

	display_bmp_draw(newPixels, colOffset, screenLineOffset, stretchedWidth, 1);
}

// Writes pixels to screen or framebuffer
void lcd_write_pixels(const uint16_t *pixels, uint8_t line, uint_fast16_t count)
{
	// lcd_write_pixels_normal(pixels, line, count);

	switch (scalingMode)
	{
	case STRETCH:
		lcd_write_pixels_stretched(pixels, line, count);
		break;
	case STRETCH_KEEP_ASPECT:
		lcd_write_pixels_stretched_keep_aspect(pixels, line, count);
		break;
	case INTERLACE_KEEP_ASPECT:
		lcd_write_pixels_interlace_aspect(pixels, line, count);
		break;
	case NORMAL:
	default:
		lcd_write_pixels_normal(pixels, line, count);
		break;
	}
}

void core1_lcd_draw_line(const uint_fast8_t line)
{
	static uint16_t fb[LCD_WIDTH];

	for (unsigned int x = 0; x < LCD_WIDTH; x++)
	{
		uint16_t color = palette[(pixels_buffer[x] & LCD_PALETTE_ALL) >> 4][pixels_buffer[x] & 3];
		fb[x] = color;
	}

	// display_bmp_draw(fb, 0, line, DISPLAY_WIDTH, 1);
	lcd_write_pixels(fb, line, LCD_WIDTH);
	__atomic_store_n(&lcd_line_busy, 0, __ATOMIC_SEQ_CST);
}

void lcd_draw_line(struct gb_s *gb, const uint8_t pixels[LCD_WIDTH], const uint_fast8_t line)
{
	union core_cmd cmd;

	/* Wait until previous line is sent. */
	while (__atomic_load_n(&lcd_line_busy, __ATOMIC_SEQ_CST))
		tight_loop_contents();

	memcpy(pixels_buffer, pixels, LCD_WIDTH);

	/* Populate command. */
	cmd.cmd = CORE_CMD_LCD_LINE;
	cmd.data = line;

	__atomic_store_n(&lcd_line_busy, 1, __ATOMIC_SEQ_CST);
	multicore_fifo_push_blocking(cmd.full);
}

_Noreturn void main_core1(void)
{
	union core_cmd cmd;

	display_clear();

	calcExtraLineTable();

	/* Handle commands coming from core0. */
	while (1)
	{
		cmd.full = multicore_fifo_pop_blocking();
		switch (cmd.cmd)
		{
		case CORE_CMD_LCD_LINE:
			core1_lcd_draw_line(cmd.data);
			break;

		case CORE_CMD_IDLE_SET:
			display_clear(); // prozatím jen vyčistí obrazovku, ale ideálně by to mělo přepnout do nějakého úsporného módu, který by omezil barvy a snížil frekvenci aktualizace, aby se ušetřila energie a snížilo zahřívání při dlouhém hraní náročnějších her
			break;

		case CORE_CMD_NOP:
		default:
			break;
		}
	}

	HEDLEY_UNREACHABLE();
}

int main(void)
{
	static struct gb_s gb;
	enum gb_init_error_e ret;

	/* Initialise USB serial connection for debugging. */
	stdio_init_all();
	morana_init_all();

	while (true)
	{
		/* ROM File selector */
		rom_file_selector();

		/* Initialise GB context. */
		memcpy(rom_bank0, rom, sizeof(rom_bank0));
		ret = gb_init(&gb, &gb_rom_read, &gb_cart_ram_read,
					  &gb_cart_ram_write, &gb_error, NULL);

		if (ret != GB_INIT_NO_ERROR)
		{
			printf("Error: %d\n", ret);
			break;
			// goto out;
		}

		/* Automatically assign a colour palette to the game */
		char rom_title[16];
		auto_assign_palette(palette, gb_colour_hash(&gb), gb_get_rom_name(&gb, rom_title));

		gb_init_lcd(&gb, &lcd_draw_line);

		/* Start Core1, which processes requests to the LCD. */
		putstdio("CORE1 ");
		multicore_launch_core1(main_core1);

		display_clear();

		// #if ENABLE_SDCARD
		// 	/* Load Save File. */
		// 	read_cart_ram_file(&gb);
		// #endif

		gb.direct.frame_skip = true; // zapnutí frame_skip může zlepšit výkon u náročnějších her, ale může způsob
		// gb.direct.interlace = true;  // zapnutí interlace módu může zlepšit výkon u náročnějších her, ale může způsobit blikání obrazu. Můžete ho vypnout v menu (select + up) pro stabilnější obraz, ale s možným snížením FPS u náročnějších her.

		while (1)
		{
			int input;

			gb.gb_frame = 0;

			gb_run_frame(&gb);

			do
			{
				__gb_step_cpu(&gb);
				tight_loop_contents();
			} while (HEDLEY_LIKELY(gb.gb_frame == 0));

			update_gb_joypad(&gb);
			if (update_menu_combos(&gb))
			{
				// pokud se v menu zvolí návrat do výběru rom, ukončíme běh aktuální instance emulátoru a vrátíme se do výběru rom
				break;
			}
			update_serial_input(&gb);
		}
	}

	morana_deinit_all();
}