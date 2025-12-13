#pragma once

#include <stdio.h>

#define CELL_SIZE 4
#define BORDER 2

typedef struct {
    size_t x_size, y_size;
} xonix_state_t;


void xonix_init(xonix_state_t *state, size_t x_size, size_t y_size);