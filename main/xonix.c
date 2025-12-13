#include <string.h>
#include <stdlib.h>
#include <stdio.h>

#include "xonix.h"
#include "badgevms/compositor.h"


void xonix_init(xonix_state_t *state) {
    window_size_t win_size = { SCREEN_WIDTH, SCREEN_HEIGHT };
    state->window = window_create(
	"Xonix", win_size, WINDOW_FLAG_FULLSCREEN | WINDOW_FLAG_DOUBLE_BUFFERED
    );

    state->win_fb = window_framebuffer_create(
	state->window, win_size, BADGEVMS_PIXELFORMAT_RGB565
    );

    xonix_reset_game_state(state);
    xonix_draw(state);
}

void xonix_reset_game_state(xonix_state_t *state) {
    state->enemies_count = 1;
    state->player_x = FIELD_WIDTH >> 1;
    state->player_y = 0;
    state->player_dx = 0;
    state->player_dy = 0;
    state->player_hot = 0;
    state->player_lives = MAX_LIVES;
    state->player_area = 0;

    xonix_reset_field(state);
    xonix_init_enemies(state);
}


inline void clear_hot(xonix_state_t *state) {
    memset(state->hot_field, 0, sizeof(state->hot_field));
}


void xonix_reset_field(xonix_state_t *state) {
    uint8_t r, c, v;

    for (r = 0; r < FIELD_HEIGHT; r++) {
        for (c = 0; c < FIELD_BYTES; c++) {
            v = 0xFF;
            if (r < 2 || r >= FIELD_HEIGHT-2)
                v = 0;
            else if (c == 0)
                v = 0b00111111;
            else if (c == FIELD_BYTES-1)
                v = 0b11111100;
            state->field[r][c] = v;
        }
    }

    clear_hot(state);
}


void xonix_init_enemies(xonix_state_t *state) {
    uint8_t i;
    for (i = 0; i < state->enemies_count; i++) {
        state->enemies_x[i] = (uint8_t)(rand() % GRASS_WIDTH) + BORDER;
        state->enemies_y[i] = (uint8_t)(rand() % GRASS_HEIGHT) + BORDER;
        state->enemies_d[i] = (uint8_t)(rand() % 4);
    }
}


inline void xonix_draw_cell(xonix_state_t *state, uint8_t r, uint8_t c, uint16_t color565) {
    //    tft_fill_area(c*CELL_SIZE, (r + FIELD_OFFSET)*CELL_SIZE, CELL_SIZE-1, CELL_SIZE-1, color);
}


void xonix_draw_row(xonix_state_t *state, uint8_t r, uint8_t c, uint8_t w, uint16_t color565) {
//    printf("row: r=%d, c=%d, w=%d, col=%x\n", r, c, w, color565);
    uint16_t y0 = (r + FIELD_OFFSET)*CELL_SIZE;
    uint16_t x0 = c * CELL_SIZE;
//    printf("x0 = %d, y0 = %d\n", x0, y0);
    for (uint16_t y = y0; y < y0 + CELL_SIZE; y++) {
	for (uint16_t x = x0; x < x0 + w*CELL_SIZE; x++) {
	    state->win_fb->pixels[x + y*SCREEN_WIDTH] = color565;
	}
    }
//    tft_fill_area(c*CELL_SIZE, (r + FIELD_OFFSET)*CELL_SIZE, w*CELL_SIZE-1, CELL_SIZE-1, color);
}


void xonix_draw(xonix_state_t *state) {
    for (int r = 0; r < FIELD_HEIGHT; r++) {
	xonix_draw_row(state, r, 0, FIELD_WIDTH, COLOR_GRASS);
    }
//    xonix_draw_row(state, 100, 0, FIELD_WIDTH, COLOR_GRASS);
//    xonix_draw_field(state);
//    xonix_draw_header(state);
//    xonix_draw_enemies(state);
//    xonix_draw_header(state);
//    xonix_draw_player(state);
}


void xonix_draw_field(xonix_state_t *state) {
    uint8_t cur_bit = 0, start = 0, end = 0;
    uint8_t mask, val, same, r, c;

//    xonix_draw_row(state, -1, 0, FIELD_WIDTH, 0x000000);

    for (r = 0; r < FIELD_HEIGHT; r++) {
        cur_bit = start = end = 0;
        for (c = 0; c < FIELD_BYTES; c++) {
            mask = 1 << 7;
            val = state->field[r][c];

            while (mask) {
                if (cur_bit)
                    same = val & mask;
                else
                    same = ~val & mask;
                if (same)
                    end += 1;
                else {
                    xonix_draw_row(state, r, start, end-start, cur_bit ? COLOR_GRASS : COLOR_ROAD);
                    cur_bit = 1-cur_bit;
                    start = end;
                    end = start+1;

                }

                mask >>= 1;
            }
        }

        if (end > start) {
            xonix_draw_row(state, r, start, end-start, cur_bit ? COLOR_GRASS : COLOR_ROAD);
        }
    }
}


