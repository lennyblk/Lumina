#include <SDL.h>

#define LEVEL_WIDTH 80   
#define LEVEL_HEIGHT 50  
#define MAX_LEVELS 100

void loadLevel(const char *filePath, int level[LEVEL_HEIGHT][LEVEL_WIDTH]);
void loadLevelsFromDirectory(const char* directory, LevelsMenuTextures* levelsMenuTextures, TTF_Font* font, SDL_Renderer* renderer, char levelNames[MAX_LEVELS][256]);
