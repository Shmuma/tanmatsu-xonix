#pragma once

#include <stdint.h>
#include "badgevms/compositor.h"

#define SCREEN_WIDTH	720
#define SCREEN_HEIGHT	720

#define CELL_SIZE	4             // how large one item of the field
#define FIELD_OFFSET    1
#define FIELD_WIDTH     (SCREEN_WIDTH/CELL_SIZE)
#define FIELD_HEIGHT    ((SCREEN_HEIGHT/CELL_SIZE) - FIELD_OFFSET)
#define FIELD_BYTES     (FIELD_WIDTH/8)

#define BORDER		2
#define GRASS_WIDTH     (FIELD_WIDTH-BORDER*2)
#define GRASS_HEIGHT    (FIELD_HEIGHT-BORDER*2)

#define PACK_COLOR(r, g, b) (rgb888_to_rgb565((r), (g), (b)))

#define COLOR_HEADER    PACK_COLOR(   0,   0,     0)
#define COLOR_GRASS     PACK_COLOR(   0, 0xFF,    0)    // green
#define COLOR_HOT_GRASS PACK_COLOR(0xA0, 0x40, 0xF0)    // purple
#define COLOR_ROAD      PACK_COLOR(   0,    0, 0x50)    // blue
#define COLOR_PLAYER    PACK_COLOR(0xFF, 0xFF,    0)    // yellow
#define COLOR_ENEMY     PACK_COLOR(0xFF,    0,    0)    // red

#define ENEMIES_MAX     10
#define MAX_LIVES       3
#define PROGRESS_LEN    70
#define AREA_TO_WIN     (FIELD_WIDTH*FIELD_HEIGHT*2/3)

typedef struct {
    window_handle_t window;
    framebuffer_t *win_fb;

    // bitmask representation of cells in the field
    uint8_t field[FIELD_HEIGHT][FIELD_BYTES];
    // bitmasked representation of hot area we're currently trying to capture
    uint8_t hot_field[FIELD_HEIGHT][FIELD_BYTES];

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
void xonix_reset_game_state(xonix_state_t *state);
void xonix_reset_field(xonix_state_t *state);
void xonix_init_enemies(xonix_state_t *state);

void xonix_draw(xonix_state_t *state);

void xonix_draw_field(xonix_state_t *state);
//void xonix_draw_enemies(xonix_state_t *state);
//void xonix_undraw_enemies(xonix_state_t *state);

//inline void xonix_draw_player(xonix_state_t *state);
//void xonix_draw_header(xonix_state_t *state);

#if 0

void xonix_main(void);
void xonix_step(void);

void xonix_wake(void);
void xonix_step_enemies(void);
void xonix_add_enemy(void);
void xonix_next_level();

inline uint8_t is_grass(uint8_t x, uint8_t y);
inline void set_hot(uint8_t x, uint8_t y);
inline void clear_hot(void);

void xonix_step_player();
void xonix_player_lost();

// commit functions
uint8_t xonix_commit_path();
void xonix_fill_hot(uint8_t x, uint8_t y);
void xonix_fill_hot_core(uint8_t x, uint8_t y);
uint16_t xonix_reclaim_hot();
void xonix_game_over();

#endif
