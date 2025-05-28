#include <stdio.h>
#include "pico/stdlib.h"
#include "pico/stdio_usb.h"
#include "pico/multicore.h"
#include "hardware/adc.h"
#include "hardware/gpio.h"

#define FLAG_VALUE    123
#define FLAG_ADC      100
#define FLAG_LED_ON   101
#define FLAG_LED_OFF  102

#define LED_PIN       15
#define ADC_PIN       26  

volatile uint16_t adc_result = 0;

void core1_entry() {
    // Handshake
    multicore_fifo_push_blocking(FLAG_VALUE);
    uint32_t g = multicore_fifo_pop_blocking();
    if (g == FLAG_VALUE)
        printf("Core 1: Handshake successful.\n");

   
    gpio_init(LED_PIN);
    gpio_set_dir(LED_PIN, GPIO_OUT);

    adc_init();
    adc_gpio_init(ADC_PIN); 
    adc_select_input(0);    
    while (1) {
        uint32_t cmd = multicore_fifo_pop_blocking();

        if (cmd == FLAG_ADC) {
            adc_result = adc_read();
            multicore_fifo_push_blocking(adc_result);
        } else if (cmd == FLAG_LED_ON) {
            gpio_put(LED_PIN, 1);
        } else if (cmd == FLAG_LED_OFF) {
            gpio_put(LED_PIN, 0);
        }
    }
}

int main() {
    stdio_init_all();
    while (!stdio_usb_connected()) {
        sleep_ms(100);
    }

    printf("Core 0: Starting handshake...\n");
    multicore_launch_core1(core1_entry);

    uint32_t g = multicore_fifo_pop_blocking();
    if (g == FLAG_VALUE) {
        multicore_fifo_push_blocking(FLAG_VALUE);
        printf("Core 0: Handshake successful.\n");
    }

    char cmd;
    while (1) {
        printf("\nEnter command (0: ADC, 1: LED ON, 2: LED OFF): ");
        scanf(" %c", &cmd);

        if (cmd == '0') {
            multicore_fifo_push_blocking(FLAG_ADC);
            uint16_t result = multicore_fifo_pop_blocking();
            float voltage = (result / 4095.0f) * 3.3f;
            printf("ADC voltage on A0: %.2f V\n", voltage);
        } else if (cmd == '1') {
            multicore_fifo_push_blocking(FLAG_LED_ON);
            printf("LED turned ON\n");
        } else if (cmd == '2') {
            multicore_fifo_push_blocking(FLAG_LED_OFF);
            printf("LED turned OFF\n");
        } else {
            printf("Invalid command.\n");
        }
    }
}



