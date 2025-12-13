#include "xonix.h"
#include "esp_log.h"

static char const TAG[] = "xonix";


// initialize xonix game
void xonix_init(xonix_state_t *state, size_t x_size, size_t y_size)
{
    ESP_LOGI(TAG, "Display %" PRIu32 "x%" PRIu32, x_size, y_size);

    state->x_size = x_size;
    state->y_size = y_size;
}