#pragma once

#define USE_SD_CARD 1

#define TFT_SPI spi0
#define TFT_SCLK 18
#define TFT_MOSI 19
#define TFT_DC 17
#define TFT_RST 20
#define TFT_CS 21
#define TFT_USE_DMA 1

#define TFT_ROTATION 3

// SD card settings
#define SD_SPI spi1
#define SD_SCLK 10
#define SD_MOSI 11
#define SD_MISO 12
#define SD_CS 13
// #define SD_BAUDRATE 25000000 // 25 MHz

// Talčítka - GPIO Connections.
#define GPIO_CONTROL_RIGHT 2
#define GPIO_CONTROL_LEFT 3
#define GPIO_CONTROL_UP 4
#define GPIO_CONTROL_DOWN 5

#define GPIO_CONTROL_B 9
#define GPIO_CONTROL_A 8
#define GPIO_CONTROL_Y 7
#define GPIO_CONTROL_X 6

#define GPIO_CONTROL_START 26
#define GPIO_CONTROL_SELECT 27
#define GPIO_CONTROL_MENU 28

// Overclock settings
#define OVERCLOCK_ENABLED 1
#define VCO 1596 * 1000 * 1000 /* 266MHz */
#define DIV1 6
#define DIV2 1