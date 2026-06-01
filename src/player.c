#include "../include/player.h"
#include "../include/collision.h"
#include "../include/text.h"
#include <SDL.h>

void initPlayer(GameContext *ctx) {
    ctx->spawn.x = 0;
    ctx->spawn.y = ctx->config.height - TILE_SIZE;
    ctx->spawn.hasCheckpoint = 0;
    ctx->spawn.checkpointX = 0;
    ctx->spawn.checkpointY = 0;
    ctx->spawn.isCheckpointActive = 0;

    ctx->player.x = ctx->spawn.x;
    ctx->player.y = ctx->spawn.y;
    ctx->player.velocityY = 0;
    ctx->player.canJump = 1;
    ctx->player.facingRight = 0;
    ctx->player.frame = 0;
    ctx->player.frameDelay = 100;
    ctx->player.frameStart = SDL_GetTicks();
}

void updatePlayer(GameContext *ctx) {
    Player      *p     = &ctx->player;
    PlayerSpawn *spawn = &ctx->spawn;
    GameConfig  *cfg   = &ctx->config;

    if (ctx->keys[SDL_GetScancodeFromKey(cfg->moveLeftKey)]) {
        p->x -= TILE_SIZE / 4;
        p->facingRight = 0;
    }
    if (ctx->keys[SDL_GetScancodeFromKey(cfg->moveRightKey)]) {
        p->x += TILE_SIZE / 4;
        p->facingRight = 1;
    }
    if (ctx->keys[SDL_GetScancodeFromKey(cfg->jumpKey)] && p->canJump > 0) {
        if (!checkAboveCollision(p->x, p->y, ctx->level, 7)) {
            p->velocityY = -TILE_SIZE / 2.2;
            p->canJump--;
        }
    }

    if (p->velocityY < 10) p->velocityY++;
    p->y += p->velocityY;

    while (checkCollision(p->x, p->y, ctx->level, 7) &&
           checkCollision(p->x, p->y + TILE_SIZE - 1, ctx->level, 7))
        p->x++;

    while (checkCollision(p->x + TILE_SIZE - 1, p->y, ctx->level, 7) &&
           checkCollision(p->x + TILE_SIZE - 1, p->y + TILE_SIZE - 1, ctx->level, 7))
        p->x--;

    while ((checkCollision(p->x, p->y, ctx->level, 7) &&
            !checkCollision(p->x, p->y + TILE_SIZE - 1, ctx->level, 7)) ||
           (checkCollision(p->x + TILE_SIZE - 1, p->y, ctx->level, 7) &&
            !checkCollision(p->x + TILE_SIZE - 1, p->y + TILE_SIZE - 1, ctx->level, 7))) {
        p->y++;
        p->velocityY = 0;
    }

    while (checkCollision(p->x, p->y + TILE_SIZE - 1, ctx->level, 7)) {
        p->y--;
        p->velocityY = 0;
        p->canJump = 2;
    }

    while (checkCollision(p->x + TILE_SIZE - 1, p->y + TILE_SIZE - 1, ctx->level, 7)) {
        p->y--;
        p->velocityY = 0;
        p->canJump = 2;
    }

    if (checkCollision(p->x, p->y, ctx->level, 8) ||
        checkCollision(p->x + TILE_SIZE - 1, p->y, ctx->level, 8) ||
        checkCollision(p->x, p->y + TILE_SIZE - 1, ctx->level, 8) ||
        checkCollision(p->x + TILE_SIZE - 1, p->y + TILE_SIZE - 1, ctx->level, 8)) {
        SDL_Color red = {255, 0, 0, 255};
        renderText(ctx->renderer, ctx->font, "GAME OVER", red, cfg->width / 2 - 80, cfg->height / 2);
        ctx->try++;
        ctx->tryString[3] = (char)(ctx->try % 10) + 48;
        ctx->tryString[2] = (char)((ctx->try / 10) % 10) + 48;
        ctx->tryString[1] = (char)((ctx->try / 100) % 10) + 48;
        ctx->tryString[0] = (char)((ctx->try / 1000) % 10) + 48;
        SDL_RenderPresent(ctx->renderer);
        SDL_Delay(2000);
        if (spawn->hasCheckpoint && spawn->isCheckpointActive) {
            p->x = spawn->checkpointX;
            p->y = spawn->checkpointY;
        } else {
            p->x = spawn->x;
            p->y = spawn->y;
        }
        p->velocityY = 0;
    }

    if (checkCollision(p->x, p->y, ctx->level, 9)) {
        SDL_Color green = {0, 255, 0, 255};
        renderText(ctx->renderer, ctx->font, "YOU WIN!", green, cfg->width / 2 - 80, cfg->height / 2);
        SDL_RenderPresent(ctx->renderer);
        SDL_Delay(2000);
        p->x = spawn->x;
        p->y = spawn->y;
        ctx->try = 0;
        updateTryString(ctx->try, ctx->tryString);
        ctx->state = MENU;
    }

    if (checkCollision(p->x, p->y, ctx->level, 6)) {
        spawn->checkpointX = p->x;
        spawn->checkpointY = p->y;
        spawn->hasCheckpoint = 1;
        if (!spawn->isCheckpointActive) {
            spawn->isCheckpointActive = 1;
            SDL_Color cyan = {0, 255, 255, 255};
            renderText(ctx->renderer, ctx->font, "Checkpoint!", cyan, cfg->width / 2, cfg->height / 2);
            SDL_RenderPresent(ctx->renderer);
            SDL_Delay(500);
        }
    }

    if (p->x < 0) p->x = 0;
    else if (p->x > cfg->width - TILE_SIZE) p->x = cfg->width - TILE_SIZE;

    if (p->y < 0) { p->y = 0; p->velocityY = 0; }
    else if (p->y >= cfg->height - TILE_SIZE) {
        p->y = cfg->height - TILE_SIZE;
        p->velocityY = 0;
        p->canJump = 2;
    }
}
