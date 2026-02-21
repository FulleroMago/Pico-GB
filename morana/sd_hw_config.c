
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

#include "hw_config.h"
#include "config.h"
#include "diskio.h"

void spi_dma_isr();

/* Configuration of hardware SPI object */
static spi_t spi = {
    .hw_inst = SD_SPI,   // SPI component
    .sck_gpio = SD_SCLK, // GPIO number (not Pico pin number)
    .mosi_gpio = SD_MOSI,
    .miso_gpio = SD_MISO,
    //.baud_rate = 125 * 1000 * 1000 / 8  // 15625000 Hz
    //.baud_rate = 125 * 1000 * 1000 / 6  // 20833333 Hz
    .baud_rate = 125 * 1000 * 1000 / 4, // 31250000 Hz
    //.baud_rate = 125 * 1000 * 1000 / 2  // 62500000 Hz
    .dma_isr = spi_dma_isr // DMA ISR function pointer
};

void spi_dma_isr()
{
    spi_irq_handler(&spi);
}

static sd_card_t sd_card = {
     .pcName = "0:",
     .spi = &spi,
     .ss_gpio = 13,
     .use_card_detect = false,
     .m_Status = STA_NOINIT
    };

/* ********************************************************************** */

size_t sd_get_num() { return 1; }

/**
 * @brief Get a pointer to an SD card object by its number.
 *
 * @param[in] num The number of the SD card to get.
 *
 * @return A pointer to the SD card object, or @c NULL if the number is invalid.
 */
sd_card_t *sd_get_by_num(size_t num) {
    if (0 == num) {
        // The number 0 is a valid SD card number.
        // Return a pointer to the sd_card object.
        return &sd_card;
    } else {
        // The number is invalid. Return @c NULL.
        return NULL;
    }
}

size_t spi_get_num()
{
    return 1;
}

spi_t *spi_get_by_num(size_t num)
{
    return &spi;
}

/* [] END OF FILE */
