#include <stdio.h>
#include <math.h>
#include "pico/stdlib.h"
#include "hardware/spi.h"

#define SPI_PORT spi0
#define PIN_MISO 16
#define PIN_CS_DAC 17
#define PIN_SCK  18
#define PIN_MOSI 19
#define PIN_CS_SRAM 13

#define NUM_SAMPLES 1000
#define DAC_MAX 1023

// Struct to cast between float and uint32_t
union FloatInt {
    float f;
    uint32_t i;
};

// Select/Deselect functions
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

/*
 * The 23K256 SRAM has three operation modes:
 * 1. Byte mode (0x00): each read/write accesses only one byte.
 * 2. Page mode (0x80): wraps around every 1024 bytes.
 * 3. Sequential mode (0x40): wraps across entire 32kB space.
 * This program uses sequential mode, configured via the WRSR (0x01) instruction.
 */

 
void spi_ram_init() {
    spi_init(SPI_PORT, 1 * 1000 * 1000);
    gpio_set_function(PIN_MISO, GPIO_FUNC_SPI);
    gpio_set_function(PIN_MOSI, GPIO_FUNC_SPI);
    gpio_set_function(PIN_SCK, GPIO_FUNC_SPI);

    gpio_init(PIN_CS_DAC);
    gpio_set_dir(PIN_CS_DAC, GPIO_OUT);
    gpio_put(PIN_CS_DAC, 1);

    gpio_init(PIN_CS_SRAM);
    gpio_set_dir(PIN_CS_SRAM, GPIO_OUT);
    gpio_put(PIN_CS_SRAM, 1);

    // Set to sequential mode using WRSR command
    uint8_t init_cmd[] = {0x01, 0x40};
    cs_select(PIN_CS_SRAM);
    spi_write_blocking(SPI_PORT, init_cmd, 2);
    cs_deselect(PIN_CS_SRAM);
}

void spi_ram_write_float(uint16_t addr, float val) {
    union FloatInt fi;
    fi.f = val;

    uint8_t cmd[7] = {
        0x02, // WRITE instruction (from datasheet)
        (addr >> 8) & 0xFF, addr & 0xFF,
        (fi.i >> 24) & 0xFF,
        (fi.i >> 16) & 0xFF,
        (fi.i >> 8) & 0xFF,
        fi.i & 0xFF
    };

    cs_select(PIN_CS_SRAM);
    spi_write_blocking(SPI_PORT, cmd, 7);
    cs_deselect(PIN_CS_SRAM);
}

float spi_ram_read_float(uint16_t addr) {
    uint8_t cmd[3] = {
        0x03, // READ instruction (from datasheet)
        (addr >> 8) & 0xFF, addr & 0xFF
    };

    uint8_t result[4];

    cs_select(PIN_CS_SRAM);
    spi_write_blocking(SPI_PORT, cmd, 3);
    spi_read_blocking(SPI_PORT, 0x00, result, 4);
    cs_deselect(PIN_CS_SRAM);

    union FloatInt fi;
    fi.i = ((uint32_t)result[0] << 24) |
           ((uint32_t)result[1] << 16) |
           ((uint32_t)result[2] << 8) |
           result[3];

    return fi.f;
}

void writeDac(float voltage) {
    uint16_t val = (uint16_t)((voltage / 3.3f) * DAC_MAX);

    uint16_t commandA = (0 << 15) | (1 << 14) | (1 << 13) | (1 << 12) | (val << 2);
    uint8_t dataA[2] = { (commandA >> 8) & 0xFF, commandA & 0xFF };

    cs_select(PIN_CS_DAC);
    spi_write_blocking(SPI_PORT, dataA, 2);
    cs_deselect(PIN_CS_DAC);
}

int main() {
    stdio_init_all();
    sleep_ms(500); // wait for terminal to catch up

    spi_ram_init();

    // Generate sine wave and write to external RAM
    for (int i = 0; i < NUM_SAMPLES; i++) {
        float angle = 2.0f * 3.1415926f * i / NUM_SAMPLES;
        float val = (sinf(angle) + 1.0f) * (3.3f / 2.0f); // scale 0–3.3V
        spi_ram_write_float(i * 4, val);
    }

    int index = 0;
    while (true) {
        float val = spi_ram_read_float(index * 4);
        writeDac(val);
        index = (index + 1) % NUM_SAMPLES;
        sleep_ms(1); // 1 ms per sample = 1 Hz waveform
    }
}
