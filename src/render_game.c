#include "../include/render_game.h"
#include "../include/text.h"

void renderGame(GameContext *ctx) {
    Player     *p   = &ctx->player;
    GameConfig *cfg = &ctx->config;

    SDL_SetRenderDrawColor(ctx->renderer, 0, 0, 0, 255);
    SDL_RenderClear(ctx->renderer);

    SDL_Rect bgRect = {0, 0, cfg->width, cfg->height};
    SDL_RenderCopy(ctx->renderer, ctx->textures.background1, NULL, &bgRect);
    SDL_RenderCopy(ctx->renderer, ctx->textures.background2, NULL, &bgRect);
    SDL_RenderCopy(ctx->renderer, ctx->textures.background3, NULL, &bgRect);

    for (int y = 0; y < LEVEL_HEIGHT; y++) {
        for (int x = 0; x < LEVEL_WIDTH; x++) {
            SDL_Rect dest = {x * TILE_SIZE, y * TILE_SIZE, TILE_SIZE, TILE_SIZE};
            switch (ctx->level[y][x]) {
                case 7: SDL_RenderCopy(ctx->renderer, ctx->textures.ground, NULL, &dest); break;
                case 8: SDL_RenderCopy(ctx->renderer, ctx->textures.rock,   NULL, &dest); break;
                case 9: SDL_RenderCopy(ctx->renderer, ctx->textures.grass,  NULL, &dest); break;
                case 6: SDL_RenderCopy(ctx->renderer, ctx->textures.lamp,   NULL, &dest); break;
            }
        }
    }

    SDL_Texture *currentTexture = ctx->playerTextures.idle;
    if (ctx->keys[SDL_GetScancodeFromKey(cfg->moveLeftKey)] ||
        ctx->keys[SDL_GetScancodeFromKey(cfg->moveRightKey)]) {
        if (SDL_GetTicks() - p->frameStart >= (Uint32)p->frameDelay) {
            p->frame = (p->frame + 1) % 8;
            p->frameStart = SDL_GetTicks();
        }
        currentTexture = ctx->playerTextures.run[p->frame];
    } else if (ctx->keys[SDL_GetScancodeFromKey(cfg->jumpKey)] && p->canJump > 0) {
        currentTexture = ctx->playerTextures.jump;
    }

    SDL_Rect playerRect = {p->x, p->y, TILE_SIZE, TILE_SIZE};
    SDL_RendererFlip flip = p->facingRight ? SDL_FLIP_NONE : SDL_FLIP_HORIZONTAL;
    SDL_RenderCopyEx(ctx->renderer, currentTexture, NULL, &playerRect, 0, NULL, flip);

    SDL_RenderCopy(ctx->renderer, ctx->pauseButtonTexture, NULL, &ctx->pauseButtonRect);

    int textWidth, textHeight;
    SDL_QueryTexture(ctx->levelTextTexture, NULL, NULL, &textWidth, &textHeight);
    SDL_Rect textRect = {(cfg->width - textWidth) / 2, 20, textWidth, textHeight};
    SDL_RenderCopy(ctx->renderer, ctx->levelTextTexture, NULL, &textRect);

    SDL_Color gray = {200, 200, 200, 255};
    renderText(ctx->renderer, ctx->font, ctx->tryString, gray,
               cfg->width / 3 - 400, cfg->height / 42);
}
