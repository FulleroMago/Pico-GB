
/* Pixel data is stored in here. */
static uint8_t pixels_buffer[LCD_WIDTH];
static int lcd_line_busy = 0;

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

void lcd_draw_line(struct gb_s *gb, const uint8_t pixels[LCD_WIDTH], const uint_fast8_t line)
{
	// static uint16_t fb[DISPLAY_WIDTH] __attribute__((aligned(4)));
	static uint16_t fb[LCD_WIDTH];

	for (unsigned int x = 0; x < LCD_WIDTH; x++)
	{
		uint16_t color = palette[(pixels[x] & LCD_PALETTE_ALL) >> 4][pixels[x] & 3];
		fb[x] = color;
		// fb[2 * x + 1] = color;
		// fb[2 * x + LCD_WIDTH * 2] = color;
		// fb[2 * x + LCD_WIDTH * 2 + 1] = color;
	}

	// uint16_t line_double = line * 2; // Interlace mode: duplicate each line to reduce flickering
	// display_bmp_draw(fb, 0, line_double, DISPLAY_WIDTH * 2, 2);
	// display_bmp_draw(fb, 0, line, DISPLAY_WIDTH, 1);
	display_bmp_draw(fb, 0, line, LCD_WIDTH, 1);

	// // Pokud je další řádek "vynechaný", vykresli tam alternativní obsah
	// if (((int)(line * 2) % 3) != 0)
	// {
	// 	uint16_t bgColor = palette[2][0];										  // Barva pozadí z palety
	// 	display_draw_line(bgColor, 0, (uint16_t)(line * 1.5 + 1), LCD_WIDTH * 2); // vykreslíme znovu stejný řádek, aby se snížilo blikání při zapnutém interlace módu
	// }
}

//////////////// LCD CORE 1 ///////////
// Writes pixels to screen or framebuffer
void lcd_write_pixels(const uint16_t *pixels, uint8_t line, uint_fast16_t count)
{
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

//////////// Menu actions
if (controls_is_button_pressed(RIGHT))
{
	if (m->on_next)
	{
		uint8_t result = m->on_next(NULL);
		if (result > 0)
		{
			break; // vyskok z menu, pokud akce vrátí nenulovou hodnotu
		}
	}
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
	display_txt_write_line_color(filename[selected], settings->text_select_color, settings->text_bgcolor);
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
	display_txt_write_line_color(filename[selected], settings->text_select_color, settings->text_bgcolor);
	sleep_ms(SELECTOR_SCROLL_DELAY_MS);
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
			current_scaling_mode = (current_scaling_mode + 1) % COUNT;

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