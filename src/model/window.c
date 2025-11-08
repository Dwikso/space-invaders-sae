#include <SDL3/SDL_keyboard.h>
#include <SDL3/SDL_scancode.h>
#define SDL_MAIN_USE_CALLBACKS 1
#include <SDL3/SDL.h>
#include <SDL3/SDL_main.h>
#include <stdio.h>

static SDL_Window *window = NULL;
static SDL_Renderer *renderer = NULL;
static SDL_Texture *texture = NULL;
static SDL_Texture *texture_player = NULL;

static int texture_width = 0;
static int texture_height = 0;
static int texture_player_width = 0;
static int texture_player_height = 0;

static float player_y = 0;

#define WINDOW_WIDTH 640
#define WINDOW_HEIGHT 480
#define PLAYER_SPEED 0.2f

SDL_AppResult SDL_AppInit(void **appstate, int argc, char *argv[]) {
  (void)appstate;
  (void)argc;
  (void)argv;

  SDL_Surface *surface = NULL;
  SDL_Surface *surface_player = NULL;
  char *bmp_path = "src/assets/Sprites/Invaders/space__0000_A1.bmp";
  char *bmp_player = "src/assets/spaceinvaders_64_icon.bmp";

  SDL_SetAppMetadata("Example Renderer Clear", "1.0",
                     "com.example.renderer-clear");

  if (!SDL_Init(SDL_INIT_VIDEO)) {
    SDL_Log("Couldn't initialize SDL: %s", SDL_GetError());
    return SDL_APP_FAILURE;
  }

  if (!SDL_CreateWindowAndRenderer("examples/renderer/clear", WINDOW_WIDTH,
                                   WINDOW_HEIGHT, SDL_WINDOW_RESIZABLE, &window,
                                   &renderer)) {
    SDL_Log("Couldn't create window/renderer: %s", SDL_GetError());
    return SDL_APP_FAILURE;
  }

  SDL_SetRenderLogicalPresentation(renderer, WINDOW_WIDTH, WINDOW_HEIGHT,
                                   SDL_LOGICAL_PRESENTATION_LETTERBOX);

  surface = SDL_LoadBMP(bmp_path);
  if (!surface) {
    SDL_Log("Couldn't load bitmap '%s': %s", bmp_path, SDL_GetError());
    return SDL_APP_FAILURE;
  }

  texture_width = surface->w;
  texture_height = surface->h;

  texture = SDL_CreateTextureFromSurface(renderer, surface);
  SDL_DestroySurface(surface);

  if (!texture) {
    SDL_Log("Couldn't create texture: %s", SDL_GetError());
    return SDL_APP_FAILURE;
  }

  surface_player = SDL_LoadBMP(bmp_player);
  if (!surface_player) {
    SDL_Log("Couldn't load bitmap '%s': %s", bmp_player, SDL_GetError());
    return SDL_APP_FAILURE;
  }

  texture_player_width = surface_player->w;
  texture_player_height = surface_player->h;

  texture_player = SDL_CreateTextureFromSurface(renderer, surface_player);
  SDL_DestroySurface(surface_player);

  if (!texture_player) {
    SDL_Log("Couldn't create texture: %s", SDL_GetError());
    return SDL_APP_FAILURE;
  }

  // Initialiser la position du joueur au centre
  player_y = (WINDOW_WIDTH - texture_player_width) / 2.0f;

  return SDL_APP_CONTINUE;
}

SDL_AppResult SDL_AppEvent(void *appstate, SDL_Event *event) {
  (void)appstate;

  if (event->type == SDL_EVENT_QUIT) {
    return SDL_APP_SUCCESS;
  }
  return SDL_APP_CONTINUE;
}

SDL_AppResult SDL_AppIterate(void *appstate) {
  (void)appstate;

  const bool *key_states = SDL_GetKeyboardState(NULL);
  if (key_states[SDL_SCANCODE_LEFT]) {
    player_y -= PLAYER_SPEED;
  } else if (key_states[SDL_SCANCODE_RIGHT]) {
    player_y += PLAYER_SPEED;
  }

  // Limiter le joueur dans l'écran
  if (player_y < 0) {
    player_y = 0;
  }
  if (player_y > WINDOW_WIDTH - texture_player_width) {
    player_y = WINDOW_WIDTH - texture_player_width;
  }

  // Effacer l'écran
  SDL_SetRenderDrawColor(renderer, 0, 0, 0, SDL_ALPHA_OPAQUE);
  SDL_RenderClear(renderer);

  // Dessiner les invaders
  for (int i = 0; i < 10; i++) {
    SDL_FRect dst_rect;
    dst_rect.x = 50 + (i * 60);
    dst_rect.y = 200;
    dst_rect.w = texture_width;
    dst_rect.h = texture_height;
    SDL_RenderTexture(renderer, texture, NULL, &dst_rect);
  }

  // Dessiner le joueur
  SDL_FRect dst_rect_player;
  dst_rect_player.x = player_y;
  dst_rect_player.y = WINDOW_HEIGHT - texture_player_height - 20;
  dst_rect_player.w = texture_player_width;
  dst_rect_player.h = texture_player_height;
  SDL_RenderTexture(renderer, texture_player, NULL, &dst_rect_player);

  // Afficher le rendu
  SDL_RenderPresent(renderer);

  return SDL_APP_CONTINUE;
}

void SDL_AppQuit(void *appstate, SDL_AppResult result) {
  (void)appstate;
  (void)result;

  SDL_DestroyTexture(texture);
  SDL_DestroyTexture(texture_player);
}
