#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include "pico/stdlib.h"
#include "hardware/pio.h"
#include "hardware/clocks.h"
#include "hardware/pwm.h"
#include "ws2812.pio.h"

#define IS_RGBW false
#define NUM_PIXELS 4
#define WS2812_PIN 13
#define SERVO_PIN 21

#if WS2812_PIN >= NUM_BANK0_GPIOS
#error Attempting to use a pin>=32 on a platform that does not support it
#endif

typedef struct {
    unsigned char r;
    unsigned char g;
    unsigned char b;
} wsColor;

static inline void put_pixel(PIO pio, uint sm, uint32_t pixel_grb) {
    pio_sm_put_blocking(pio, sm, pixel_grb << 8u);
}

static inline uint32_t urgb_u32(uint8_t r, uint8_t g, uint8_t b) {
    return ((uint32_t)(g) << 8) | ((uint32_t)(r) << 16) | (uint32_t)(b);
}

wsColor HSBtoRGB(float hue, float sat, float brightness) {
    float red = 0.0, green = 0.0, blue = 0.0;
    if (sat == 0.0) {
        red = green = blue = brightness;
    } else {
        if (hue == 360.0) hue = 0;
        int slice = hue / 60.0;
        float hue_frac = (hue / 60.0) - slice;
        float aa = brightness * (1.0 - sat);
        float bb = brightness * (1.0 - sat * hue_frac);
        float cc = brightness * (1.0 - sat * (1.0 - hue_frac));

        switch (slice) {
            case 0: red = brightness; green = cc; blue = aa; break;
            case 1: red = bb; green = brightness; blue = aa; break;
            case 2: red = aa; green = brightness; blue = cc; break;
            case 3: red = aa; green = bb; blue = brightness; break;
            case 4: red = cc; green = aa; blue = brightness; break;
            case 5: red = brightness; green = aa; blue = bb; break;
            default: break;
        }
    }

    wsColor c;
    c.r = red * 255.0;
    c.g = green * 255.0;
    c.b = blue * 255.0;
    return c;
}

void servo_init(uint pin) {
    gpio_set_function(pin, GPIO_FUNC_PWM);
    uint slice = pwm_gpio_to_slice_num(pin);
    pwm_set_clkdiv(slice, 100.0f);
    pwm_set_wrap(slice, 30000);
    pwm_set_enabled(slice, true);
}

void servo_set_angle(uint pin, float angle) {
    if (angle < 0.0f) angle = 0.0f;
    if (angle > 180.0f) angle = 180.0f;
    float pulse_ms = 0.5f + (angle / 180.0f) * 2.0f;
    uint16_t level = (uint16_t)((pulse_ms / 20.0f) * 30000.0f);
    pwm_set_gpio_level(pin, level);
}

int main() {
    stdio_init_all();
    servo_init(SERVO_PIN);

    PIO pio;
    uint sm;
    uint offset;
    bool success = pio_claim_free_sm_and_add_program_for_gpio_range(&ws2812_program, &pio, &sm, &offset, WS2812_PIN, 1, true);
    hard_assert(success);
    ws2812_program_init(pio, sm, offset, WS2812_PIN, 800000, IS_RGBW);

    float hue_base = 0.0f;

    while (true) {
        for (int step = 0; step <= 250; step++) {
            float angle = (180.0f * step) / 250.0f;
            servo_set_angle(SERVO_PIN, angle);

            for (int i = 0; i < NUM_PIXELS; i++) {
                float hue = fmodf(hue_base + (i * 12.0f), 360.0f);
                wsColor color = HSBtoRGB(hue, 1.0f, 0.1f);
                put_pixel(pio, sm, urgb_u32(color.r, color.g, color.b));
            }
            hue_base = fmodf(hue_base + 1.0f, 360.0f);
            sleep_ms(20);
        }

        for (int step = 0; step < 20; step++) {
            float angle = 180.0f * (1.0f - ((float)step / 20.0f));
            servo_set_angle(SERVO_PIN, angle);

            for (int i = 0; i < NUM_PIXELS; i++) {
                float hue = fmodf(hue_base + (i * 4.0f), 360.0f);
                wsColor color = HSBtoRGB(hue, 1.0f, 0.1f);
                put_pixel(pio, sm, urgb_u32(color.r, color.g, color.b));
            }
            hue_base = fmodf(hue_base + 1.0f, 360.0f);
            sleep_ms(10);
        }
    }

    pio_remove_program_and_unclaim_sm(&ws2812_program, pio, sm, offset);
}
