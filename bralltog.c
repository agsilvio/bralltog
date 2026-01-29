#include "SDL3/SDL_main.h"
#include "game.h"

SDL_AppResult SDL_AppInit(void **appstate, int argc, char **argv) {
    return Core_SDL_AppInit(appstate, argc, argv);
}

SDL_AppResult SDL_AppIterate(void *appstate){
    return Core_SDL_AppIterate(appstate);
}

SDL_AppResult SDL_AppEvent(void *appstate, SDL_Event *event){
    return Core_SDL_AppEvent(appstate, event);
}

void SDL_AppQuit(void *appstate, SDL_AppResult result) {
    Core_SDL_AppQuit(appstate, result);
}
