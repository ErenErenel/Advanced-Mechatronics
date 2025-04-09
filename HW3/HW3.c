#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/adc.h"

int main() {
    stdio_init_all();
    while (!stdio_usb_connected()) {
        sleep_ms(100);
    }
    printf("Start!\n");

    // Initialize LED
    gpio_init(15);
    gpio_set_dir(15, GPIO_OUT);

    // Initialize button
    gpio_init(2);
    gpio_set_dir(2, GPIO_IN);

    // Initialize ADC
    adc_init();
    adc_gpio_init(26);      // ADC0 = GPIO 26
    adc_select_input(0);    // Select ADC0
    gpio_put(15, 1);  // Turn on LED
    printf("Waiting for button press...\n");

    // Wait for button press (active LOW)
    while (gpio_get(2)) {
        sleep_ms(50);
    }

    gpio_put(15, 0);  // Turn off LED
    printf("Button pressed.\n");

    while (1) {

        int num_samples = 0;

        while (1) {
            printf("Enter number of samples (1-100): ");
            int result = scanf("%d", &num_samples);
        
            if (result == 1 && num_samples >= 1 && num_samples <= 100) {
                break;  // Valid number
            } else {
                printf("Invalid input.\n");
            }

        }

        printf("Taking %d samples at 100Hz...\n", num_samples);

        for (int i = 0; i < num_samples; i++) {
            uint16_t raw = adc_read();
            float voltage = raw * 3.3f / 4095.0f;
            printf("Sample %d: %.2f V\n", i + 1, voltage);
            sleep_ms(10);
        }

        printf("Done!\n\n");
        sleep_ms(500);
    }
}