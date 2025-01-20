#include <SDL.h>

#define TILE_SIZE 16

void renderCreateLevel(SDL_Renderer* renderer, int level[LEVEL_HEIGHT][LEVEL_WIDTH], SDL_Texture* saveText, SDL_Rect* saveRect, int cursorX, int cursorY, SDL_Texture* backToMenuText, SDL_Rect* backToMenuRect, TTF_Font* font, GameConfig* config);

void saveLevelWithFilename(const char* filePath, int level[LEVEL_HEIGHT][LEVEL_WIDTH]);

int isValidFilename(const char* filename);
