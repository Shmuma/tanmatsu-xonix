#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include "xonix.h"

#include "badgevms/compositor.h"


int main(int argc, char *argv[]) {
    uint32_t i = 0;
    xonix_state_t state;
    xonix_init(&state);

    while (true) {
	i += 1;
	xonix_draw(&state);
	window_present(state.window, true, NULL, 0);
	window_event_poll(state.window, false, 1);
    }

    window_destroy(state.window);
    return 0;
}
