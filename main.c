#include <stdio.h>
#include "include/types.h"
#include "include/init.h"
#include "include/events.h"
#include "include/player.h"
#include "include/render_game.h"
#include "include/menu.h"
#include "include/createlevel.h"
#include "include/text.h"

int main(int argc, char *argv[]) {
    (void)argc; (void)argv;

    static GameContext ctx;

    if (!initContext(&ctx)) {
        freeContext(&ctx);
        return 1;
    }

    initPlayer(&ctx);

    while (ctx.running) {
        handleEvents(&ctx);

        if (ctx.state == MENU) {
            ctx.try = 0;
            updateTryString(ctx.try, ctx.tryString);
            renderMenu(ctx.renderer, &ctx.menuTextures,
                       &ctx.levelsRect, &ctx.settingsRect, &ctx.exitRect);

        } else if (ctx.state == LEVELS) {
            ctx.try = 0;
            updateTryString(ctx.try, ctx.tryString);
            renderLevelsMenu(ctx.renderer, &ctx.levelsMenuTextures,
                             ctx.levelRects, &ctx.createLevelRect, &ctx.backRect);

        } else if (ctx.state == SETTINGS) {
            renderSettings(ctx.renderer, ctx.font, &ctx.config,
                           ctx.levelsMenuTextures.backText, &ctx.backRect);

        } else if (ctx.state == PLAYING) {
            updatePlayer(&ctx);
            renderGame(&ctx);
            SDL_Delay(16);

        } else if (ctx.state == PAUSED) {
            SDL_RenderClear(ctx.renderer);
            SDL_Rect fullscreen = {0, 0, ctx.config.width, ctx.config.height};
            SDL_RenderCopy(ctx.renderer, ctx.menuTextures.pauseWallpaper, NULL, &fullscreen);
            SDL_RenderCopy(ctx.renderer, ctx.playButtonTexture,          NULL, &ctx.playButtonRect);
            SDL_RenderCopy(ctx.renderer, ctx.backToMenuButtonTexture,    NULL, &ctx.backToMenuButtonRect);

        } else if (ctx.state == CREATE_LEVEL) {
            renderCreateLevel(ctx.renderer, ctx.customLevel, ctx.saveText, &ctx.saveRect,
                              ctx.cursorX, ctx.cursorY, ctx.backToMenuText,
                              &ctx.backToMenuRect, ctx.font, &ctx.config);
            if (ctx.enteringFilename) {
                int tw, th;
                SDL_QueryTexture(ctx.saveText, NULL, NULL, &tw, &th);
                int cx = (ctx.config.width  - tw) / 2;
                int cy = (ctx.config.height - th) / 2;
                renderTextWithBackground(ctx.renderer, ctx.font, "Enter filename: ",
                                         ctx.blackColor, ctx.highlightColor, cx - 100, cy);
                renderTextWithBackground(ctx.renderer, ctx.font, ctx.filename,
                                         ctx.blackColor, ctx.highlightColor, cx + 100, cy);
            }
        }

        SDL_RenderPresent(ctx.renderer);
    }

    freeContext(&ctx);
    return 0;
}
