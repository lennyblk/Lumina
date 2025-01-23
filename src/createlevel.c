#include "../include/text.h"
#include "../include/level.h"
#include "../include/menu.h"
#include "../include/config.h"
#include "../include/createlevel.h"
#include "../cJSON.h"
#include <stdio.h>
#include <SDL_ttf.h>

void renderCreateLevel(SDL_Renderer* renderer, int level[LEVEL_HEIGHT][LEVEL_WIDTH], SDL_Texture* saveText, SDL_Rect* saveRect, int cursorX, int cursorY, SDL_Texture* backToMenuText, SDL_Rect* backToMenuRect, TTF_Font* font, GameConfig* config) {
    SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255); // Fond blanc
    SDL_RenderClear(renderer);

    for (int y = 0; y < LEVEL_HEIGHT; y++) {
        for (int x = 0; x < LEVEL_WIDTH; x++) {
            SDL_Rect destRect = {x * TILE_SIZE, y * TILE_SIZE, TILE_SIZE, TILE_SIZE};
            if (x == cursorX && y == cursorY) {
                SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255); // Noir pour le curseur
            } else if (level[y][x] == 7) {
                SDL_SetRenderDrawColor(renderer, 139, 69, 19, 255); // Marron pour les blocs solides
            } else if (level[y][x] == 8) {
                SDL_SetRenderDrawColor(renderer, 255, 0, 0, 255); // Rouge pour les pics
            } else if (level[y][x] == 9) {
                SDL_SetRenderDrawColor(renderer, 0, 255, 0, 255); // Vert pour la ligne d'arrivée
            } else if (level[y][x] == 6) {
                SDL_SetRenderDrawColor(renderer, 0, 0, 255, 255); // Bleu pour le point de contrôle
            } else {
                SDL_SetRenderDrawColor(renderer, 200, 200, 200, 255); // Gris clair pour les espaces vides
            }
            SDL_RenderFillRect(renderer, &destRect);
            SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255); // Bordure noire
            SDL_RenderDrawRect(renderer, &destRect);
        }
    }

    SDL_QueryTexture(saveText, NULL, NULL, &saveRect->w, &saveRect->h);
    saveRect->x = (config->width - saveRect->w) / 2; // Centrer horizontalement
    saveRect->y = 10; // En haut de la fenêtre
    SDL_Color highlightColor = {255, 255, 0, 255}; // Surbrillance jaune
    SDL_Color blackColor = {0, 0, 0, 255};
    renderTextWithBackground(renderer, font, "Save", blackColor, highlightColor, saveRect->x, saveRect->y);
    SDL_RenderCopy(renderer, saveText, NULL, saveRect);

    SDL_QueryTexture(backToMenuText, NULL, NULL, &backToMenuRect->w, &backToMenuRect->h);
    renderTextWithBackground(renderer, font, "menu", blackColor, highlightColor, backToMenuRect->x, backToMenuRect->y);
    SDL_RenderCopy(renderer, backToMenuText, NULL, backToMenuRect);

    renderTextWithBackground(renderer, font, "- 6 checkpoint", blackColor, highlightColor, 10, 10);
    renderTextWithBackground(renderer, font, "- 7 blocs", blackColor, highlightColor, 10, 40);
    renderTextWithBackground(renderer, font, "- 8 pics", blackColor, highlightColor, 10, 70);
    renderTextWithBackground(renderer, font, "- 9 fin", blackColor, highlightColor, 10, 100);

    SDL_RenderPresent(renderer);
}

void saveLevelWithFilename(const char *filePath, int level[LEVEL_HEIGHT][LEVEL_WIDTH]) {
    char fullPath[256];
    snprintf(fullPath, sizeof(fullPath), "levels/%s.txt", filePath);

    FILE *file = fopen(fullPath, "w");
    if (file == NULL) {
        perror("Erreur lors de l'ouverture du fichier de niveau pour écriture");
        exit(1);
    }

    for (int y = 0; y < LEVEL_HEIGHT; y++) {
        for (int x = 0; x < LEVEL_WIDTH; x++) {
            fprintf(file, "%d ", level[y][x]);
        }
        fprintf(file, "\n");
    }

    fclose(file);
}

int isValidFilename(const char *filename) {
    for (int i = 0; filename[i] != '\0'; i++) {
        if (!(isalnum(filename[i]) || filename[i] == '_' || filename[i] == '-')) {
            return 0;
        }
    }
    return 1;
}