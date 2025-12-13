#include "xonix.h"
#include "esp_log.h"
#include "pax_gfx.h"
#include "bsp/device.h"
#include "bsp/display.h"

static char const TAG[] = "xonix";


// initialize xonix game
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
    state->field = malloc((state->field_w * state->field_h) >> 8);
}


void xonix_deinit(xonix_state_t *state)
{
    free(state->field);
    state->field = NULL;
}
