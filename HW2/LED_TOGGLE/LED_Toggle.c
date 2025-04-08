#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/gpio.h"

#define BUTTON_PIN 2     // GPIO pin for the button
#define LED_PIN    3     // GPIO pin for the LED

volatile uint32_t press_count = 0;
volatile bool led_on = false;

void gpio_callback(uint gpio, uint32_t events) {
    if (gpio == BUTTON_PIN && (events & GPIO_IRQ_EDGE_RISE)) {
        press_count++;
        led_on = !led_on;
        gpio_put(LED_PIN, led_on);  // Toggle the LED
        printf("Button pressed %lu times\n", press_count);
    }
}

int main() {
    stdio_init_all();

    // Setup button
    gpio_init(BUTTON_PIN);
    gpio_set_dir(BUTTON_PIN, GPIO_IN);
    gpio_pull_down(BUTTON_PIN);  
    gpio_set_irq_enabled_with_callback(BUTTON_PIN, GPIO_IRQ_EDGE_RISE, true, &gpio_callback);

    // Setup LED
    gpio_init(LED_PIN);
    gpio_set_dir(LED_PIN, GPIO_OUT);
    gpio_put(LED_PIN, 0);  // Start with LED off

    printf("Ready. Waiting for button presses...\n");

    while (true) {
        tight_loop_contents(); 
    }
}
