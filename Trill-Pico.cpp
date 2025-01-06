#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/i2c.h"
#include "Trill.h"

#define I2C_PORT i2c0
#define I2C_SDA 0
#define I2C_SCL 1

Trill trill;

int main() {
    // Initialize serial output and I2C
    stdio_init_all();
    i2c_init(I2C_PORT, 400 * 1000);  // 400 kHz
    gpio_set_function(I2C_SDA, GPIO_FUNC_I2C);  // SDA
    gpio_set_function(I2C_SCL, GPIO_FUNC_I2C);  // SCL
    gpio_pull_up(I2C_SDA);
    gpio_pull_up(I2C_SCL);

    // Initialize Trill device
    if (trill.setup(I2C_PORT, I2C_SDA, I2C_SCL, Trill::SQUARE, 0x28) != 0) {
        printf("Failed to initialize Trill device.\n");
        return -1;
    }
    printf("Trill device initialized successfully.\n");

    while (true) {
        // Read touch data from Trill
        if (trill.readI2C() == 0) {
            unsigned int numTouches = trill.getNumTouches();
            printf("Number of touches: %u\n", numTouches);

            // Print touch details
            for (unsigned int i = 0; i < numTouches; i++) {
                float location = trill.touchLocation(i);
                float size = trill.touchSize(i);
                printf("Touch %u: Location = %.2f, Size = %.2f\n", i, location, size);
            }
        } else {
            printf("Failed to read touch data.\n");
        }

        sleep_ms(100);  // Delay between reads
    }
    return 0;
}
