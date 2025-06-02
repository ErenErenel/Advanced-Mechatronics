#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "pico/stdlib.h"
#include "hardware/pwm.h"
#include "cam.h"

// === LED PWM Setup ===
#define LEFT_LED 16
#define RIGHT_LED 18
#define WRAP 255
#define CLK_DIV 1.0f

void init_pwm(uint gpio) {
    gpio_set_function(gpio, GPIO_FUNC_PWM);
    uint slice = pwm_gpio_to_slice_num(gpio);
    pwm_set_wrap(slice, WRAP);
    pwm_set_clkdiv(slice, CLK_DIV);
    pwm_set_enabled(slice, true);
}

void set_motor_speeds(int com) {
    int line_pos = ((com - 40) * 100) / 40;
    if (line_pos > 100) line_pos = 100;
    if (line_pos < -100) line_pos = -100;

    int base_speed = WRAP * 0.8;
    int adjust = (WRAP * abs(line_pos)) / 100;

    int left_duty = base_speed;
    int right_duty = base_speed;

    if (line_pos > 0) {
        right_duty -= adjust;
    } else if (line_pos < 0) {
        left_duty -= adjust;
    }

    pwm_set_gpio_level(LEFT_LED, left_duty);
    pwm_set_gpio_level(RIGHT_LED, right_duty);
    printf("COM: %d | Left PWM: %d | Right PWM: %d\n", com, left_duty, right_duty);
}

int main() {
    stdio_init_all();
    while (!stdio_usb_connected()) {
        sleep_ms(100);
    }
    printf("Hello, camera!\n");

    init_camera_pins();
    init_pwm(LEFT_LED);
    init_pwm(RIGHT_LED);

    while (true) {
        // Exit check
        int c = getchar_timeout_us(0);
        if (c != PICO_ERROR_TIMEOUT) {
            char ch = (char)c;
            if (ch == 'q' || ch == 'Q') {
                printf("Quitting.\n");
                break;
            }
        }

        // Continuously capture and respond to image
        setSaveImage(1);
        while (getSaveImage() == 1) {}

        convertImage();
        int com = findLine(IMAGESIZEY / 2);
        setPixel(IMAGESIZEY / 2, com, 0, 255, 0);
        printImage();
        printf("%d\r\n", com);
        set_motor_speeds(com);

        sleep_ms(100);  // Optional: reduce frequency of updates
    }

    // Turn off motors/LEDs
    pwm_set_gpio_level(LEFT_LED, 0);
    pwm_set_gpio_level(RIGHT_LED, 0);

    return 0;
}
