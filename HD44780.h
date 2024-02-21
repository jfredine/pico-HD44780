#include "pico/stdlib.h"
#include "hardware/pio.h"

class HD44780 {
 public:
#ifdef HD44780_MODE_CPU
    HD44780(uint en, uint rs, uint d7, uint d6, uint d5, uint d4);
#endif
#ifdef HD44780_MODE_PIO
    HD44780(uint en, uint d4, PIO pio=pio0);
#endif

    void clear(void);
    void home(void);
    void init(void);
    void position(uint row, uint col);
    void write_string(const char *str);

 private:
#ifdef HD44780_MODE_CPU
    void pulse_enable(void);
#endif
#ifdef HD44780_MODE_PIO
    static uint add_delay_cmd(uint cmds, uint delay);
    static uint add_write_cmd(uint cmds, uint rs, uint value);
#endif
    void write_byte(uint rs, uint value, uint delay=0);
    void write_nibble(uint rs, uint value, uint delay=0);

    uint en_;
    uint d4_;

#ifdef HD44780_MODE_CPU
    uint rs_;
    uint d7_;
    uint d6_;
    uint d5_;
    uint data_mask_;
    uint rs_data_mask_;
    uint all_mask_;
#endif

#ifdef HD44780_MODE_PIO
    PIO  pio_;
    uint sm_;
#endif

    static uint const row_starts_[];
};
