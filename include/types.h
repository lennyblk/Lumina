#ifndef TYPES_H
#define TYPES_H

#include <SDL.h>
#include <SDL_ttf.h>
#include "config.h"
#include "game.h"
#include "menu.h"
#include "level.h"
#include "collision.h"

typedef enum {
    MENU, LEVELS, SETTINGS, PLAYING, PAUSED, CREATE_LEVEL, EXIT
} GameState;

typedef struct {
    int x, y;
    int hasCheckpoint, checkpointX, checkpointY, isCheckpointActive;
} PlayerSpawn;

typedef struct {
    int x, y;
    int velocityY;
    int canJump;
    int facingRight;
    int frame;
    int frameDelay;
    Uint32 frameStart;
} Player;

typedef struct {
    GameState state;
    int running;

    SDL_Window   *window;
    SDL_Renderer *renderer;
    TTF_Font     *font;

    GameConfig        config;
    GameTextures      textures;
    MenuTextures      menuTextures;
    LevelsMenuTextures levelsMenuTextures;
    PlayerTextures    playerTextures;

    int  level[LEVEL_HEIGHT][LEVEL_WIDTH];
    int  customLevel[LEVEL_HEIGHT][LEVEL_WIDTH];
    char levelNames[MAX_LEVELS][256];
    char levelText[256];
    SDL_Texture *levelTextTexture;

    Player      player;
    PlayerSpawn spawn;
    int         keys[SDL_NUM_SCANCODES];

    int  try;
    char tryString[18];

    char filename[256];
    int  enteringFilename;
    int  cursorX, cursorY;

    SDL_Rect    levelsRect, settingsRect, exitRect;
    SDL_Rect    levelRects[MAX_LEVELS];
    SDL_Rect    createLevelRect, backRect;
    SDL_Rect    saveRect;
    SDL_Texture *saveText;
    SDL_Rect    pauseButtonRect, playButtonRect, backToMenuButtonRect;
    SDL_Texture *pauseButtonTexture, *playButtonTexture, *backToMenuButtonTexture;
    SDL_Rect    backToMenuRect;
    SDL_Texture *backToMenuText;

    SDL_Color textColor, blackColor, highlightColor;
} GameContext;

#endif // TYPES_H
