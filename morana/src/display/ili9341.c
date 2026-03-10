#include "pico/stdlib.h"
#include "hardware/dma.h"
#include "hardware/spi.h"
#include "hardware/gpio.h"
#include <stdio.h>

#include "ili9341.h"
#include "config.h"

uint16_t _width; // Display width as modified by current rotation
uint16_t _height; // Display height as modified by current rotation
uint8_t rotation; // Current rotation (0-3)

#ifdef TFT_USE_DMA
uint dma_tx;
dma_channel_config dma_cfg;
void waitForDMA()
{
    dma_channel_wait_for_finish_blocking(dma_tx);
}
#endif

void lcd_select()
{
    gpio_put(TFT_CS, 0);
}

void lcd_deselect()
{
    gpio_put(TFT_CS, 1);
}

void lcd_reset()
{
    gpio_put(TFT_RST, 0);
    sleep_ms(5);
    gpio_put(TFT_RST, 1);
    sleep_ms(150);
}

void lcd_command_mode()
{
    gpio_put(TFT_DC, 0);
}

void lcd_data_mode()
{
    gpio_put(TFT_DC, 1);
}

void lcd_write_command(uint8_t cmd)
{
    lcd_command_mode();
    spi_set_format(TFT_SPI, 8, SPI_CPOL_1, SPI_CPOL_1, SPI_MSB_FIRST);
    spi_write_blocking(TFT_SPI, &cmd, sizeof(cmd));
}

void lcd_write_data(uint8_t *buff, size_t buff_size)
{
    lcd_data_mode();
    spi_set_format(TFT_SPI, 8, SPI_CPOL_1, SPI_CPOL_1, SPI_MSB_FIRST);
    spi_write_blocking(TFT_SPI, buff, buff_size);
}

void lcd_write_command_with_data(uint8_t cmd, uint8_t *data, uint8_t data_len)
{
    lcd_select();
    lcd_write_command(cmd);
    lcd_write_data(data, data_len);
    lcd_deselect();
}

void lcd_init_spi()
{
    spi_init(TFT_SPI, 1000 * 40000);
    spi_set_format(TFT_SPI, 16, SPI_CPOL_1, SPI_CPOL_1, SPI_MSB_FIRST);
    gpio_set_function(TFT_SCLK, GPIO_FUNC_SPI);
    gpio_set_function(TFT_MOSI, GPIO_FUNC_SPI);

    gpio_init(TFT_CS);
    gpio_set_dir(TFT_CS, GPIO_OUT);
    gpio_put(TFT_CS, 1);

    gpio_init(TFT_DC);
    gpio_set_dir(TFT_DC, GPIO_OUT);
    gpio_put(TFT_DC, 1);

    gpio_init(TFT_RST);
    gpio_set_dir(TFT_RST, GPIO_OUT);
    gpio_put(TFT_RST, 1);

#if TFT_USE_DMA
    dma_tx = dma_claim_unused_channel(true);
    dma_cfg = dma_channel_get_default_config(dma_tx);
    channel_config_set_transfer_data_size(&dma_cfg, DMA_SIZE_16);
    channel_config_set_dreq(&dma_cfg, spi_get_dreq(TFT_SPI, true));

    dma_channel_set_irq0_enabled(dma_tx, true);
    irq_set_exclusive_handler(DMA_IRQ_0, dma_irq_handler);
    irq_set_enabled(DMA_IRQ_0, true);
#endif

}

void dma_irq_handler()
{
    if (dma_channel_get_irq0_status(dma_tx))
    {
        dma_channel_acknowledge_irq0(dma_tx);
        lcd_deselect();
    }
}

void lcd_init()
{
    puts("lcd_init start");

    lcd_init_spi();

    lcd_select();
    lcd_reset();

    // Initialization sequence based on ILI9341 data sheet and Adafruit library
    lcd_write_command_with_data(ILI9341_MANUFACTURER_ACCESS, (uint8_t[]){0x03, 0x80, 0x02}, 3); // Manufacturer Command Access Protect
    lcd_write_command_with_data(ILI9341_POWER_CONTROL_B, (uint8_t[]){0x00, 0xC1, 0x30}, 3); // Power Control B
    lcd_write_command_with_data(ILI9341_POWER_ON_SEQ_CONTROL, (uint8_t[]){0x64, 0x03, 0x12, 0x81}, 4); // Power On Sequence Control
    lcd_write_command_with_data(ILI9341_DRIVER_TIMING_CTRL_A, (uint8_t[]){0x85, 0x00, 0x78}, 3); // Driver Timing Control A
    lcd_write_command_with_data(ILI9341_POWER_CONTROL_A, (uint8_t[]){0x39, 0x2C, 0x00, 0x34, 0x02}, 5); // Power Control A
    lcd_write_command_with_data(ILI9341_PUMP_RATIO_CONTROL, (uint8_t[]){0x20}, 1); // Pump Ratio Control
    lcd_write_command_with_data(ILI9341_DRIVER_TIMING_CTRL_B, (uint8_t[]){0x00, 0x00}, 2); // Driver Timing Control B

    lcd_write_command_with_data(ILI9341_PWCTR1, (uint8_t[]){0x23}, 1); // Power Control 1
    lcd_write_command_with_data(ILI9341_PWCTR2, (uint8_t[]){0x10}, 1); // Power Control 2
    lcd_write_command_with_data(ILI9341_VMCTR1, (uint8_t[]){0x3E, 0x28}, 2); // VCOM Control 1
    lcd_write_command_with_data(ILI9341_VMCTR2, (uint8_t[]){0x86}, 1); // VCOM Control 2
    lcd_write_command_with_data(ILI9341_MADCTL, (uint8_t[]){0x48}, 1); // Memory Access Control
    lcd_write_command_with_data(ILI9341_VSCRSADD, (uint8_t[]){0x00}, 1); // Vertical Scroll Start Address
    lcd_write_command_with_data(ILI9341_PIXFMT, (uint8_t[]){0x55}, 1); // 16-bit color
    lcd_write_command_with_data(ILI9341_FRMCTR1, (uint8_t    []){0x00, 0x18}, 2); // Frame Rate Control (Normal Mode)
    lcd_write_command_with_data(ILI9341_DFUNCTR, (uint8_t[]){0x08, 0x82, 0x27}, 3); // Display Function Control

    lcd_write_command_with_data(ILI9341_3GAMMA_FUNCTION_DISABLE, (uint8_t[]){0x00}, 1); // 3Gamma Function Disable
    lcd_write_command_with_data(ILI9341_GAMMASET, (uint8_t[]){0x01}, 1); // Gamma curve selected
    lcd_write_command_with_data(ILI9341_GMCTRP1, (uint8_t[]){0x0F, 0x31, 0x2B, 0x0C, 0x0E, 0x08, 0x4E, 0xF1, 0x37, 0x07, 0x10, 0x03, 0x0E, 0x09, 0x00}, 15); // Positive Gamma Correction
    lcd_write_command_with_data(ILI9341_GMCTRN1, (uint8_t[]){0x00, 0x0E, 0x14, 0x03, 0x11, 0x07, 0x31, 0xC1, 0x48, 0x08, 0x0F, 0x0C, 0x31, 0x36, 0x0F}, 15); // Negative Gamma Correction

    lcd_write_command_with_data(ILI9341_SLPOUT, (uint8_t[]){0x00}, 1); // Exit Sleep
    // lcd_write_command(ILI9341_SLPOUT); // Exit Sleep
    sleep_ms(150);
    lcd_write_command_with_data(ILI9341_DISPON, (uint8_t[]){0x00}, 1); // Display on
    // lcd_write_command(ILI9341_DISPON); // Display on
    sleep_ms(150);

    _width = ILI9341_TFTWIDTH;
    _height = ILI9341_TFTHEIGHT;

    puts("lcd_init done");
}

void lcd_set_rotation(uint8_t m)
{
    puts("lcd_set_rotation start");

    rotation = m % 4; // can't be higher than 3

    puts("lcd_set_rotation calculating parameters");

    switch (rotation)
    {
        case 0:
            puts("lcd_set_rotation case 0");
            m = (MADCTL_MX | MADCTL_BGR);
            _width = ILI9341_TFTWIDTH;
            _height = ILI9341_TFTHEIGHT;
            break;
        case 1:
            puts("lcd_set_rotation case 1");
            m = (MADCTL_MV | MADCTL_BGR);
            _width = ILI9341_TFTHEIGHT;
            _height = ILI9341_TFTWIDTH;
            break;
        case 2:
            puts("lcd_set_rotation case 2");
            m = (MADCTL_MY | MADCTL_BGR);
            _width = ILI9341_TFTWIDTH;
            _height = ILI9341_TFTHEIGHT;
            break;
        case 3:
            puts("lcd_set_rotation case 3");
            m = (MADCTL_MX | MADCTL_MY | MADCTL_MV | MADCTL_BGR);
            _width = ILI9341_TFTHEIGHT;
            _height = ILI9341_TFTWIDTH;
            break;
    }

    puts("lcd_set_rotation sending command");

    lcd_write_command_with_data(ILI9341_MADCTL, &m, 1);
    
    puts("lcd_set_rotation done");
}

void lcd_set_window(uint16_t x, uint16_t y, uint16_t w, uint16_t h)
{
    uint32_t xa = ((uint32_t)x << 16) | (x + w - 1);
    uint32_t ya = ((uint32_t)y << 16) | (y + h - 1);

    xa = __builtin_bswap32(xa);
    ya = __builtin_bswap32(ya);

    lcd_write_command(ILI9341_CASET);
    lcd_write_data((uint8_t *)&xa, sizeof(xa));

    lcd_write_command(ILI9341_PASET);
    lcd_write_data((uint8_t *)&ya, sizeof(ya));

    lcd_write_command(ILI9341_RAMWR);
}

void lcd_write_bitmap(uint16_t x, uint16_t y, uint16_t w, uint16_t h, uint16_t *bitmap)
{
    lcd_select();
    lcd_set_window(x, y, w, h);
    lcd_data_mode();
    spi_set_format(TFT_SPI, 16, SPI_CPOL_1, SPI_CPOL_1, SPI_MSB_FIRST);

#if TFT_USE_DMA
    if (!dma_channel_is_busy(dma_tx))
    {
        dma_channel_configure(dma_tx, &dma_cfg, &spi_get_hw(TFT_SPI)->dr, bitmap, w * h, true);
    }
    // dma_channel_wait_for_finish_blocking(dma_tx);
#else
    spi_write16_blocking(TFT_SPI, bitmap, w * h);
    lcd_deselect();
#endif
}

void lcd_fill_rect(uint16_t x, uint16_t y, uint16_t w, uint16_t h, uint16_t color)
{
    lcd_select();
    lcd_set_window(x, y, w, h);
    lcd_data_mode();
    spi_set_format(TFT_SPI, 16, SPI_CPOL_1, SPI_CPOL_1, SPI_MSB_FIRST);

    uint16_t color_buffer[w]; // Buffer for one line of pixels
    for (size_t i = 0; i < w; i++)
        color_buffer[i] = color;

    for (size_t row = 0; row < h; row++)
    {
        // #if TFT_USE_DMA
        //         dma_channel_configure(dma_tx, &dma_cfg, &spi_get_hw(TFT_SPI)->dr, &color, w * h, false);
        //         // dma_channel_wait_for_finish_blocking(dma_tx);
        // #else
        spi_write16_blocking(TFT_SPI, color_buffer, w);
    }

    lcd_deselect();
}

void lcd_clear_screen(uint16_t color)
{
    lcd_fill_rect(0, 0, _width, _height, color);
}