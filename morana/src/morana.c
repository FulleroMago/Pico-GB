
#include <hardware/timer.h>
#include <hardware/vreg.h>
#include <hardware/clocks.h>
#include "config.h"
#include "morana.h"

void morana_overclock()
{
    vreg_set_voltage(VREG_VOLTAGE_1_15);
    sleep_ms(2);
    set_sys_clock_pll(VCO, DIV1, DIV2);
    sleep_ms(2);
}

void morana_init_all()
{
#ifdef OVERCLOCK_ENABLED
    morana_overclock();
#endif

    display_init(0);
    controls_init();
    sd_card_init();
}

void morana_deinit_all()
{
    display_deinit();
    sd_deinit();
}