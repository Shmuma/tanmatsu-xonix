#pragma once

#include <stdio.h>
#include "pax_gfx.h"

#define CELL_SIZE 4
#define BORDER 2
#define ENEMIES_MAX     10
#define MAX_LIVES       3


typedef struct {
    size_t       screen_w, screen_h;
    pax_buf_t    fb;
    size_t       field_h, field_w;
    size_t       mask_len;
    // bitmask representation of cells in the field
    uint8_t      *field;
    // bitmasked representation of hot area we're currently trying to capture
    uint8_t      *hot_field;

    // enemies
    uint8_t enemies_count;
    uint8_t enemies_x[ENEMIES_MAX];
    uint8_t enemies_y[ENEMIES_MAX];
    uint8_t enemies_d[ENEMIES_MAX];

    // player
    uint8_t player_x;
    uint8_t player_y;
    int8_t player_dx;
    int8_t player_dy;
    uint8_t player_hot;
    uint8_t player_lives;
    uint16_t player_area;
} xonix_state_t;


void xonix_init(xonix_state_t *state);
void xonix_deinit(xonix_state_t *state);
void xonix_reset_state(xonix_state_t *state, uint8_t level);
void xonix_init_enemies(xonix_state_t *state, uint8_t level);
void xonix_draw(xonix_state_t *state);