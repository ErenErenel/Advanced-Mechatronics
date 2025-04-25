#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/i2c.h"

#define I2C_PORT i2c0
#define I2C_SDA 16
#define I2C_SCL 17
#define MCP23008_ADDR 0x20

#define IODIR 0x00
#define GPIO  0x09
#define OLAT  0x0A

void setPin(uint8_t address, uint8_t reg, uint8_t value) {
    uint8_t buf[] = {reg, value};
    i2c_write_blocking(I2C_PORT, address, buf, 2, false);
}

// General-purpose read function
uint8_t readPin(uint8_t address, uint8_t reg) {
    uint8_t value;
    i2c_write_blocking(I2C_PORT, address, &reg, 1, true);  // Set register address
    i2c_read_blocking(I2C_PORT, address, &value, 1, false); // Read data
    return value;
}

int main() {
    stdio_init_all();

    // Initialize I2C
    i2c_init(I2C_PORT, 400 * 1000);
    gpio_set_function(I2C_SDA, GPIO_FUNC_I2C);
    gpio_set_function(I2C_SCL, GPIO_FUNC_I2C);
    gpio_pull_up(I2C_SDA);
    gpio_pull_up(I2C_SCL);

    // Initialize Pico built-in heartbeat LED (green LED on GP25)
    const uint LED_PIN = 25;
    gpio_init(LED_PIN);
    gpio_set_dir(LED_PIN, GPIO_OUT);

    // Configure MCP23008: GP7 = output, GP0 = input, others = input
    setPin(MCP23008_ADDR, IODIR, 0b01111111);  // GP7 output (bit 7 = 0), GP0 input (bit 0 = 1)

    while (1) {
        // Blink Pico onboard green LED (heartbeat)
        gpio_put(LED_PIN, 1);
        sleep_ms(100);
        gpio_put(LED_PIN, 0);
        sleep_ms(100);

        // Read button state from GP0
        uint8_t gpio_val = readPin(MCP23008_ADDR, GPIO);

        // Print out GPIO 0 state for debugging
        if (gpio_val & 0b00000001) {
            printf("Button NOT pressed (GP0 = HIGH)\n");
        } else {
            printf("Button PRESSED (GP0 = LOW)\n");
        }

        // Control GP7 based on button press
        if ((gpio_val & 0b00000001) == 0) {
            // Button pressed : Turn on GP7 LED
            setPin(MCP23008_ADDR, OLAT, 0b10000000);
        } else {
            // Button not pressed : Turn off GP7 LED
            setPin(MCP23008_ADDR, OLAT, 0x00);
        }
    }
}
