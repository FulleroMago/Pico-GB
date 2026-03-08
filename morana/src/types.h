#pragma once
#include <stdint.h>

typedef struct
{
    uint16_t x; // x pozice
    uint16_t y; // y pozice
} vector2d;

typedef struct
{
    uint16_t width;        // šířka obdélníku
    uint16_t height;       // výška obdélníku
} size2d;

typedef struct
{
    vector2d position;     // pozice levého horního rohu obdélníku
    size2d size;           // rozměry obdélníku
} box;