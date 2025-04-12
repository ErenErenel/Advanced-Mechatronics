#include <stdio.h>
#include <math.h>  // for sin
#include "pico/stdlib.h"
#include "hardware/spi.h"

#define SPI_PORT spi0
#define PIN_MISO 16
#define PIN_CS   17
#define PIN_SCK  18
#define PIN_MOSI 19

#define NUM_SAMPLES 100
#define DAC_MAX 1023  // 10-bit DAC

uint16_t sine_wave[NUM_SAMPLES];
uint16_t triangle_wave[NUM_SAMPLES];

static inline void cs_select(uint cs_pin) {
    asm volatile("nop \n nop \n nop");
    gpio_put(cs_pin, 0);
    asm volatile("nop \n nop \n nop");
}

static inline void cs_deselect(uint cs_pin) {
    asm volatile("nop \n nop \n nop");
    gpio_put(cs_pin, 1);
    asm volatile("nop \n nop \n nop");
}

void generate_waveforms() {
    for (int i = 0; i < NUM_SAMPLES; i++) {
        float angle = 2.0f * 3.1415926f * i / NUM_SAMPLES;
        float sine_val = (sinf(angle) + 1.0f) / 2.0f; // Normalize to 0–1
        float tri_val = (i < NUM_SAMPLES / 2)
                        ? (float)i / (NUM_SAMPLES / 2)
                        : (float)(NUM_SAMPLES - i) / (NUM_SAMPLES / 2);
        sine_wave[i] = (uint16_t)(sine_val * DAC_MAX);
        triangle_wave[i] = (uint16_t)(tri_val * DAC_MAX);
    }
}

void writeDac(uint16_t valA, uint16_t valB) {
    uint16_t commandA = (0 << 15) | (1 << 14) | (1 << 13) | (1 << 12) | (valA << 2);
    uint16_t commandB = (1 << 15) | (1 << 14) | (1 << 13) | (1 << 12) | (valB << 2);

    uint8_t dataA[2] = { (commandA >> 8) & 0xFF, commandA & 0xFF };
    uint8_t dataB[2] = { (commandB >> 8) & 0xFF, commandB & 0xFF };

    cs_select(PIN_CS);
    spi_write_blocking(SPI_PORT, dataA, 2);
    cs_deselect(PIN_CS);
    sleep_us(5);
    cs_select(PIN_CS);
    spi_write_blocking(SPI_PORT, dataB, 2);
    cs_deselect(PIN_CS);
}

int main() {
    stdio_init_all();
    spi_init(SPI_PORT, 100000);  // max nscope at 100k
    gpio_set_function(PIN_MISO, GPIO_FUNC_SPI);
    gpio_set_function(PIN_CS,   GPIO_FUNC_SIO);
    gpio_set_function(PIN_SCK,  GPIO_FUNC_SPI);
    gpio_set_function(PIN_MOSI, GPIO_FUNC_SPI);

    gpio_init(PIN_CS);
    gpio_set_dir(PIN_CS, GPIO_OUT);
    gpio_put(PIN_CS, 1);

    generate_waveforms();

    int index_sine = 0;
    int index_tri = 0;

    while (true) {
        writeDac(sine_wave[index_sine], triangle_wave[index_tri]);

        index_sine = (index_sine + 2) % NUM_SAMPLES;   // 2 steps per cycle → 2Hz
        index_tri  = (index_tri + 1) % NUM_SAMPLES;    // 1 step per cycle → 1Hz

        sleep_ms(10);  // 100Hz update rate
    }
}
