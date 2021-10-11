#include <stdio.h>
#include "lib/st7789.h"
#include "pico/stdlib.h"
#include "hardware/spi.h"

#define LED_PIN 25

int main()
{
    stdio_init_all();
    spi_init(SPI_PORT, 1251000000);

    gpio_init(DC_PIN);
    gpio_init(RES_PIN);
    gpio_init(BK_PIN);
    gpio_init(LED_PIN);
    gpio_set_dir(DC_PIN, GPIO_OUT);
    gpio_set_dir(RES_PIN, GPIO_OUT);
    gpio_set_dir(BK_PIN, GPIO_OUT);
    gpio_set_dir(LED_PIN, GPIO_OUT);

    // Set SPI format spi, bits, polarity (CPOL), phase (CPHA)
    spi_set_format( SPI_PORT, 8, 1, 0, 1);

    //SPI pins
    gpio_set_function(MOSI_PIN, GPIO_FUNC_SPI);
    gpio_set_function(SCK_PIN, GPIO_FUNC_SPI);

    display_init();
    fill(GREEN);
    fill(RED);
    int r_width = DISP_WIDTH - 40;
    int r_height = DISP_HEIGHT - 40;
    fill_rect(20, 20, r_width, r_height, convert_rgb_to_hex(0, 0, 255));
    while (true)
    {
        //Loop
        tight_loop_contents();
        fill(RED);
        sleep_ms(500);
        fill(GREEN);
        sleep_ms(500);
        fill(BLUE);
        sleep_ms(500);
        fill(BLACK);
        sleep_ms(500);
        fill(WHITE);
        sleep_ms(500);
    }
    return 0;
}