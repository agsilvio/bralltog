#include "SDL3/SDL_timer.h"
#define SDL_MAIN_USE_CALLBACKS

#include "SDL3_image/SDL_image.h"
#include "SDL3/SDL_main.h"
#include "SDL3/SDL_video.h"
#include "SDL3/SDL_init.h"
#include "SDL3/SDL_keycode.h"
#include "SDL3/SDL_render.h"
#include "SDL3/SDL_log.h"
#include "SDL3_ttf/SDL_ttf.h" 
#include "SDL3_mixer/SDL_mixer.h"
#include <stdio.h>
#include <stdlib.h>

#define VIEW_WIDTH 900
#define VIEW_HEIGHT 500
#define DESIRED_FPS 60

typedef struct {
    Uint32 lastTime;
    int x, y;
    SDL_Window * window;
    SDL_Renderer * renderer;
    SDL_Texture * image;
    TTF_Font * font;
    Mix_Chunk * sound;
    Mix_Music * music;
} GameState;


SDL_Texture * loadTexture(char * path, SDL_Renderer * renderer) {
    SDL_Texture * newTexture = IMG_LoadTexture(renderer, path);
    if (!newTexture) {
      printf( "Could not load image at '%s' could not be loaded! SDL Error: %s\n", path, SDL_GetError() );
      return NULL;
    }

    return newTexture;
}

Mix_Chunk * loadSound(char * path) {
  Mix_Chunk * sound = Mix_LoadWAV(path);
  if (!sound) {
      printf( "Could not load sound at '%s'. SDL Error: %s\n", path, SDL_GetError());
      return NULL;
  }

  return sound;
}

void rateLimitFps(Uint32 lastTime) {
    Uint32 frameTime = 1000 / DESIRED_FPS;
    Uint32 delay = frameTime - (SDL_GetTicks() - lastTime);
    if (delay > 0) {
      SDL_Delay(delay);
    }
  }



SDL_AppResult SDL_AppInit(void **appstate, int argc, char **argv)
{
    printf("SDL_AppInit\n");

    GameState * ctx = malloc(sizeof(GameState));
    ctx->x = 100;
    ctx->y = 100;

    int result = SDL_InitSubSystem(SDL_INIT_VIDEO | SDL_INIT_AUDIO | SDL_INIT_EVENTS);
    if(result < 0) {
        printf("SDL_InitSubSystem failed with code %d.", result);
        return SDL_APP_FAILURE;
    }

    //open window and renderer
    SDL_Window *window;
    SDL_Renderer *renderer;
    if (!SDL_CreateWindowAndRenderer("bralltog", VIEW_WIDTH, VIEW_HEIGHT, SDL_WINDOW_BORDERLESS, &window, &renderer)) {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Couldn't create window and renderer: %s", SDL_GetError());
        return SDL_APP_FAILURE;
    }
    ctx->window = window;
    ctx->renderer = renderer;

    //open audio device
    //if (Mix_OpenAudio(0, NULL)) {
    //  printf( "SDL Mixer could not be initialized! SDL Error: %s\n", SDL_GetError() );
    //  return SDL_APP_FAILURE;
    //}

    //load image
    ctx->image = loadTexture("assets/image.png", renderer);
    if (!ctx->image) {
        printf( "Could not load image. %s", SDL_GetError() );
        return SDL_APP_FAILURE;
    }

    ////load sound
    //ctx->sound = loadSound("assets/sound.wav");

    ////load music
    //ctx->music = Mix_LoadMUS("assets/music.wav");
    //if (!ctx->music) {
    //    return SDL_APP_FAILURE;
    //}

    ////play music
    //Mix_VolumeMusic(30);
    //Mix_PlayMusic(ctx->music, -1);

    if (!TTF_Init()) {
        printf( "Could not load font. %s", SDL_GetError() );
        return SDL_APP_FAILURE;
    }

    ctx->font = TTF_OpenFont("assets/font.ttf", 48);
    if (!ctx->font) {
      printf( "Font could not be loaded! SDL Error: %s\n", SDL_GetError() );
      return SDL_APP_FAILURE;
    }

    *appstate = ctx;
    return SDL_APP_CONTINUE;
}

void handleInput(GameState *ctx) {
    const bool* keystates = SDL_GetKeyboardState(NULL);

    if (keystates[SDL_SCANCODE_UP]) {
            ctx->y -= 2;
    }
    if (keystates[SDL_SCANCODE_DOWN]) {
            ctx->y += 2;
    }
    if (keystates[SDL_SCANCODE_LEFT]) {
            ctx->x -= 2;
    }
    if (keystates[SDL_SCANCODE_RIGHT]) {
            ctx->x += 2;
    }
}

SDL_AppResult SDL_AppIterate(void *appstate){
    GameState * ctx = (GameState *)appstate;
    if (!ctx) { return SDL_APP_FAILURE; }

    ctx->lastTime = SDL_GetTicks();
    rateLimitFps(ctx->lastTime);

    handleInput(ctx);

    SDL_SetRenderDrawColor(ctx->renderer, 100, 150, 50, SDL_ALPHA_OPAQUE);
    SDL_RenderClear(ctx->renderer);

    //simple rectangle
    SDL_SetRenderDrawColor(ctx->renderer, 200, 255, 200, SDL_ALPHA_OPAQUE);
    SDL_FRect square = {ctx->x, ctx->y, 75, 75};
    SDL_RenderFillRect(ctx->renderer, &square);

    //image
    SDL_FRect imageRect = { 200, 200, 50, 150 };
    SDL_RenderTexture(ctx->renderer, ctx->image, NULL, &imageRect);

    ////text
    SDL_Color color = { 100, 255, 100 };
    SDL_Surface * textSurface = TTF_RenderText_Blended(ctx->font, "Sample Text", 0, color);
    SDL_Texture* textTexture = SDL_CreateTextureFromSurface( ctx->renderer, textSurface );
    SDL_FRect destRect = { 50, 50, textSurface->w, textSurface->h };
    SDL_RenderTexture(ctx->renderer, textTexture, NULL, &destRect);

    //present
    SDL_RenderPresent(ctx->renderer);

    //this is needed, here, to avoid memory leak
    //one could create these surfaces and textures in the Init event
    //if the text never changed. This example would work for 
    //changing text (though not done here).
    SDL_DestroySurface(textSurface); 
    SDL_DestroyTexture(textTexture);

    return SDL_APP_CONTINUE;
}

SDL_AppResult SDL_AppEvent(void *appstate, SDL_Event *event){
    GameState * ctx = (GameState *)appstate;
    if (!ctx) { return SDL_APP_FAILURE; }

    if (event->type == SDL_EVENT_KEY_DOWN) {
        switch(event->key.key) {
            case SDLK_K:  ctx->y -= 10; break;
            case SDLK_J:  
                ctx->y += 10; 
                //Mix_PlayChannel(-1, ctx->sound, 0);
                break;
            case SDLK_H:  ctx->x -= 10; break;
            case SDLK_L:  ctx->x += 10; break;
            case SDLK_Q:  return SDL_APP_SUCCESS;
        }
    }

    return SDL_APP_CONTINUE;
}

void SDL_AppQuit(void *appstate, SDL_AppResult result) {
    GameState * ctx = (GameState *)appstate;

    if (ctx) { 
        TTF_CloseFont(ctx->font);
        Mix_FreeChunk(ctx->sound);
        Mix_FreeMusic(ctx->music);
        SDL_DestroyTexture(ctx->image);
        SDL_DestroyRenderer(ctx->renderer);
        SDL_DestroyWindow(ctx->window);
    }

    TTF_Quit();
    Mix_Quit();
    SDL_Quit();
}
