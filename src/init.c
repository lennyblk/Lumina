#include "../include/init.h"
#include "../include/text.h"
#include "../include/level.h"
#include <SDL_image.h>
#include <string.h>
#include <stdio.h>

int initContext(GameContext *ctx) {
    memset(ctx, 0, sizeof(GameContext));

    ctx->config  = loadConfig("config.json");
    ctx->running = 1;
    ctx->state   = MENU;

    char tryStr[] = {'0','0','0','0',' ',':',' ','t','e','n','t','a','t','i','v','e','s','\0'};
    memcpy(ctx->tryString, tryStr, sizeof(tryStr));

    ctx->textColor      = (SDL_Color){255, 255, 255, 255};
    ctx->blackColor     = (SDL_Color){0,   0,   0,   255};
    ctx->highlightColor = (SDL_Color){255, 255, 0,   255};

    ctx->cursorX = 0;
    ctx->cursorY = LEVEL_HEIGHT - 1;

    if (SDL_Init(SDL_INIT_VIDEO) < 0) {
        printf("SDL init failed: %s\n", SDL_GetError());
        return 0;
    }
    if (!(IMG_Init(IMG_INIT_PNG) & IMG_INIT_PNG)) {
        printf("SDL_image init failed: %s\n", SDL_GetError());
        return 0;
    }
    if (TTF_Init() < 0) {
        printf("SDL_ttf init failed: %s\n", TTF_GetError());
        return 0;
    }

    ctx->window = SDL_CreateWindow("Lumina",
        SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
        ctx->config.width, ctx->config.height, SDL_WINDOW_SHOWN);
    if (!ctx->window) { printf("Window failed: %s\n", SDL_GetError()); return 0; }

    ctx->renderer = SDL_CreateRenderer(ctx->window, -1, SDL_RENDERER_ACCELERATED);
    if (!ctx->renderer) { printf("Renderer failed: %s\n", SDL_GetError()); return 0; }

    ctx->font = TTF_OpenFont("oak_woods_v1.0/fonts/KnightWarrior-w16n8.otf", 32);
    if (!ctx->font) { printf("Font failed: %s\n", TTF_GetError()); return 0; }

    SDL_Surface *surf = TTF_RenderText_Solid(ctx->font, "Init Level", ctx->textColor);
    ctx->textures.levelText = SDL_CreateTextureFromSurface(ctx->renderer, surf);
    SDL_FreeSurface(surf);

    ctx->textures.background1 = loadTexture("oak_woods_v1.0/background/background_layer_1.png", ctx->renderer);
    ctx->textures.background2 = loadTexture("oak_woods_v1.0/background/background_layer_2.png", ctx->renderer);
    ctx->textures.background3 = loadTexture("oak_woods_v1.0/background/background_layer_3.png", ctx->renderer);
    ctx->textures.ground      = loadTexture("oak_woods_v1.0/decorations/sol1.png",    ctx->renderer);
    ctx->textures.rock        = loadTexture("oak_woods_v1.0/decorations/rock_3.png",  ctx->renderer);
    ctx->textures.lamp        = loadTexture("oak_woods_v1.0/decorations/lamp.png",    ctx->renderer);
    ctx->textures.grass       = loadTexture("oak_woods_v1.0/decorations/grass_2.png", ctx->renderer);

    if (!ctx->textures.background1 || !ctx->textures.background2 || !ctx->textures.background3 ||
        !ctx->textures.ground || !ctx->textures.rock || !ctx->textures.lamp ||
        !ctx->textures.grass  || !ctx->textures.levelText) {
        printf("Game texture load failed\n");
        return 0;
    }

    ctx->menuTextures.titleText    = createTextTexture(ctx->font, "LUMINA",   ctx->textColor, ctx->renderer);
    ctx->menuTextures.levelsText   = createTextTexture(ctx->font, "Levels",   ctx->textColor, ctx->renderer);
    ctx->menuTextures.settingsText = createTextTexture(ctx->font, "Settings", ctx->textColor, ctx->renderer);
    ctx->menuTextures.exitText     = createTextTexture(ctx->font, "Exit",     ctx->textColor, ctx->renderer);
    ctx->menuTextures.menuWallpaper  = loadTexture("oak_woods_v1.0/wallapaper2.jpeg", ctx->renderer);
    ctx->menuTextures.pauseWallpaper = loadTexture("oak_woods_v1.0/wallapaper1.jpeg", ctx->renderer);

    if (!ctx->menuTextures.titleText || !ctx->menuTextures.levelsText ||
        !ctx->menuTextures.settingsText || !ctx->menuTextures.exitText ||
        !ctx->menuTextures.menuWallpaper || !ctx->menuTextures.pauseWallpaper) {
        printf("Menu texture load failed\n");
        return 0;
    }

    ctx->levelsMenuTextures.createLevelText = createTextTexture(ctx->font, "cree ton niveau", ctx->textColor, ctx->renderer);
    ctx->levelsMenuTextures.backText        = createTextTexture(ctx->font, "Back",            ctx->textColor, ctx->renderer);

    if (!ctx->levelsMenuTextures.createLevelText || !ctx->levelsMenuTextures.backText) {
        printf("Levels menu texture load failed\n");
        return 0;
    }

    for (int i = 0; i < 8; i++) {
        char path[64];
        snprintf(path, sizeof(path), "player_assets/run/run-frame%d.png", i + 1);
        ctx->playerTextures.run[i] = loadTexture(path, ctx->renderer);
    }
    ctx->playerTextures.jump = loadTexture("player_assets/jump/jump-frame1.png",    ctx->renderer);
    ctx->playerTextures.idle = loadTexture("player_assets/idle/debout-frame1.png",  ctx->renderer);

    ctx->saveText              = createTextTexture(ctx->font, "Save",  ctx->blackColor, ctx->renderer);
    ctx->pauseButtonTexture    = createTextTexture(ctx->font, "Pause", ctx->textColor,  ctx->renderer);
    ctx->playButtonTexture     = createTextTexture(ctx->font, "Play",  ctx->textColor,  ctx->renderer);
    ctx->backToMenuButtonTexture = createTextTexture(ctx->font, "Menu", ctx->textColor, ctx->renderer);
    ctx->backToMenuText        = createTextTexture(ctx->font, "Menu",  ctx->blackColor, ctx->renderer);

    ctx->pauseButtonRect      = (SDL_Rect){ctx->config.width - 50,        10,                       40,  40};
    ctx->playButtonRect       = (SDL_Rect){ctx->config.width / 2 - 50,    ctx->config.height / 2 - 20, 100, 40};
    ctx->backToMenuButtonRect = (SDL_Rect){ctx->config.width / 2 - 50,    ctx->config.height / 2 + 30, 100, 40};
    ctx->backToMenuRect       = (SDL_Rect){ctx->config.width - 150,        10,                       140, 40};

    loadLevelsFromDirectory("levels", &ctx->levelsMenuTextures, ctx->font, ctx->renderer, ctx->levelNames);

    return 1;
}

void freeContext(GameContext *ctx) {
    SDL_DestroyTexture(ctx->menuTextures.titleText);
    SDL_DestroyTexture(ctx->menuTextures.levelsText);
    SDL_DestroyTexture(ctx->menuTextures.settingsText);
    SDL_DestroyTexture(ctx->menuTextures.exitText);
    SDL_DestroyTexture(ctx->menuTextures.menuWallpaper);
    SDL_DestroyTexture(ctx->menuTextures.pauseWallpaper);
    SDL_DestroyTexture(ctx->textures.background1);
    SDL_DestroyTexture(ctx->textures.background2);
    SDL_DestroyTexture(ctx->textures.background3);
    SDL_DestroyTexture(ctx->textures.ground);
    SDL_DestroyTexture(ctx->textures.rock);
    SDL_DestroyTexture(ctx->textures.lamp);
    SDL_DestroyTexture(ctx->textures.grass);
    SDL_DestroyTexture(ctx->textures.levelText);
    if (ctx->levelTextTexture) SDL_DestroyTexture(ctx->levelTextTexture);
    SDL_DestroyTexture(ctx->pauseButtonTexture);
    SDL_DestroyTexture(ctx->playButtonTexture);
    SDL_DestroyTexture(ctx->backToMenuButtonTexture);
    SDL_DestroyTexture(ctx->saveText);
    SDL_DestroyTexture(ctx->backToMenuText);
    for (int i = 0; i < ctx->levelsMenuTextures.levelCount; i++)
        SDL_DestroyTexture(ctx->levelsMenuTextures.levelTexts[i]);
    SDL_DestroyTexture(ctx->levelsMenuTextures.createLevelText);
    SDL_DestroyTexture(ctx->levelsMenuTextures.backText);
    for (int i = 0; i < 8; i++)
        SDL_DestroyTexture(ctx->playerTextures.run[i]);
    SDL_DestroyTexture(ctx->playerTextures.jump);
    SDL_DestroyTexture(ctx->playerTextures.idle);

    if (ctx->renderer) SDL_DestroyRenderer(ctx->renderer);
    if (ctx->window)   SDL_DestroyWindow(ctx->window);
    if (ctx->font)     TTF_CloseFont(ctx->font);
    TTF_Quit();
    IMG_Quit();
    SDL_Quit();
}
