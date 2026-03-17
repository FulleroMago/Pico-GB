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

#if !DEBUG
#include "rom_selector.h"
#endif

#include "ff.h"
#include "f_util.h"

#define putstdio(x) write(1, x, strlen(x))

static palette_t palette; // Colour palette
static uint8_t manual_palette_selected = 0;

volatile enum ScalingMode scalingMode = STRETCH;

static uint8_t scaledLineOffsetTable[LCD_HEIGHT]; // scaled to 240 lines

#define IS_REPEATED(pos) ((pos % 2) || (pos % 6 == 0))

static uint16_t display_buffers[2][DISPLAY_WIDTH * BUFFERED_LINES]; // buffer pro 10 řádků, který se používá pro různé režimy kreslení (stretched, interlace atd.)

#if !DEBUG
const uint8_t *rom = (const uint8_t *)(XIP_BASE + FLASH_TARGET_OFFSET);
char rom_title[64];
#endif

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
			lcd_clear_screen(0);
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

static void calcExtraLineTable()
{
	uint8_t offset = 0;
	for (uint8_t line = 0; line < LCD_HEIGHT; ++line)
	{
		scaledLineOffsetTable[line] = offset;
		offset += 1 + IS_REPEATED(line);
	}
}

static uint8_t active_display_buffer = 0; // index aktivního bufferu

void write_display_buffer_line(const uint16_t *pixels, uint16_t x, uint16_t y, uint16_t width)
{
	memcpy(&display_buffers[active_display_buffer][y * DISPLAY_WIDTH + x], pixels, width * sizeof(uint16_t));
}

void lcd_write_pixels_normal(const uint16_t *pixels, uint8_t line, uint_fast16_t count)
{
	const uint16_t colOffset = (DISPLAY_WIDTH - LCD_WIDTH) / 2;
	const uint16_t screenLineOffset = (DISPLAY_HEIGHT - LCD_HEIGHT) / 2;

	display_bmp_draw(pixels, colOffset, screenLineOffset + line, LCD_WIDTH, 1);
}

void lcd_write_pixels_stretched(const uint16_t *pixels, uint8_t line, uint_fast16_t count)
{
	static uint16_t doubledPixels[DISPLAY_WIDTH];
	uint16_t pos = 0;
	for (int col = 0; col < count; ++col)
	{
		doubledPixels[pos++] = pixels[col];
		doubledPixels[pos++] = pixels[col];
	}

	const uint8_t lineRepeated = IS_REPEATED(line);
	const uint16_t lineOffset = scaledLineOffsetTable[line];

	// Cyclic buffer logic
	static uint16_t filledLines = 0;
	uint16_t bufferLine = lineOffset % BUFFERED_LINES;
	write_display_buffer_line(doubledPixels, 0, bufferLine, DISPLAY_WIDTH);

	filledLines++;
	if (lineRepeated)
	{
		uint16_t nextBufferLine = (bufferLine + 1) % BUFFERED_LINES;
		write_display_buffer_line(doubledPixels, 0, nextBufferLine, DISPLAY_WIDTH);
		filledLines++;
	}

	// Pokud je buffer plný, vypiš na display
	if (filledLines >= BUFFERED_LINES || line == LCD_HEIGHT - 1)
	{
		display_wait_until_not_busy(); // počkej, dokud není displej připraven pro další příkazy (např. dokončení DMA přenosu)
		uint8_t firstLineInBuffer = (lineOffset / BUFFERED_LINES) * BUFFERED_LINES;
		display_bmp_draw(display_buffers[active_display_buffer], 0, firstLineInBuffer, DISPLAY_WIDTH, BUFFERED_LINES);

		// Přepni aktivní buffer
		active_display_buffer = (active_display_buffer + 1) % 2;

		filledLines = 0;
	}
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

	// Cyclic buffer logic
	static uint16_t filledLines = 0;
	uint16_t bufferLine = lineOffset % BUFFERED_LINES;
	write_display_buffer_line(doubledPixels, colOffset, bufferLine, stretchedWidth);
	filledLines++;
	if (lineRepeated)
	{
		uint16_t nextBufferLine = (bufferLine + 1) % BUFFERED_LINES;
		write_display_buffer_line(doubledPixels, colOffset, nextBufferLine, stretchedWidth);
		filledLines++;
	}

	// Pokud je buffer plný, vypiš na display
	if (filledLines >= BUFFERED_LINES || line == LCD_HEIGHT - 1)
	{
		display_wait_until_not_busy(); // počkej, dokud není displej připraven pro další příkazy (např. dokončení DMA přenosu)
		uint8_t firstLineInBuffer = (lineOffset / BUFFERED_LINES) * BUFFERED_LINES;
		display_bmp_draw(display_buffers[active_display_buffer], 0, firstLineInBuffer, DISPLAY_WIDTH, BUFFERED_LINES);

		// Přepni aktivní buffer
		active_display_buffer = (active_display_buffer + 1) % 2;

		filledLines = 0;
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

void lcd_draw_line(struct gb_s *gb, const uint8_t pixels[LCD_WIDTH], const uint_fast8_t line)
{
	static uint16_t fb[LCD_WIDTH];

	for (unsigned int x = 0; x < LCD_WIDTH; x++)
	{
		uint16_t color = palette[(pixels[x] & LCD_PALETTE_ALL) >> 4][pixels[x] & 3];
		fb[x] = color;
	}

	switch (scalingMode)
	{
	case STRETCH:
		lcd_write_pixels_stretched(fb, line, LCD_WIDTH);
		break;
	case STRETCH_KEEP_ASPECT:
		lcd_write_pixels_stretched_keep_aspect(fb, line, LCD_WIDTH);
		break;
	// case INTERLACE_KEEP_ASPECT:
	// 	lcd_write_pixels_interlace_aspect(fb, line, LCD_WIDTH);
	// 	break;
	case NORMAL:
	default:
		lcd_write_pixels_normal(fb, line, LCD_WIDTH);
		break;
	}
}

int main(void)
{
	static struct gb_s gb;
	enum gb_init_error_e ret;

	// Inicializace displeje, DMA, PIO a dalších periferií
	stdio_init_all();
	morana_init_all();

	calcExtraLineTable();

	while (true)
	{
#if !DEBUG
		rom_selector();
#endif

		display_clear();

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
#if DEBUG
		char rom_title[16] = "Duck Tales (U) [!].gba";
#endif
		auto_assign_palette(palette, gb_colour_hash(&gb), gb_get_rom_name(&gb, rom_title));

		gb_init_lcd(&gb, &lcd_draw_line);

		// #if ENABLE_SDCARD
		// 	/* Load Save File. */
		// 	read_cart_ram_file(&gb);
		// #endif

		gb.direct.frame_skip = true; // zapnutí frame_skip může zlepšit výkon u náročnějších her, ale může způsob
		// gb.direct.interlace = true;	 // zapnutí interlace módu může zlepšit výkon u náročnějších her, ale může způsobit blikání obrazu. Můžete ho vypnout v menu (select + up) pro stabilnější obraz, ale s možným snížením FPS u náročnějších her.

		while (1)
		{
			int input;

			gb.gb_frame = 0;

			do
			{
				__gb_step_cpu(&gb);
				tight_loop_contents();
			} while (HEDLEY_LIKELY(gb.gb_frame == 0));

			update_gb_joypad(&gb);
			// if (update_menu_combos(&gb))
			// {
			// 	// pokud se v menu zvolí návrat do výběru rom, ukončíme běh aktuální instance emulátoru a vrátíme se do výběru rom
			// 	break;
			// }

			update_serial_input(&gb);
		}
	}

	morana_deinit_all();
}