#include "pico/stdlib.h"
#ifdef HD44780_MODE_PIO
#include "hardware/pio.h"
#include "HD44780.pio.h"
#endif
#include "HD44780.h"

uint const HD44780::row_starts_[] = {0, 0x40, 0x14, 0x54};

#ifdef HD44780_MODE_CPU
//
// HD44780::HD44780
// Arguments; en -- GPIO number for the enable pin
//            rs -- GPIO number for the rs pin
//            d7 -- GPIO number for the data bit 7 pin
//            d6 -- GPIO number for the data bit 6 pin
//            d5 -- GPIO number for the data bit 5 pin
//            d4 -- GPIO number for the data bit 4 pin
// Returns: Nothing
//
// Constructor for the HD44780 class which sets up data.  The init() member
// must be called before use unless another instance of the class with the 
// same pin connections has been created and initialized.
//

HD44780::HD44780(uint en, uint rs, uint d7, uint d6, uint d5, uint d4) {
#ifdef HD44780_MODE_PIO
    pio_ = nullptr;
#endif
    en_ = en;
    rs_ = rs;
    d7_ = d7;
    d6_ = d6;
    d5_ = d5;
    d4_ = d4;

    data_mask_ = 0;
    data_mask_ |= 1 << d7_;
    data_mask_ |= 1 << d6_;
    data_mask_ |= 1 << d5_;
    data_mask_ |= 1 << d4_;

    rs_data_mask_ = data_mask_;
    rs_data_mask_ |= 1 << rs_;

    all_mask_ = rs_data_mask_;
    all_mask_ |= 1 << en_;
}


// HD44780::pulse_enable
// Arguments: None
// Returns: Nothing
//
// Pulse the enable pin allowing time for setup and hold of control/data
// as well as time for instructions to execute after the pulse

void HD44780::pulse_enable(void) {
    gpio_put(en_, 0);
    sleep_us(1);
    gpio_put(en_, 1);
    sleep_us(1);
    gpio_put(en_, 0);
    sleep_us(1);
}
#endif


#ifdef HD44780_MODE_PIO
//
// HD44780::HD44780
// Arguments; en -- GPIO number for the enable pin
//            d4 -- GPIO number for the data bit 4 pin
// Returns: Nothing
//
// Constructor for the HD44780 class which sets up data.  The GPIO pins used
// for RS, D[7:4] must be consecutive with RS at the high pin and D4 at the
// low pin.  This is why only d4 is specified in the constructor as the GPIO
// used for RS and [7:5] are derived from the d4 value.
// The init() member must be called before use unless another instance of the
// class with the same pin connections has been created and initialized.
//

HD44780::HD44780(uint en, uint d4, PIO pio) {
    pio_ = pio;
    en_ = en;
    d4_ = d4;
}


// HD44780::add_delay_cmd
// Arguments: delay -- Delay value in us
// Returns: New command buffer after setting the delay
//
// Update the given command buffer to include a command for the specifed delay.
// see HD44780.pio for command format
//

uint HD44780::add_delay_cmd(uint cmds, uint delay) {
    uint new_cmds = cmds;

    new_cmds = (new_cmds << 19) | (delay & 0x7ffff);
    new_cmds = (new_cmds << 1) | 0;

    return new_cmds;
}


// HD44780::add_write_cmd
// Arguments: rs - State of RS pin for the write
//            value -- Value to write to the controller
// Returns: New command buffer after adding the write command
// 
// Update the given command buffer to include a command to write the
// specified data to the LCD controller
// see HD44780.pio for command format
//

uint HD44780::add_write_cmd(uint cmds, uint rs, uint value) {
    uint new_cmds = cmds;

    new_cmds = (new_cmds << 1) | (rs & 1);
    new_cmds = (new_cmds << 4) | (value & 0xf);
    new_cmds = (new_cmds << 1) | 1;

    return new_cmds;
}
#endif


// HD44780::write_nibble
// Arguments: rs - State of RS pin for the write
//            value -- Value to write to the controller
//            delay -- Additional delay (in us) after the write
// Returns: Nothing
//
// Write a nibble of data to the HD44780 controller with the RS pin
// in the provided state and then delay by the specified amount.
//

void HD44780::write_nibble(uint rs, uint value, uint delay) {
#ifdef HD44780_MODE_PIO
    if (pio_) {
        uint cmds = 0;
        cmds = add_delay_cmd(cmds, delay);
        cmds = add_write_cmd(cmds, rs, value);

        pio_sm_put_blocking(pio_, sm_, cmds);
#ifdef HD44780_MODE_CPU
    } else {
#else
    }
#endif
#endif
#ifdef HD44780_MODE_CPU
        uint v = 0;
        v |= ((rs & 1) << rs_);
        v |= ((value >> 3) & 1) << d7_;
        v |= ((value >> 2) & 1) << d6_;
        v |= ((value >> 1) & 1) << d5_;
        v |= (value & 1) << d4_;

        gpio_put_masked(rs_data_mask_, v);
        pulse_enable();
        sleep_us(delay);
#ifdef HD44780_MODE_PIO
    }
#endif
#endif
}


// HD44780::write_byte
// Arguments: rs - State of RS pin for the write
//            value -- Value to write to the controller
//            delay -- Additional delay (in us) after the write
// Returns: Nothing
//
// Write a byte of data to the HD44780 controller with the RX pin
// in the provided state.  The byte is written as two nibbles on d[7:4].
//

void HD44780::write_byte(uint rs, uint value, uint delay) {
#ifdef HD44780_MODE_PIO
    if (pio_) {
        uint cmds = 0;
        cmds = add_delay_cmd(cmds, delay);
        cmds = add_write_cmd(cmds, rs, value);
        cmds = add_write_cmd(cmds, rs, value >> 4);

        pio_sm_put_blocking(pio_, sm_, cmds);
#ifdef HD44780_MODE_CPU
    } else {
#else
    }
#endif
#endif
#ifdef HD44780_MODE_CPU
        uint v = 0;
        v |= ((rs & 1) << rs_);
        v |= ((value >> 7) & 1) << d7_;
        v |= ((value >> 6) & 1) << d6_;
        v |= ((value >> 5) & 1) << d5_;
        v |= ((value >> 4) & 1) << d4_;

        gpio_put_masked(rs_data_mask_, v);
        pulse_enable();

        v = 0;
        v |= ((rs & 1) << rs_);
        v |= ((value >> 3) & 1) << d7_;
        v |= ((value >> 2) & 1) << d6_;
        v |= ((value >> 1) & 1) << d5_;
        v |= (value & 1) << d4_;

        gpio_put_masked(rs_data_mask_, v);
        pulse_enable();
        sleep_us(delay);
#ifdef HD44780_MODE_PIO
    }
#endif
#endif
}


// HD44780::write_string
// Arguments: str -- string to write to the screen
// Returns: Nothing
//
// Write a string to the screen.  The string must be null terminated.
// Done as a simple sequency of bytes written to the controller with the
// RS pin held high
//

void HD44780::write_string(char const *str) {
    for (char const *cp = str; *cp != '\0'; cp++) {
        write_byte(1, *cp, 40);
    }
}


// HD44780::init
// Arguments: None
// Returns: Nothing
//
// Initialize the LCD screen.  The sequence is taken from the HD44780
// data sheet.  It puts the controller into 4-bit mode and initialzes
// the display
//

void HD44780::init(void) {
#ifdef HD44780_MODE_PIO
    if (pio_) {
        // Find a location in the PIO memory for our programm and save the address
        uint offset = pio_add_program(pio_, &HD44780_program);

        // Find a free state machine on our chosen PIO
        sm_ = pio_claim_unused_sm(pio_, true);

        // Initialize the PIO program
        HD44780_program_init(pio_, sm_, offset, en_, d4_);
#ifdef HD44780_MODE_CPU
    } else {
#else
    }
#endif
#endif

#ifdef HD44780_MODE_CPU
        gpio_init_mask(all_mask_);
        gpio_set_dir_out_masked(all_mask_);
        gpio_put_masked(all_mask_, 0);
#ifdef HD44780_MODE_PIO
    }
#endif
#endif

    // sequence from HD44780 datasheet for entering 4-bit mode
    write_nibble(0, 0x3, 4200);  // 8-bit mode with 4200us delay
    write_nibble(0, 0x3, 100);   // 8-bit mode with 100us delay
    write_nibble(0, 0x3, 40);    // 8-bit mode with 40us delay
    write_nibble(0, 0x2, 40);    // 4-bit mode with 4us delay

    // initialization proceeds using 4-bit mode
    write_byte(0, 0x28, 40);   // 4-bit mode with second nibble now active
    write_byte(0, 0xc, 40);    // display and cursor on
    write_byte(0, 0x6, 40);    // cursor moves right
    clear();
}


// HD44780::clear
// Arguments: None
// Returns: Nothing
//
// Clear the script.  An additional delay is needed after the command because
// clearing the screen is a long operation (see HD44780 data sheet).
//

void HD44780::clear(void) {
    write_byte(0, 0x1, 2000);
}


// HD44780::position
// Arguments: row -- Row to reposition the cursor to
//            col -- column to reposition the cursor to
// Returns: Nothing
//

void HD44780::position(uint row, uint col) {
    write_byte(0, 0x80 | ((row_starts_[row] + col) & 0x7f), 40);
}


// HD44780::home
// Arguments: None
// Returns: Nothing
//
// Position the cursor at the home position.  Requires extra delay because the
// command takes significant time to complete (see HD44780 data sheet)
//

void HD44780::home(void) {
    write_byte(0, 0x02, 1600);
}
