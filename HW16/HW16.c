#include "pico/stdlib.h"
#include "hardware/pwm.h"
#include <stdio.h>
#include <stdlib.h> 

#define PHASE_PIN 16
#define ENABLE_PIN 17

#define PWM_WRAP 9999               // wrap = 9999 → 10,000 total steps
#define STEP_PERCENT 1             // Step size: 1% increments

int main() {
    stdio_init_all();

    // Set direction pin as output
    gpio_init(PHASE_PIN);
    gpio_set_dir(PHASE_PIN, GPIO_OUT);

    // Set PWM pin for motor speed control
    gpio_set_function(ENABLE_PIN, GPIO_FUNC_PWM);
    uint slice_num = pwm_gpio_to_slice_num(ENABLE_PIN);
    pwm_set_wrap(slice_num, PWM_WRAP);  // Set max counter value (0–9999)
    pwm_set_enabled(slice_num, true);   // Start PWM

    int duty_percent = 0; // Range: -100 to +100

    while (true) {
        int c = getchar_timeout_us(0);
        if (c != PICO_ERROR_TIMEOUT) {
            if (c == '+') {
                if (duty_percent < 100) duty_percent += STEP_PERCENT;
            } else if (c == '-') {
                if (duty_percent > -100) duty_percent -= STEP_PERCENT;
            }

            // Calculate level using (wrap + 1) to allow full 100% range
            uint16_t level = (abs(duty_percent) * (PWM_WRAP + 1)) / 100;

            // Set motor direction
            gpio_put(PHASE_PIN, duty_percent >= 0 ? 1 : 0);

            // Apply PWM level
            pwm_set_chan_level(slice_num, pwm_gpio_to_channel(ENABLE_PIN), level);

            printf("Duty cycle: %d%%\n", duty_percent);
        }

        sleep_ms(10);
    }
}
