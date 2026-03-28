#pragma once
#include <stdint.h>

struct Pattern {
    uint8_t r;
    uint8_t g;
    uint8_t b;
    uint16_t ms_per_led;
    uint8_t spacing;
    int8_t direction;
    uint8_t offset;
    uint8_t shape;
};
