#include "xonix.h"
#include "esp_log.h"
#include "pax_gfx.h"
#include "bsp/device.h"
#include "bsp/display.h"

static char const TAG[] = "xonix";

#define BLACK 0xFF000000
#define GREEN 0xFF00FF00


// initialize xonix game state
void xonix_init(xonix_state_t *state)
{
    esp_err_t res;
    lcd_color_rgb_pixel_format_t display_color_format = LCD_COLOR_PIXEL_FORMAT_RGB565;

    // Initialize the Board Support Package
    const bsp_configuration_t bsp_configuration = {
        .display =
        {
            //.requested_color_format = LCD_COLOR_PIXEL_FORMAT_RGB565,
            .requested_color_format = display_color_format,
            .num_fbs                = 1,
        },
    };
    ESP_ERROR_CHECK(bsp_device_initialize(&bsp_configuration));

    // Get display parameters and rotation
    size_t screen_w, screen_h;
    lcd_rgb_data_endian_t display_data_endian;

    res = bsp_display_get_parameters(&screen_w, &screen_h, &display_color_format, &display_data_endian);
    ESP_ERROR_CHECK(res);  // Check that the display parameters have been initialized
    bsp_display_rotation_t display_rotation = bsp_display_get_default_rotation();

    // Convert ESP-IDF color format into PAX buffer type
    pax_buf_type_t format = PAX_BUF_24_888RGB;
    switch (display_color_format) {
        case LCD_COLOR_PIXEL_FORMAT_RGB565:
            format = PAX_BUF_16_565RGB;
            break;
        case LCD_COLOR_PIXEL_FORMAT_RGB888:
            format = PAX_BUF_24_888RGB;
            break;
        default:
            break;
    }

    // Convert BSP display rotation format into PAX orientation type
    pax_orientation_t orientation = PAX_O_UPRIGHT;
    switch (display_rotation) {
        case BSP_DISPLAY_ROTATION_90:
            orientation = PAX_O_ROT_CCW;
            break;
        case BSP_DISPLAY_ROTATION_180:
            orientation = PAX_O_ROT_HALF;
            break;
        case BSP_DISPLAY_ROTATION_270:
            orientation = PAX_O_ROT_CW;
            break;
        case BSP_DISPLAY_ROTATION_0:
        default:
            orientation = PAX_O_UPRIGHT;
            break;
    }

    // Initialize graphics stack
    pax_buf_init(&state->fb, NULL, screen_w, screen_h, format);
    pax_buf_reversed(&state->fb, display_data_endian == LCD_RGB_DATA_ENDIAN_BIG);
    pax_buf_set_orientation(&state->fb, orientation);
    ESP_LOGI(TAG, "Display %" PRIu32 "x%" PRIu32, screen_w, screen_h);

    state->screen_w = screen_w;
    state->screen_h = screen_h;
    state->field_w = screen_w / CELL_SIZE;
    state->field_h = screen_h / CELL_SIZE;
    state->row_bytes = state->field_w >> 3;
    state->field = malloc(state->row_bytes * state->field_h);
    state->hot_field = malloc(state->row_bytes * state->field_h);

    // reset field
    // reset hot area
    // reset players and enemies position
    // draw the board
    xonix_reset_state(state, 1);
    xonix_draw(state);
}


void xonix_deinit(xonix_state_t *state)
{
    free(state->field);
    free(state->hot_field);
    state->field = NULL;
}


void xonix_reset_state(xonix_state_t *state, uint8_t level)
{
    xonix_init_enemies(state, level);

    state->player_x = state->field_w >> 1;
    state->player_y = 0;
    state->player_dx = 0;
    state->player_dy = 0;
    state->player_hot = 0;
    state->player_lives = MAX_LIVES;
    state->player_area = 0;
}


inline void clear_hot(xonix_state_t *state) {
    memset(state->hot_field, 0, state->row_bytes * state->field_h);
}


void xonix_init_enemies(xonix_state_t *state, uint8_t level)
{
    state->enemies_count = level;
    state->enemies_x[0] = state->field_w >> 1;
    state->enemies_y[0] = state->field_h >> 1;
    state->enemies_d[0] = 0;
}


static void draw_row(pax_buf_t* fb, size_t row, uint8_t* row_data, uint8_t row_len, pax_col_t color)
{
    uint16_t col_start = 0, col_end;
    uint8_t byte_ofs, bit_ofs;
    bool start_found = false;

    // find first set bit at or after col_start
    while (true) {
        byte_ofs = col_start >> 3;
        bit_ofs = col_start % 8;
        if (byte_ofs > row_len)
            break;
        if (row_data[byte_ofs] & (1 << bit_ofs)) {
            start_found = true;
            break;
        }
        col_start++;
    }

    if (!start_found) {
        ESP_LOGI(TAG, "%" PRIu32 ": not found, exit", row);
        return;
    }

    // find first not set bit after col_start
    col_end = col_start+1;
    while (true) {
        byte_ofs = col_end >> 3;
        bit_ofs = col_end % 8;
        if (byte_ofs > row_len)
            break;
        if ((row_data[byte_ofs] & (1 << bit_ofs)) == 0)
            break;
        col_end++;
    }

    // fill the rectangle from col_start to col_end-1
    ESP_LOGI(TAG, "%" PRIu32 ": Draw %" PRIu16 " - %" PRIu16,
             row, col_start, col_end);

    // todo: need to repeat the search further
}


// Very simple version redrawing everything
void xonix_draw(xonix_state_t *state)
{
    size_t row;
    pax_background(&state->fb, BLACK);

    // draw the field
    for (row = 0; row < state->field_h; row++) {
        draw_row(&state->fb, row, state->field + row*state->row_bytes, state->row_bytes, GREEN);
    }

    // draw hot area
    // draw player
    // draw enemies

    bsp_display_blit(0, 0, state->screen_w, state->screen_h, pax_buf_get_pixels(&state->fb));
}

