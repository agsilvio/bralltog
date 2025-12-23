#include "SDL3/SDL_error.h"
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
#include <stdbool.h>

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
    MIX_Mixer * mixer;
    MIX_Track * sound;
    MIX_Track * music;
    bool musicMuted;
} GameContext;


SDL_Texture * loadTexture(char * path, SDL_Renderer * renderer) {
    SDL_Texture * newTexture = IMG_LoadTexture(renderer, path);
    if (!newTexture) {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Could not load image at '%s' could not be loaded! SDL Error: %s", path, SDL_GetError());
        return NULL;
    }

    return newTexture;
}

MIX_Track * loadTrack(MIX_Mixer * mixer, char * path) {
    MIX_Audio * sound = MIX_LoadAudio(mixer, path, true);
    if (!sound) {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Could not load sound at '%s'. SDL Error: %s", path, SDL_GetError());
        return NULL;
    }

    MIX_Track * track = MIX_CreateTrack(mixer);
    MIX_SetTrackAudio(track, sound);
    MIX_DestroyAudio(sound);

    return track;
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
    //this method of initialization is preferred for a couple of reasons:
    //1. I think having everything that's not a constant in GameContext may work better 
    //on systems like Android that really control the behaviour of your app.
    //2. It allows for more concise initialization of the GameContext. 
    //This is paired with the member-wise copy at the end of this function.
    GameContext * newAppState = (GameContext *)malloc(sizeof(GameContext));
    GameContext ctx = {
        .x = 100,
        .y = 100,
	.musicMuted = true
    };

    int result = SDL_InitSubSystem(SDL_INIT_VIDEO | SDL_INIT_AUDIO | SDL_INIT_EVENTS);
    if(result < 0) {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "SDL_InitSubSystem failed with code %d. Error: %s", result, SDL_GetError());
        return SDL_APP_FAILURE;
    }

    //open window and renderer
    SDL_Window *window;
    SDL_Renderer *renderer;
    if (!SDL_CreateWindowAndRenderer("bralltog", VIEW_WIDTH, VIEW_HEIGHT, SDL_WINDOW_BORDERLESS, &window, &renderer)) {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Couldn't create window and renderer. Error: %s", SDL_GetError());
        return SDL_APP_FAILURE;
    }
    ctx.window = window;
    ctx.renderer = renderer;

    //initialize sound
    MIX_Init();
    ctx.mixer = MIX_CreateMixerDevice(SDL_AUDIO_DEVICE_DEFAULT_PLAYBACK, NULL);
    if (!ctx.mixer) {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "SDL Mixer could not be initialized! Error: %s", SDL_GetError());
        return SDL_APP_FAILURE;
    }

    //load image
    ctx.image = loadTexture("assets/image.png", renderer);
    if (!ctx.image) {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Could not load image. Error: %s", SDL_GetError());
        return SDL_APP_FAILURE;
    }

    //load sound
    ctx.sound = loadTrack(ctx.mixer, "assets/sound.wav");
    if (!ctx.sound) {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Could not load sound. Error: %s", SDL_GetError());
        return SDL_APP_FAILURE;
    }

    //load music
    ctx.music = loadTrack(ctx.mixer, "assets/music.wav");
    if (!ctx.music) {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Could not load music. Error: %s", SDL_GetError());
        return SDL_APP_FAILURE;
    }


    MIX_SetTrackGain(ctx.sound, 1.0f);

    //playMusic (muted on init)
    MIX_SetTrackGain(ctx.music, 0.0f);
    MIX_PlayTrack(ctx.music, 0);

    if (!TTF_Init()) {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Could not load font. Error: %s", SDL_GetError());
        return SDL_APP_FAILURE;
    }

    ctx.font = TTF_OpenFont("assets/font.ttf", 48);
    if (!ctx.font) {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Font could not be loaded! Error: %s\n", SDL_GetError());
        return SDL_APP_FAILURE;
    }

    *newAppState = ctx;
    *appstate = newAppState;
    return SDL_APP_CONTINUE;
}

void playSound(GameContext *appstate) {
    GameContext * ctx = (GameContext *)appstate;
    MIX_PlayTrack(ctx->sound, 0);
}

void toggleMusic(GameContext *appstate) {
    GameContext * ctx = (GameContext *)appstate;
    if (ctx->musicMuted) {
        MIX_SetTrackGain(ctx->music, 0.1f);
	ctx->musicMuted = false;
    } else {
        MIX_SetTrackGain(ctx->music, 0.0f);
	ctx->musicMuted = true;
    }
}

void handleInput(GameContext *ctx) {
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
    GameContext * ctx = (GameContext *)appstate;
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
    SDL_Surface * textSurface = TTF_RenderText_Blended(ctx->font, "Press S for sound", 0, color);
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
    GameContext * ctx = (GameContext *)appstate;
    if (!ctx) { return SDL_APP_FAILURE; }

    if (event->type == SDL_EVENT_KEY_DOWN) {
        switch(event->key.key) {
            case SDLK_Q:  return SDL_APP_SUCCESS; break;
            case SDLK_S:  playSound(ctx); break;
            case SDLK_M:  toggleMusic(ctx); break;
        }
    }

    return SDL_APP_CONTINUE;
}

void SDL_AppQuit(void *appstate, SDL_AppResult result) {
    GameContext * ctx = (GameContext *)appstate;

    if (ctx) { 
        TTF_CloseFont(ctx->font);
        MIX_DestroyTrack(ctx->sound);
        MIX_DestroyTrack(ctx->music);
        SDL_DestroyTexture(ctx->image);
        SDL_DestroyRenderer(ctx->renderer);
        SDL_DestroyWindow(ctx->window);
    }

    TTF_Quit();
    MIX_Quit();
    SDL_Quit();
}
