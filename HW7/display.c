#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/i2c.h"
#include "hardware/adc.h"
#include "hardware/timer.h"
#include "ssd1306.h"
#include "font.h"

#define I2C_PORT i2c0
#define I2C_SDA 16
#define I2C_SCL 17

// === Text Functions ===
void drawLetter(int x, int y, char letter);
void drawMessage(int x, int y, char *message);

int main() {
    stdio_init_all();

    // Initialize I2C
    i2c_init(I2C_PORT, 400 * 1000); // 400kHz
    gpio_set_function(I2C_SDA, GPIO_FUNC_I2C);
    gpio_set_function(I2C_SCL, GPIO_FUNC_I2C);
    gpio_pull_up(I2C_SDA);
    gpio_pull_up(I2C_SCL);

    // Initialize OLED display
    ssd1306_setup();

    // Initialize ADC
    adc_init();
    adc_gpio_init(26);  // GPIO26 = ADC0
    adc_select_input(0);

    unsigned int t1 = to_us_since_boot(get_absolute_time());

    while (1) {
        // Measure FPS
        unsigned int t2 = to_us_since_boot(get_absolute_time());
        float fps = 1e6f / (t2 - t1);
        t1 = t2;

        // Read ADC value and convert to voltage
        uint16_t adc_raw = adc_read();
        float volts = 3.3f * adc_raw / 4095.0f;

        char adc_message[50];
        char fps_message[50];
        sprintf(adc_message, "ADC0 = %.2f V", volts);
        sprintf(fps_message, "FPS = %.2f", fps);

        // Draw to OLED
        ssd1306_clear();
        drawMessage(0, 0, adc_message);    // Top row
        drawMessage(0, 24, fps_message);   // Bottom row (y=24 because screen is 32 tall, each letter 8 tall)
        ssd1306_update();
    }
}

// === Text Drawing Functions ===

// Draw a single letter at (x,y)
void drawLetter(int x, int y, char letter) {
    if (letter < 0x20 || letter > 0x7F) return; // Ignore unsupported characters
    for (int i = 0; i < 5; i++) { // 5 columns
        char col = ASCII[letter - 0x20][i];
        for (int j = 0; j < 8; j++) { // 8 rows
            int pixelOn = (col >> j) & 0x01;
            ssd1306_drawPixel(x + i, y + j, pixelOn);
        }
    }
}

// Draw a string starting at (x,y)
void drawMessage(int x, int y, char *message) {
    int i = 0;
    while (message[i] != '\0') {
        drawLetter(x + (6 * i), y, message[i]);
        i++;
    }
}
