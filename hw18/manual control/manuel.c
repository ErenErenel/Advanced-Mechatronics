#include "pico/stdlib.h"
#include "hardware/pwm.h"
#include <stdio.h>
#include <stdlib.h>

// === Your Motor Pin Setup ===
#define A_PHASE 16     // Motor A (Left) Direction
#define A_ENABLE 17    // Motor A (Left) Speed
#define B_PHASE 18     // Motor B (Right) Direction
#define B_ENABLE 19    // Motor B (Right) Speed

#define WRAP 255
#define CLK_DIV 1.0f

int duty_a = 0; // Range: -100 to +100
int duty_b = 0; // Range: -100 to +100

// Add a reverse flag for correcting direction per motor
void set_motor(int duty_percent, uint enbl, uint phase, bool invert_dir) {
    if (duty_percent > 100) duty_percent = 100;
    if (duty_percent < -100) duty_percent = -100;

    bool forward = duty_percent >= 0;
    gpio_put(phase, invert_dir ? !forward : forward ? 0 : 1); // Adjust per motor
    gpio_set_function(enbl, GPIO_FUNC_PWM);
    uint slice = pwm_gpio_to_slice_num(enbl);
    pwm_set_wrap(slice, WRAP);
    pwm_set_clkdiv(slice, CLK_DIV);
    pwm_set_chan_level(slice, pwm_gpio_to_channel(enbl), abs(duty_percent * WRAP / 100));
    pwm_set_enabled(slice, true);
}

int main() {
    stdio_init_all();

    gpio_init(A_PHASE); gpio_set_dir(A_PHASE, GPIO_OUT);
    gpio_init(B_PHASE); gpio_set_dir(B_PHASE, GPIO_OUT);

    while (!stdio_usb_connected()) {
        tight_loop_contents();
    }

    printf("Manual dual motor control ready.\n");

    while (true) {
        int c = getchar_timeout_us(0);
        if (c != PICO_ERROR_TIMEOUT) {
            if (c == 'w') {
                duty_a = 100;   // Left motor forward
                duty_b = 100;   // Right motor forward
            } else if (c == 's') {
                duty_a = 0;
                duty_b = 0;
            } else if (c == 'a') {
                duty_a = -100;  // Left motor backward
                duty_b = 100;   // Right motor forward → Turn left
            } else if (c == 'd') {
                duty_a = 100;   // Left motor forward
                duty_b = -100;  // Right motor backward → Turn right
            }else if (c == 'x') {
                duty_a = -100;
                duty_b = -100;
            } else if (c == 'q') {
                printf("Square path movement starting...\n");
                for (int i = 0; i < 4; i++) {
                    set_motor(80, A_ENABLE, A_PHASE, false);
                    set_motor(80, B_ENABLE, B_PHASE, true);
                    sleep_ms(1000);

                    set_motor(80, A_ENABLE, A_PHASE, false);
                    set_motor(-80, B_ENABLE, B_PHASE, true);
                    sleep_ms(1000);
                }
                duty_a = 0;
                duty_b = 0;
            }

            set_motor(duty_a, A_ENABLE, A_PHASE, false); // Left motor: normal
            set_motor(duty_b, B_ENABLE, B_PHASE, true);  // Right motor: inverted
        }

        sleep_ms(20);
    }

    return 0;
}
