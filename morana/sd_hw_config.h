
/* hw_config.c
Copyright 2021 Carl John Kugler III

Licensed under the Apache License, Version 2.0 (the License); you may not use
this file except in compliance with the License. You may obtain a copy of the
License at

   http://www.apache.org/licenses/LICENSE-2.0
Unless required by applicable law or agreed to in writing, software distributed
under the License is distributed on an AS IS BASIS, WITHOUT WARRANTIES OR
CONDITIONS OF ANY KIND, either express or implied. See the License for the
specific language governing permissions and limitations under the License.
*/

/*
This file should be tailored to match the hardware design.

See
https://github.com/carlk3/no-OS-FatFS-SD-SDIO-SPI-RPi-Pico/tree/main#customizing-for-the-hardware-configuration
*/

#include <pico/stdlib.h>
#include <pico/stdio.h>
#include <string.h>
#include "my_debug.h"
#include "f_util.h"
#include "ff.h"
#include "libs/storage/FatFs_SPI/sd_driver/sd_card.h"
#include "diskio.h"
#include "rtc.h"
#include "config.h"

void spi_dma_isr();

/* Configuration of hardware SPI object */
static spi_t spis[] = {
    {
        .hw_inst = SD_SPI,   // SPI component
        .sck_gpio = SD_SCLK, // GPIO number (not Pico pin number)
        .mosi_gpio = SD_MOSI,
        .miso_gpio = SD_MISO,
        //.baud_rate = 125 * 1000 * 1000 / 8  // 15625000 Hz
        //.baud_rate = 125 * 1000 * 1000 / 6  // 20833333 Hz
        // .baud_rate = 125 * 1000 * 1000 / 4, // 31250000 Hz
        //.baud_rate = 125 * 1000 * 1000 / 2  // 62500000 Hz
        .baud_rate = 12500 * 1000,
        .dma_isr = spi_dma_isr // DMA ISR function pointer
    }
};

static sd_card_t sd_cards[] = {
    {.pcName = "0:",
     .spi = &spis[0],
     .ss_gpio = SD_CS,
     .use_card_detect = false,
     .m_Status = STA_NOINIT}};

void spi_dma_isr()
{
    spi_irq_handler(&spis[0]);
}

size_t sd_get_num()
{
    return count_of(sd_cards);
}

sd_card_t *sd_get_by_num(size_t num)
{
    if (num <= sd_get_num())
    {
        return &sd_cards[num];
    }
    else
    {
        return NULL;
    }
}

size_t spi_get_num()
{
    return count_of(spis);
}

spi_t *spi_get_by_num(size_t num)
{
    if (num <= sd_get_num())
    {
        return &spis[num];
    }
    else
    {
        return NULL;
    }
}