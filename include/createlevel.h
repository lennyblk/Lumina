#ifndef CREATELEVEL_H
#define CREATELEVEL_H

#include <SDL.h>
#include <SDL_ttf.h>
#include "config.h"
#include "level.h"

void renderCreateLevel(SDL_Renderer* renderer, int level[LEVEL_HEIGHT][LEVEL_WIDTH], SDL_Texture* saveText, SDL_Rect* saveRect, int cursorX, int cursorY, SDL_Texture* backToMenuText, SDL_Rect* backToMenuRect, TTF_Font* font, GameConfig* config);
void saveLevelWithFilename(const char* filePath, int level[LEVEL_HEIGHT][LEVEL_WIDTH]);
int isValidFilename(const char* filename);

#endif // CREATELEVEL_H
