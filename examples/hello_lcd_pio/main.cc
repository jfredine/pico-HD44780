#include <stdio.h>
#include "pico/stdlib.h"
#include "pico/binary_info.h"
#include "hardware/pio.h"
#include "HD44780.h"

int main(int argc, char *argv[]) {
 
    bi_decl(bi_program_description("Drive HD44780 based LCD"));
    stdio_init_all();

    HD44780 hd44780(7, 2, pio0);

    hd44780.init();

    hd44780.position(2, 4);
    hd44780.write_string("Hello world!");
    while(1) {
    }
}
