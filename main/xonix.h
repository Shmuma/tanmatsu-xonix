#pragma once

#include <stdio.h>
#include "pax_gfx.h"

#define CELL_SIZE 4
#define BORDER 2

typedef struct {
    size_t       screen_w, screen_h;
    pax_buf_t    fb;
    size_t       field_h, field_w;
    uint8_t      *field;
} xonix_state_t;


void xonix_init(xonix_state_t *state);
void xonix_deinit(xonix_state_t *state);
