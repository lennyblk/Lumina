#ifndef LEVEL_H
#define LEVEL_H

#include <SDL.h>
#include <SDL_ttf.h>
#include "menu.h"

#define LEVEL_WIDTH 40
#define LEVEL_HEIGHT 25
#define MAX_LEVELS 100

void loadLevel(const char *filePath, int level[LEVEL_HEIGHT][LEVEL_WIDTH]);
void loadLevelsFromDirectory(const char* directory, LevelsMenuTextures* levelsMenuTextures, TTF_Font* font, SDL_Renderer* renderer, char levelNames[MAX_LEVELS][256]);

#endif // LEVEL_H
