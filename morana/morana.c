
#include "config.h"
#include "display.h"
#include "controls.h"
#include "sd_card.h"
#include "morana.h"
#include "ili9341.h"
#include "gfx.h"

void morana_init_all()
{
    display_init(0);
    controls_init();
    sd_card_init();
}

void morana_deinit_all()
{
    display_deinit();
    sd_deinit();
}