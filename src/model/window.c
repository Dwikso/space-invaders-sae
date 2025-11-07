#include <SDL3/SDL_render.h>
#define SDL_MAIN_USE_CALLBACKS 1
#include <SDL3/SDL.h>
#include <SDL3/SDL_main.h>

static SDL_Window *window = NULL;
static SDL_Renderer *renderer = NULL;

SDL_AppResult SDL_AppInit(void **appstate, int argc, char *argv[]) {
  SDL_SetAppMetadata("Example Renderer Clear", "1.0",
                     "com.example.renderer-clear");

  if (!SDL_Init(SDL_INIT_VIDEO)) {
    SDL_Log("Couldn't initialize SDL: %s", SDL_GetError());
    return SDL_APP_FAILURE;
  }

  if (!SDL_CreateWindowAndRenderer("examples/renderer/clear", 640, 480,
                                   SDL_WINDOW_RESIZABLE, &window, &renderer)) {
    SDL_Log("Couldn't create window/renderer: %s", SDL_GetError());
    return SDL_APP_FAILURE;
  }
  SDL_SetRenderLogicalPresentation(renderer, 640, 480,
                                   SDL_LOGICAL_PRESENTATION_LETTERBOX);

  return SDL_APP_CONTINUE;
}

SDL_AppResult SDL_AppEvent(void *appstate, SDL_Event *event) {
  if (event->type == SDL_EVENT_QUIT) {
    return SDL_APP_SUCCESS;
  }
  return SDL_APP_CONTINUE;
}

SDL_AppResult SDL_AppIterate(void *appstate) {
  int i;

  const SDL_FPoint points[] = {{100, 354}, {220, 230}, {140, 230},
                               {320, 100}, {500, 230}, {420, 230},
                               {540, 354}, {400, 354}, {100, 354}};

  SDL_SetRenderDrawColor(renderer, 127, 49, 32, SDL_ALPHA_OPAQUE);
  SDL_RenderPoint(renderer, 100, 354);
  SDL_RenderPoint(renderer, 220, 230);
  SDL_RenderPoint(renderer, 140, 230);
  SDL_RenderPoint(renderer, 320, 100);
  SDL_RenderPoint(renderer, 500, 230);
  SDL_RenderPoint(renderer, 420, 230);
  SDL_RenderPoint(renderer, 540, 354);
  SDL_RenderPoint(renderer, 400, 354);
  SDL_RenderPoint(renderer, 100, 354);

  SDL_SetRenderDrawColor(renderer, 255, 255, 255, SDL_ALPHA_OPAQUE);
  SDL_RenderPoints(renderer, points, SDL_arraysize(points));

  for (i = 0; i < 360; i++) {
    const int size = 100;
    const int x = 320;
    const int y = 95 - (size / 2);
    SDL_SetRenderDrawColor(renderer, SDL_rand(256), SDL_rand(256),
                           SDL_rand(256), SDL_ALPHA_OPAQUE);
    SDL_RenderPoint(renderer, x, y);
  }

  SDL_RenderPresent(renderer);

  return SDL_APP_CONTINUE;
}

void SDL_AppQuit(void *appstate, SDL_AppResult result) {}
