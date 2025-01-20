#include "../include/text.h"
#include "../include/level.h"
#include "../include/menu.h"
#include "../include/config.h"
#include "../cJSON.h"
#include <stdio.h>
#include <SDL_ttf.h>


void renderMenu(SDL_Renderer* renderer, MenuTextures* menuTextures, SDL_Rect* levelsRect, SDL_Rect* settingsRect, SDL_Rect* exitRect) {
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
    SDL_RenderClear(renderer);
    GameConfig config = loadConfig("config.json");
    // Draw the wallpaper
    SDL_Rect fullscreen = {0, 0, config.width, config.height};
    SDL_RenderCopy(renderer, menuTextures->menuWallpaper, NULL, &fullscreen);

    int textWidth, textHeight;

    SDL_QueryTexture(menuTextures->titleText, NULL, NULL, &textWidth, &textHeight);
    SDL_Rect titleRect = {300, 100, textWidth, textHeight};
    SDL_RenderCopy(renderer, menuTextures->titleText, NULL, &titleRect);

    SDL_QueryTexture(menuTextures->levelsText, NULL, NULL, &textWidth, &textHeight);
    *levelsRect = (SDL_Rect){350, 200, textWidth, textHeight};
    SDL_RenderCopy(renderer, menuTextures->levelsText, NULL, levelsRect);

    SDL_QueryTexture(menuTextures->settingsText, NULL, NULL, &textWidth, &textHeight);
    *settingsRect = (SDL_Rect){350, 300, textWidth, textHeight};
    SDL_RenderCopy(renderer, menuTextures->settingsText, NULL, settingsRect);

    SDL_QueryTexture(menuTextures->exitText, NULL, NULL, &textWidth, &textHeight);
    *exitRect = (SDL_Rect){350, 400, textWidth, textHeight};
    SDL_RenderCopy(renderer, menuTextures->exitText, NULL, exitRect);

    SDL_RenderPresent(renderer);
}

void renderLevelsMenu(SDL_Renderer* renderer, LevelsMenuTextures* levelsMenuTextures, SDL_Rect* levelRects, SDL_Rect* createLevelRect, SDL_Rect* backRect) {
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
    SDL_RenderClear(renderer);

    int textWidth, textHeight;

    for (int i = 0; i < levelsMenuTextures->levelCount; i++) {
        SDL_QueryTexture(levelsMenuTextures->levelTexts[i], NULL, NULL, &textWidth, &textHeight);
        levelRects[i] = (SDL_Rect){350, 200 + i * 50, textWidth, textHeight};
        SDL_RenderCopy(renderer, levelsMenuTextures->levelTexts[i], NULL, &levelRects[i]);
    }

    SDL_QueryTexture(levelsMenuTextures->createLevelText, NULL, NULL, &textWidth, &textHeight); // ajout du texte de création de niveau
    *createLevelRect = (SDL_Rect){350, 200 + levelsMenuTextures->levelCount * 50, textWidth, textHeight};
    SDL_RenderCopy(renderer, levelsMenuTextures->createLevelText, NULL, createLevelRect);

    SDL_QueryTexture(levelsMenuTextures->backText, NULL, NULL, &textWidth, &textHeight);
    *backRect = (SDL_Rect){350, 200 + (levelsMenuTextures->levelCount + 1) * 50, textWidth, textHeight};
    SDL_RenderCopy(renderer, levelsMenuTextures->backText, NULL, backRect);

    SDL_RenderPresent(renderer);
}

void renderSettings(SDL_Renderer* renderer, TTF_Font* font, GameConfig* config, SDL_Texture* backText, SDL_Rect* backRect) {
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
    SDL_RenderClear(renderer);

    SDL_Color textColor = {255, 255, 255, 255};
    char buffer[256];
    int textWidth, textHeight;

    snprintf(buffer, sizeof(buffer), "Settings");
    renderText(renderer, font, buffer, textColor, 100, 100);

    snprintf(buffer, sizeof(buffer), "Resolution: %dx%d", config->width, config->height);
    renderText(renderer, font, buffer, textColor, 100, 150);

    snprintf(buffer, sizeof(buffer), "Jump Key: Space");
    renderText(renderer, font, buffer, textColor, 100, 200);

    snprintf(buffer, sizeof(buffer), "Move Left Key: q");
    renderText(renderer, font, buffer, textColor, 100, 250);

    snprintf(buffer, sizeof(buffer), "Move Right Key: d");
    renderText(renderer, font, buffer, textColor, 100, 300);

    SDL_QueryTexture(backText, NULL, NULL, &textWidth, &textHeight);
    *backRect = (SDL_Rect){350, 400, textWidth, textHeight};
    SDL_RenderCopy(renderer, backText, NULL, backRect);

    SDL_RenderPresent(renderer);
}