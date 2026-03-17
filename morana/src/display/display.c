#include "display.h"
#include "display_txt.h"
#include "display_bmp.h"
#include "config.h"
#include "hardware/clocks.h"
#include <string.h>

void display_init()
{
    // Initialize display
    puts("Initializing display...");

    // Nastavení SPI pro komunikaci s displejem
    uint32_t sys_clk = clock_get_hz(clk_sys);

    clock_configure(clk_peri, 0,
                    CLOCKS_CLK_PERI_CTRL_AUXSRC_VALUE_CLK_SYS,
                    sys_clk, sys_clk);

    gpio_set_slew_rate(TFT_SCLK, GPIO_SLEW_RATE_FAST);
    gpio_set_slew_rate(TFT_MOSI, GPIO_SLEW_RATE_FAST);

    current_text_box = default_text_box;

    lcd_init();
    lcd_set_rotation(TFT_ROTATION);
    lcd_clear_screen(0x0000);
}

void display_wait_until_not_busy()
{
    lcd_wait_until_not_busy();
}

bool display_is_busy()
{
    return lcd_is_busy();
}

void display_deinit()
{
    puts("Deinitializing display...");
}

void display_clear()
{
    lcd_clear_screen(0x0000);
}
