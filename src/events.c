#include "../include/events.h"
#include "../include/level.h"
#include "../include/text.h"
#include "../include/createlevel.h"
#include <string.h>
#include <stdio.h>

static int isMouseOverButton(int mouseX, int mouseY, SDL_Rect rect) {
    return mouseX >= rect.x && mouseX <= rect.x + rect.w &&
           mouseY >= rect.y && mouseY <= rect.y + rect.h;
}

void handleEvents(GameContext *ctx) {
    SDL_Event event;
    while (SDL_PollEvent(&event)) {
        if (event.type == SDL_QUIT) {
            ctx->running = 0;

        } else if (event.type == SDL_KEYDOWN) {
            ctx->keys[event.key.keysym.scancode] = 1;

            if (ctx->state == MENU) {
                switch (event.key.keysym.sym) {
                    case SDLK_1: ctx->state = LEVELS;    break;
                    case SDLK_2: ctx->state = SETTINGS;  break;
                    case SDLK_3: ctx->running = 0;       break;
                }
            } else if (ctx->state == CREATE_LEVEL) {
                if (ctx->enteringFilename) {
                    if (event.key.keysym.sym == SDLK_RETURN) {
                        if (isValidFilename(ctx->filename)) {
                            saveLevelWithFilename(ctx->filename, ctx->customLevel);
                            loadLevelsFromDirectory("levels", &ctx->levelsMenuTextures,
                                                    ctx->font, ctx->renderer, ctx->levelNames);
                            ctx->state = LEVELS;
                        } else {
                            printf("Invalid filename. Use only alphanumeric, underscore, hyphen.\n");
                        }
                        ctx->enteringFilename = 0;
                    } else if (event.key.keysym.sym == SDLK_BACKSPACE && strlen(ctx->filename) > 0) {
                        ctx->filename[strlen(ctx->filename) - 1] = '\0';
                    } else if (event.key.keysym.sym >= SDLK_a && event.key.keysym.sym <= SDLK_z) {
                        char c = (char)event.key.keysym.sym;
                        strncat(ctx->filename, &c, 1);
                    }
                } else {
                    if      (event.key.keysym.sym == SDLK_UP    && ctx->cursorY > 0)              ctx->cursorY--;
                    else if (event.key.keysym.sym == SDLK_DOWN  && ctx->cursorY < LEVEL_HEIGHT-1) ctx->cursorY++;
                    else if (event.key.keysym.sym == SDLK_LEFT  && ctx->cursorX > 0)              ctx->cursorX--;
                    else if (event.key.keysym.sym == SDLK_RIGHT && ctx->cursorX < LEVEL_WIDTH-1)  ctx->cursorX++;
                    else if (event.key.keysym.sym == SDLK_6)
                        ctx->customLevel[ctx->cursorY][ctx->cursorX] =
                            ctx->customLevel[ctx->cursorY][ctx->cursorX] == 6 ? 0 : 6;
                    else if (event.key.keysym.sym == SDLK_7)
                        ctx->customLevel[ctx->cursorY][ctx->cursorX] =
                            ctx->customLevel[ctx->cursorY][ctx->cursorX] == 7 ? 0 : 7;
                    else if (event.key.keysym.sym == SDLK_8)
                        ctx->customLevel[ctx->cursorY][ctx->cursorX] =
                            ctx->customLevel[ctx->cursorY][ctx->cursorX] == 8 ? 0 : 8;
                    else if (event.key.keysym.sym == SDLK_9)
                        ctx->customLevel[ctx->cursorY][ctx->cursorX] =
                            ctx->customLevel[ctx->cursorY][ctx->cursorX] == 9 ? 0 : 9;
                }
            }

        } else if (event.type == SDL_KEYUP) {
            ctx->keys[event.key.keysym.scancode] = 0;

        } else if (event.type == SDL_MOUSEBUTTONDOWN) {
            int mouseX = event.button.x;
            int mouseY = event.button.y;

            if (ctx->state == MENU) {
                if      (isMouseOverButton(mouseX, mouseY, ctx->levelsRect))   ctx->state = LEVELS;
                else if (isMouseOverButton(mouseX, mouseY, ctx->settingsRect)) ctx->state = SETTINGS;
                else if (isMouseOverButton(mouseX, mouseY, ctx->exitRect))     ctx->running = 0;

            } else if (ctx->state == LEVELS) {
                for (int i = 0; i < ctx->levelsMenuTextures.levelCount; i++) {
                    if (isMouseOverButton(mouseX, mouseY, ctx->levelRects[i])) {
                        loadLevel(ctx->levelNames[i], ctx->level);
                        ctx->state = PLAYING;
                        snprintf(ctx->levelText, sizeof(ctx->levelText), "%s", ctx->levelNames[i]);
                        if (ctx->levelTextTexture) SDL_DestroyTexture(ctx->levelTextTexture);
                        ctx->levelTextTexture = createTextTexture(ctx->font, ctx->levelText,
                                                                  ctx->textColor, ctx->renderer);
                        ctx->spawn.hasCheckpoint = 0;
                        ctx->spawn.isCheckpointActive = 0;
                        break;
                    }
                }
                if      (isMouseOverButton(mouseX, mouseY, ctx->createLevelRect)) ctx->state = CREATE_LEVEL;
                else if (isMouseOverButton(mouseX, mouseY, ctx->backRect)) {
                    ctx->try = 0;
                    updateTryString(ctx->try, ctx->tryString);
                    ctx->state = MENU;
                }

            } else if (ctx->state == SETTINGS) {
                if (isMouseOverButton(mouseX, mouseY, ctx->backRect)) ctx->state = MENU;

            } else if (ctx->state == PLAYING) {
                if (isMouseOverButton(mouseX, mouseY, ctx->pauseButtonRect)) ctx->state = PAUSED;

            } else if (ctx->state == PAUSED) {
                if (isMouseOverButton(mouseX, mouseY, ctx->playButtonRect)) {
                    ctx->state = PLAYING;
                } else if (isMouseOverButton(mouseX, mouseY, ctx->backToMenuButtonRect)) {
                    ctx->spawn.hasCheckpoint = 0;
                    ctx->spawn.isCheckpointActive = 0;
                    ctx->player.x = ctx->spawn.x;
                    ctx->player.y = ctx->spawn.y;
                    ctx->player.velocityY = 0;
                    ctx->player.canJump = 2;
                    ctx->try = 0;
                    updateTryString(ctx->try, ctx->tryString);
                    ctx->state = MENU;
                }

            } else if (ctx->state == CREATE_LEVEL) {
                if      (isMouseOverButton(mouseX, mouseY, ctx->saveRect))       ctx->enteringFilename = 1;
                else if (isMouseOverButton(mouseX, mouseY, ctx->backToMenuRect)) {
                    ctx->try = 0;
                    updateTryString(ctx->try, ctx->tryString);
                    ctx->state = MENU;
                }
            }
        }
    }
}
