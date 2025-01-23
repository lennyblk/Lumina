#include "../include/level.h"
#include "../include/menu.h"
#include "../include/text.h"
#include "../cJSON.h"
#include <stdio.h>
#include <SDL_ttf.h>
#include <dirent.h> 

void loadLevel(const char *filePath, int level[LEVEL_HEIGHT][LEVEL_WIDTH]) {
    FILE *file = fopen(filePath, "r");
    if (file == NULL) {
        perror("Erreur lors de l'ouverture du fichier de niveau");
        exit(1);
    }

    for (int y = 0; y < LEVEL_HEIGHT; y++) {
        for (int x = 0; x < LEVEL_WIDTH; x++) {
            fscanf(file, "%d", &level[y][x]);
        }
    }

    fclose(file);
}

void loadLevelsFromDirectory(const char *directory, LevelsMenuTextures *levelsMenuTextures, TTF_Font *font, SDL_Renderer *renderer, char levelNames[MAX_LEVELS][256]) {
    DIR *dir;
    struct dirent *ent;    // on fait un pointeur de structure car la fonction readdir() renvoie un pointeur de structure
    SDL_Color textColor = {255, 255, 255, 255};
    levelsMenuTextures->levelCount = 0;

    if ((dir = opendir(directory)) != NULL) {
        while ((ent = readdir(dir)) != NULL) {      // chaque iteration, lit un fichier du répertoire et le met dans la structure dirent -> d_name
            if (strstr(ent->d_name, ".txt") != NULL) {
                char levelName[256];
                snprintf(levelName, sizeof(levelName), "%s", ent->d_name);
                levelName[strlen(levelName) - 4] = '\0'; // retire ".txt" extension
                levelsMenuTextures->levelTexts[levelsMenuTextures->levelCount] = createTextTexture(font, levelName, textColor, renderer);
                snprintf(levelNames[levelsMenuTextures->levelCount], sizeof(levelNames[levelsMenuTextures->levelCount]), "levels/%s", ent->d_name);
                levelsMenuTextures->levelCount++;
                if (levelsMenuTextures->levelCount >= MAX_LEVELS) {
                    break;
                }
            }
        }
        closedir(dir);
    } else {
        perror("Erreur lors de l'ouverture du répertoire des niveaux");
    }
}

