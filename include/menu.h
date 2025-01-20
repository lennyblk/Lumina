#ifndef MENU_H
#define MENU_H

#define MAX_LEVELS 100

typedef struct {
    SDL_Texture* titleText;
    SDL_Texture* levelsText;
    SDL_Texture* settingsText;
    SDL_Texture* exitText;
    SDL_Texture* menuWallpaper;  
    SDL_Texture* pauseWallpaper;
} MenuTextures;

typedef struct {
    SDL_Texture* levelTexts[MAX_LEVELS];
    SDL_Texture* createLevelText; // Add new texture for create level
    SDL_Texture* backText;
    int levelCount;
} LevelsMenuTextures;

void renderMenu(SDL_Renderer* renderer, MenuTextures* menuTextures, SDL_Rect* levelsRect, SDL_Rect* settingsRect, SDL_Rect* exitRect);

void renderLevelsMenu(SDL_Renderer* renderer, LevelsMenuTextures* levelsMenuTextures, SDL_Rect* levelRects, SDL_Rect* createLevelRect, SDL_Rect* backRect);

#endif // MENU_H