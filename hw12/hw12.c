#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "pico/stdlib.h"
#include "cam.h"

int main() {
    stdio_init_all();
    while (!stdio_usb_connected()) {
        sleep_ms(100);
    }
    printf("Camera ready.\n");

    init_camera_pins();

    while (true) {
        int c = getchar_timeout_us(0);
        if (c != PICO_ERROR_TIMEOUT) {
            char ch = (char)c;

            if (ch == 'q' || ch == 'Q') {
                printf("Quitting.\n");
                break;
            } else if (ch == 'c' || ch == 'C') {
                // Trigger image capture
                setSaveImage(1);
                while (getSaveImage() == 1) {}  // wait until image is saved

                // Process image
                convertImage();
                int com = findLine(IMAGESIZEY / 2);  // Line at middle row
                setPixel(IMAGESIZEY / 2, com, 0, 255, 0);  // Mark COM in green
                printImage();
                printf("COM: %d\r\n", com);
            }
        }
    }

    return 0;
}
