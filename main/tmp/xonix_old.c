#include "hw.h"
#include "xonix.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdint.h>

#define DEFAULT_DELAY 100

// bitmask representation of cells in the field
uint8_t field[FIELD_HEIGHT][FIELD_BYTES];
// bitmasked representation of hot area we're currently trying to capture
uint8_t hot_field[FIELD_HEIGHT][FIELD_BYTES];

// enemies
uint8_t enemies_count = 1;
uint8_t enemies_x[ENEMIES_MAX];
uint8_t enemies_y[ENEMIES_MAX];
uint8_t enemies_d[ENEMIES_MAX];

// player
uint8_t player_x;
uint8_t player_y;
int8_t player_dx = 0;
int8_t player_dy = 0;
uint8_t player_hot = 0;
uint8_t player_lives = MAX_LIVES;
uint16_t player_area = 0;


void xonix_main() {
    uint32_t wait_until = millis() + DEFAULT_DELAY;
    uint8_t keys_got;
    int8_t keys[3];
    
    start_after_wake = &xonix_wake;
    xonix_init();
    
    while (1) {
        if (millis() >= wait_until) {
            xonix_step();
            wait_until = millis() + DEFAULT_DELAY;
        }
        
        // read input
		keys_got = stdio_get(keys);
		if (keys_got) {
			if (keys[0] == K_UP) {
                player_dx = 0;
                player_dy = -1;
            }
			if (keys[0] == K_LT) {
                player_dx = -1;
                player_dy = 0;
            }
			if (keys[0] == K_DN) {
                player_dx = 0;
                player_dy = 1;
            }
			if (keys[0] == K_RT) {
                player_dx = 1;
                player_dy = 0;
            }
		}
    }
}

void xonix_init() {    
    xonix_init_field();
    xonix_draw_field();
    
    xonix_init_enemies();
    xonix_draw_enemies();
    player_x = FIELD_WIDTH/2;
    player_y = 0;
    player_dx = player_dy = 0;
    player_hot = 0;
    player_area = 0;
    xonix_draw_header();
    xonix_draw_player();
}


void xonix_wake() {
    xonix_draw_field();
    xonix_draw_enemies();
    xonix_draw_player();
    xonix_draw_header();
}

void xonix_init_field() {
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
            field[r][c] = v;
        }
    }
    
    clear_hot();
}


void xonix_init_enemies() {
    uint8_t i;
    for (i = 0; i < enemies_count; i++) {
        enemies_x[i] = (uint8_t)(rand() % GRASS_WIDTH)+2;
        enemies_y[i] = (uint8_t)(rand() % GRASS_HEIGHT)+2;
        enemies_d[i] = (uint8_t)(rand() % 4);
    }
}


void xonix_draw_field() {
    uint8_t cur_bit = 0, start = 0, end = 0;
    uint8_t mask, val, same, r, c;

    xonix_draw_row(-1, 0, FIELD_WIDTH, 0x000000);
    
    for (r = 0; r < FIELD_HEIGHT; r++) {
        cur_bit = start = end = 0;
        for (c = 0; c < FIELD_BYTES; c++) {
            mask = 1 << 7;
            val = field[r][c];
            
            while (mask) {
                if (cur_bit)
                    same = val & mask;
                else
                    same = ~val & mask;
                if (same)
                    end += 1;
                else {
                    xonix_draw_row(r, start, end-start, cur_bit ? COLOR_GRASS : COLOR_ROAD);
                    cur_bit = 1-cur_bit;
                    start = end;
                    end = start+1;
                            
                }
                        
                mask >>= 1;
            }
        }

        if (end > start) {
            xonix_draw_row(r, start, end-start, cur_bit ? COLOR_GRASS : COLOR_ROAD);
        }
    }
}

// checks that cell of the field contains grass
inline uint8_t is_grass(uint8_t x, uint8_t y) {
    return field[y][x / 8] & (1 << (7 - x % 8));
    
    // x=0 -> 0, 7
    // x=1 -> 0, 6
    // x=6 -> 0, 1
    // x=7 -> 0, 0
    // x=8 -> 1, 7
}

// the same as is_grass, but for hot_field
inline uint8_t is_hot(uint8_t x, uint8_t y) {
    return hot_field[y][x / 8] & (1 << (7 - x % 8));
}

inline void set_hot(uint8_t x, uint8_t y) {
    ht_field[y][x / 8] |= 1 << (7 - x % 8);
}

inline void clear_hot() {
   memset(hot_field, 0, sizeof(hot_field)); 
}

inline void xonix_draw_cell(uint8_t r, uint8_t c, uint32_t color) {
    tft_fill_area(c*CELL_SIZE, (r + FIELD_OFFSET)*CELL_SIZE, CELL_SIZE-1, CELL_SIZE-1, color);
}

inline void xonix_draw_row(uint8_t r, uint8_t c, uint8_t w, uint32_t color) {
    tft_fill_area(c*CELL_SIZE, (r + FIELD_OFFSET)*CELL_SIZE, w*CELL_SIZE-1, CELL_SIZE-1, color);
}


void xonix_draw_enemies() {
    uint8_t i;
    
    for (i = 0; i < enemies_count; i++) {
        xonix_draw_cell(enemies_y[i], enemies_x[i], COLOR_ENEMY);
    }
}

void xonix_undraw_enemies() {
    uint8_t i;
    
    for (i = 0; i < enemies_count; i++) {
        xonix_draw_cell(enemies_y[i], enemies_x[i], COLOR_GRASS);
    }
}

void xonix_draw_header() {
    const uint8_t prog = (uint8_t)((uint32_t)PROGRESS_LEN * (uint32_t)player_area / (uint32_t)AREA_TO_WIN);
    uint8_t i;
    
    if (prog)
        xonix_draw_row(-1, 0, prog, COLOR_GRASS);
    xonix_draw_row(-1, prog, PROGRESS_LEN-prog, COLOR_ENEMY);
    xonix_draw_row(-1, PROGRESS_LEN, FIELD_WIDTH-PROGRESS_LEN, COLOR_HEADER);
    
    for (i = 1; i <= player_lives; i++) {
        xonix_draw_cell(-1, PROGRESS_LEN + 2 + (MAX_LIVES-i)*2, COLOR_PLAYER);
    }
}

void xonix_step_enemies() {
    uint8_t i, x, y;
    int8_t dy, dx;
    
    for (i = 0; i < enemies_count; i++) {
        dx = (enemies_d[i] & 0b01) ? 1 : -1;
        dy = (enemies_d[i] & 0b10) ? 1 : -1;
        
        x = enemies_x[i];
        y = enemies_y[i];

        if (!is_grass(x+dx, y+dy)) {
            if (is_grass(x+dx, y-dy))
                enemies_d[i] ^= 0b10;           // invert y dir
            else if (is_grass(x-dx, y+dy))
                enemies_d[i] ^= 0b01;           // invert x dir
            else
                enemies_d[i] ^= 0b11;           // invert both
            continue;
        }
        
        if (is_hot(x+dx, y+dy)) {
            xonix_player_lost();
            return;
        }
        
        xonix_draw_cell(y+dy, x+dx, COLOR_ENEMY);
        xonix_draw_cell(y, x, COLOR_GRASS);
        enemies_x[i] = x + dx;
        enemies_y[i] = y + dy;
    }
}

//void xonix_add_enemy() {
//    if (enemies_count+1 == ENEMIES_MAX)
//        return;
//    xonix_undraw_enemies();
//    enemies_count++;
//    xonix_init_enemies();
//    xonix_draw_enemies();
//}

void xonix_step() {
    xonix_step_enemies();
    xonix_step_player();
}

inline void xonix_draw_player() {
    xonix_draw_cell(player_y, player_x, COLOR_PLAYER);
}


void xonix_step_player() {
    uint8_t x, y;
    
    if (player_dx == 0 && player_dy == 0)
        return;
    
    // if we've reached bounds
    if ((player_dx < 0 && player_x == 0) || \
        (player_dx > 0 && player_x == FIELD_WIDTH-1))
    {
        player_dx = 0;
        return;
    }

    if ((player_dy < 0 && player_y == 0) || \
        (player_dy > 0 && player_y == FIELD_HEIGHT-1))
    {
        player_dy = 0;
        return;
    }
    
    x = player_x + player_dx;
    y = player_y + player_dy;
    
    xonix_draw_cell(y, x, COLOR_PLAYER);

    if (is_hot(x, y)) {
        xonix_player_lost();
        return;
    }

    if (is_grass(x, y)) {
        set_hot(x, y);
        player_hot = 1;
    }
    else {
        // we finished the drawing of hot area
        if (player_hot) {
            if (xonix_commit_path())
                return;
            player_hot = player_dx = player_dy = 0;
        }
    }
  
    if (is_grass(player_x, player_y))
        xonix_draw_cell(player_y, player_x, COLOR_HOT_GRASS);
    else
        xonix_draw_cell(player_y, player_x, COLOR_ROAD);
    
    player_x = x;
    player_y = y;
}


// mark our path in hot area as road and draw it
uint8_t xonix_commit_path() {
    uint8_t r, c, mask, ofs, v, i;
    uint8_t x, y;
    uint16_t cells_got = 0;
    
    for (r = 0; r < FIELD_HEIGHT; r++) {
        for (c = 0; c < FIELD_BYTES; c++) {
            v = hot_field[r][c];
            if (!v)
                continue;
            field[r][c] &= ~v;
            
            mask = 1 << 7;
            ofs = 0;
            while (mask) {
                if (v & mask) {
                    xonix_draw_cell(r, c*8 + ofs, COLOR_ROAD);
                    cells_got++;
                }
                ofs++;
                mask >>= 1;
            }
        }
    }
    
    clear_hot();
    
    for (i = 0; i < enemies_count; i++) {
        if (!is_hot(enemies_x[i], enemies_y[i])) {
            xonix_fill_hot(enemies_x[i], enemies_y[i]);
        }
    }
    
    // compare hot and grass fields to find spaces we might have occupied
    cells_got += xonix_reclaim_hot();
    clear_hot();
    
    // calculate percentage
    player_area += cells_got;
    if (AREA_TO_WIN <= player_area) {
        xonix_next_level();
        return 1;
    }
    xonix_draw_header();
    return 0;
}


// method taken from http://www.adammil.net/blog/v126_A_More_Efficient_Flood_Fill.html
void xonix_fill_hot(uint8_t x, uint8_t y) {
    uint8_t ox, oy;
    while (1) {
        ox = x;
        oy = y;
        while (is_grass(x, y-1)) y--;
        while (is_grass(x-1, y)) x--;
        if (ox == x && oy == y)
            break;
    }
    
    xonix_fill_hot_core(x, y);
}

void xonix_fill_hot_core(uint8_t x, uint8_t y) {
    uint8_t last_row_len = 0, row_len, sx;
    uint8_t ux;
    
    do {
        row_len = 0;
        sx = x;
        
        if (last_row_len != 0 && (!is_grass(x, y) || is_hot(x, y))) {
            do {
                if (--last_row_len == 0)
                    return;                
            } while (!is_grass(++x, y) || is_hot(x, y));
            sx = x;
        }
        else {
            for (; is_grass(x-1, y) && !is_hot(x-1, y); row_len++, last_row_len++) {
                set_hot(--x, y);
                if (is_grass(x, y-1) && !is_hot(x, y-1))
                    xonix_fill_hot(x, y-1);
            }
        }
        
        for(; is_grass(sx, y) && !is_hot(sx, y); row_len++, sx++) {
            set_hot(sx, y);
        }
        
        if (row_len < last_row_len) {
            for (ux = x + last_row_len; ++sx < ux;) {
                if (is_grass(sx, y) && !is_hot(sx, y))
                    xonix_fill_hot_core(sx, y);
            }
        }
        else if (row_len > last_row_len) {
            for (ux = x + last_row_len; ++ux < sx;) {
                if (is_grass(ux, y-1) && !is_hot(ux, y-1))
                    xonix_fill_hot(ux, y-1);
            }
        }
        last_row_len = row_len;
        ++y;
    } while (last_row_len != 0);
}


uint16_t xonix_reclaim_hot() {
    uint16_t res = 0;
    uint8_t r, c, mask, v, ofs;
    uint8_t s_col;
    
    for (r = 0; r < FIELD_HEIGHT; r++) {
        s_col = 0;
        for (c = 0; c < FIELD_BYTES; c++) {
            v = field[r][c] ^ hot_field[r][c];
            if (!v) {
                if (s_col)
                    xonix_draw_row(r, s_col, c*8 - s_col + 1, COLOR_ROAD);
                s_col = 0;
                continue;
            }
            
            field[r][c] = hot_field[r][c];
            mask = 1 << 7;
            ofs = 0;
            while (mask) {
                if (v & mask) {
                    res++;
                    if (!s_col)
                        s_col = c*8 + ofs;
                }
                else if (s_col) {
                    xonix_draw_row(r, s_col, c*8 + ofs - s_col + 1, COLOR_ROAD);
                    s_col = 0;
                }
                ofs++;
                mask >>= 1;
            }
        }
    }
    return res;
}


void xonix_player_lost() {
    wait_ms(1000);
    player_lives--;
    if (!player_lives) {
        xonix_game_over();
    }
    else {
        player_x = FIELD_WIDTH/2;
        player_y = 0;
        player_dx = player_dy = 0;
        player_hot = 0;

        clear_hot();
        xonix_draw_field();
        xonix_draw_enemies();
        xonix_draw_header();
        xonix_draw_player();
    }
}


void xonix_next_level() {
    wait_ms(1000);
    if (enemies_count < ENEMIES_MAX)
        enemies_count++;
    xonix_init();
}


void xonix_game_over() {
    wait_ms(1000);
    // show game over message
    // wait for key press
    player_lives = MAX_LIVES;
    enemies_count = 1;
    xonix_init();
}

