#pragma once

typedef struct
{
    uint32_t flash_target_offset;
    bool mismatch;
} flash_write_state_t;

void rom_selector();