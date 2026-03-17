#pragma once

#define ILI9341_TFTWIDTH 240  // ILI9341 max TFT width
#define ILI9341_TFTHEIGHT 320 // ILI9341 max TFT height

#define ILI9341_MANUFACTURER_ACCESS 0xEF     // Manufacturer Command Access Protect
#define ILI9341_POWER_CONTROL_B 0xCF         // Power Control B
#define ILI9341_POWER_ON_SEQ_CONTROL 0xED    // Power On Sequence Control
#define ILI9341_DRIVER_TIMING_CTRL_A 0xE8    // Driver Timing Control A
#define ILI9341_POWER_CONTROL_A 0xCB         // Power Control A
#define ILI9341_PUMP_RATIO_CONTROL 0xF7      // Pump Ratio Control
#define ILI9341_DRIVER_TIMING_CTRL_B 0xEA    // Driver Timing Control B
#define ILI9341_3GAMMA_FUNCTION_DISABLE 0xF2 // 3Gamma Function Disable

#define ILI9341_PWCTR1 0xC0   // Power Control 1
#define ILI9341_PWCTR2 0xC1   // Power Control 2
#define ILI9341_VMCTR1 0xC5   // VCOM Control 1
#define ILI9341_VMCTR2 0xC7   // VCOM Control 2
#define ILI9341_MADCTL 0x36   // Memory Access Control
#define ILI9341_VSCRSADD 0x37 // Vertical Scroll Start Address
#define ILI9341_PIXFMT 0x3A   // Pixel Format Set
#define ILI9341_FRMCTR1 0xB1  // Frame Rate Control (Normal Mode)
#define ILI9341_DFUNCTR 0xB6  // Display Function Control
#define ILI9341_GAMMASET 0x26 // Gamma Set
#define ILI9341_GMCTRP1 0xE0  // Positive Gamma Correction
#define ILI9341_GMCTRN1 0xE1  // Negative Gamma Correction
#define ILI9341_SLPOUT 0x11   // Sleep Out
#define ILI9341_DISPON 0x29   // Display ON

#define ILI9341_CASET 0x2A // Column Address Set
#define ILI9341_PASET 0x2B // Page Address Set
#define ILI9341_RAMWR 0x2C // Memory Write
#define ILI9341_RAMRD 0x2E // Memory Read

#define ILI9341_CMDLIST_END 0x00 // End of command list marker

#define MADCTL_MY 0x80  // Bottom to top
#define MADCTL_MX 0x40  // Right to left
#define MADCTL_MV 0x20  // Reverse Mode
#define MADCTL_ML 0x10  // LCD refresh Bottom to top
#define MADCTL_RGB 0x00 // Red-Green-Blue pixel order
#define MADCTL_BGR 0x08 // Blue-Green-Red pixel order
#define MADCTL_MH 0x04  // LCD refresh right to left

void lcd_init();
bool lcd_is_busy();

void lcd_wait_until_not_busy();
void lcd_set_rotation(uint8_t m);
void lcd_write_bitmap(uint16_t x, uint16_t y, uint16_t w, uint16_t h, const uint16_t *bitmap);
void lcd_fill_rect(uint16_t x, uint16_t y, uint16_t w, uint16_t h, uint16_t color);
void lcd_clear_screen(uint16_t color);
