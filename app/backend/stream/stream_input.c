#include "stream_input.h"

bool stream_input_handle_key_event(stream_manager_t *manager, const SDL_KeyboardEvent *event) {
#ifdef SDL_WEBOS_SCANCODE_EXIT
    SDL_Keysym keysym = event->keysym;
    switch ((int) keysym.scancode) {
        case SDL_WEBOS_SCANCODE_EXIT:
            if (event->state == SDL_RELEASED) {
                stream_manager_set_overlay_opened(manager, true);
            }
            return true;
    }
#endif
    return true;
}