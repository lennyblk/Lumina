#include <SDL.h>

typedef struct {
    SDL_Texture* background1;
    SDL_Texture* background2;
    SDL_Texture* background3;
    SDL_Texture* player;
    SDL_Texture* ground;
    SDL_Texture* rock;
    SDL_Texture* lamp;
    SDL_Texture* grass;
    SDL_Texture* levelText;
    SDL_Texture* messageText;
} GameTextures;

typedef struct {
    SDL_Texture* run[8];
    SDL_Texture* jump;
    SDL_Texture* idle;
} PlayerTextures;

GameConfig loadConfig(const char *filePath);