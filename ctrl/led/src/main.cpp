#include <Arduino.h>
#include <FastLED.h>
#include "protocol.h"

#define LED_PIN     5
#define N_LEDS    60
#define N_STEPS_PER_LED 10 // divide each led into this many steps
#define REFRESH_MS   20
#define PERIOD_MS   200
#define PULSE_MS    1000
#define BPM 80
#define WAVE_WIDTH 6
#define N_PATTERNS 3
#define SHAPE_SIZE 4
#define N_SHAPES 2

const uint8_t SHAPES[N_SHAPES][SHAPE_SIZE] = {
    {16, 128, 196, 255},
    {8, 32, 128, 255},
};
const uint16_t N_TOTAL_STEPS = N_LEDS * N_STEPS_PER_LED;

void update_pattern_leds(uint millis, Pattern pattern, CRGB *leds);
CRGB combine_inks(CRGB c1, CRGB c2);

Pattern make_pattern(CRGB color, uint16_t ms_per_led, uint8_t spacing, int8_t direction, uint8_t offset, uint8_t shape) {
    return {color.r, color.g, color.b, ms_per_led, spacing, direction, offset, shape};
}

Pattern patterns[N_PATTERNS] = {
    make_pattern(CRGB::Red, 200, 12, 1, 0, 0),
    make_pattern(CRGB::Yellow, 100, 24, 1, 0, 1),
    make_pattern(CRGB::Blue, 100, 24, -1, 0, 1),
};
CRGB leds[N_LEDS];
CRGB pattern_leds[N_PATTERNS][N_LEDS];

void setup() {
    FastLED.addLeds<WS2812B, LED_PIN, GRB>(leds, N_LEDS);
    FastLED.setBrightness(255);

    fill_solid(leds, N_LEDS, CRGB::Black);
}

void loop() {
    EVERY_N_MILLISECONDS(REFRESH_MS) {
        fill_solid(leds, N_LEDS, CRGB::Black);

        uint32_t ms = millis();
        for (int i = 0; i < N_PATTERNS; i++) {
            update_pattern_leds(ms, patterns[i], leds);
        }

        nscale8_video(leds, N_LEDS, 200);

        FastLED.show();
    }
}

void update_pattern_leds(uint millis, Pattern pattern, CRGB *leds) {
    uint16_t step = (millis * pattern.direction) / (pattern.ms_per_led / N_STEPS_PER_LED); // Aka divide by ms_per_step
    uint8_t step_offset = step % N_STEPS_PER_LED;
    const uint16_t step_pattern_sz = (SHAPE_SIZE + 1) * N_STEPS_PER_LED;
    static uint8_t step_pattern[step_pattern_sz];
    memset(step_pattern, 0, step_pattern_sz);
    for (int i = 0; i < SHAPE_SIZE * N_STEPS_PER_LED; i++) {
        uint8_t shape_idx = i / N_STEPS_PER_LED;
        if (pattern.direction == -1) {
            shape_idx = SHAPE_SIZE - 1 - shape_idx;
        }
        step_pattern[i + step_offset] = SHAPES[pattern.shape][shape_idx];
    }
    uint16_t led_pattern[SHAPE_SIZE + 1] = {};
    for (int i = 0; i < step_pattern_sz; i++) {
        led_pattern[i / N_STEPS_PER_LED] += step_pattern[i];
    }
    for (int i = 0; i < SHAPE_SIZE + 1; i++) {
        led_pattern[i] = led_pattern[i] / N_STEPS_PER_LED;
    }

    uint8_t led = (step / N_STEPS_PER_LED) % N_LEDS;
    uint8_t n_pats = N_LEDS / pattern.spacing;

    for (int i = 0; i < n_pats; i++) {
        for (int j = 0; j < SHAPE_SIZE + 1; j++) {
            uint8_t led_idx = (led + i * pattern.spacing + pattern.offset + j) % N_LEDS;
            if (led_pattern[j] > 0) {
                // leds[led_idx] += CRGB(pattern.color).nscale8(led_pattern[j]); // ligth-additive
                leds[led_idx] = combine_inks(leds[led_idx], CRGB(pattern.r, pattern.g, pattern.b).nscale8(led_pattern[j]));
            }
        }
    }
}

CRGB combine_inks(CRGB c1, CRGB c2) {
    if (!c1) return c2;
    if (!c2) return c1;

    CHSV h1 = rgb2hsv_approximate(c1);
    CHSV h2 = rgb2hsv_approximate(c2);

    // Blend hue along the shorter arc, weighted by value
    int16_t diff = (int16_t)h2.hue - h1.hue;
    if (diff > 128) diff -= 256;
    if (diff < -128) diff += 256;
    uint16_t total = (uint16_t)h1.val + h2.val;
    uint8_t hue = h1.hue + (int8_t)((int32_t)diff * h2.val / total);

    uint8_t sat = max(h1.sat, h2.sat);
    uint8_t val = qadd8(h1.val, h2.val);

    return CHSV(hue, sat, val);
}