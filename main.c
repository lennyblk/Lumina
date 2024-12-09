#include <stdio.h>
#include <stdlib.h>
#include <SDL.h>
#include "cJSON.h"

#define TILE_SIZE 50
#define LEVEL_WIDTH 16
#define LEVEL_HEIGHT 12

typedef struct {
    int volume;
    int width;
    int height;
    SDL_Keycode jumpKey;
    SDL_Keycode dashKey;
    SDL_Keycode moveLeftKey;
    SDL_Keycode moveRightKey;
} GameConfig;

GameConfig loadConfig(const char *filePath) {
    GameConfig config;
    FILE *file = fopen(filePath, "r");
    if (file == NULL) {
        perror("Erreur lors de l'ouverture du fichier de configuration");
        exit(1);
    }

    // Lire le fichier JSON
    fseek(file, 0, SEEK_END);
    long fileSize = ftell(file);
    rewind(file);

    char *data = (char *)malloc(fileSize + 1);
    fread(data, 1, fileSize, file);
    data[fileSize] = '\0';
    fclose(file);

    cJSON *json = cJSON_Parse(data);
    free(data);

    if (!json) {
        printf("Erreur de parsing JSON: %s\n", cJSON_GetErrorPtr());
        exit(1);
    }

    // Extraire les valeurs du JSON
    config.volume = cJSON_GetObjectItem(json, "volume")->valueint;
    config.width = cJSON_GetObjectItem(json, "resolution_width")->valueint;
    config.height = cJSON_GetObjectItem(json, "resolution_height")->valueint;
    config.jumpKey = cJSON_GetObjectItem(json, "jumpKey")->valueint;
    config.dashKey = cJSON_GetObjectItem(json, "dashKey")->valueint;
    config.moveLeftKey = cJSON_GetObjectItem(json, "moveLeftKey")->valueint;
    config.moveRightKey = cJSON_GetObjectItem(json, "moveRightKey")->valueint;

    cJSON_Delete(json);
    return config;
}

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

int main(int argc, char *argv[]) {
    GameConfig config = loadConfig("config.json");

    int level[LEVEL_HEIGHT][LEVEL_WIDTH];
    loadLevel("levels/level1.txt", level);

    if (SDL_Init(SDL_INIT_VIDEO) < 0) {
        printf("Erreur lors de l'initialisation de SDL : %s\n", SDL_GetError());
        return 1;
    }

    SDL_Window *window = SDL_CreateWindow(
        "Lumina",
        SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
        config.width, config.height, SDL_WINDOW_SHOWN
    );

    if (!window) {
        printf("Erreur lors de la création de la fenêtre : %s\n", SDL_GetError());
        SDL_Quit();
        return 1;
    }

    SDL_Renderer *renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);

    int playerX = 0;
    int playerY = config.height - TILE_SIZE;
    int velocityY = 0;
    int canJump = 2;

    SDL_Event event;
    int running = 1;
    while (running) {
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT) {
                running = 0;
            } else if (event.type == SDL_KEYDOWN) {
                if (event.key.keysym.sym == config.jumpKey && canJump > 0) {
                    velocityY = -15;
                    canJump--;
                }
                if (event.key.keysym.sym == config.moveLeftKey) {
                    playerX -= 5;
                }
                if (event.key.keysym.sym == config.moveRightKey) {
                    playerX += 5;
                }
            }
        }

        velocityY += 1;
        playerY += velocityY;

        if (playerY >= config.height - TILE_SIZE) {
            playerY = config.height - TILE_SIZE;
            velocityY = 0;
            canJump = 2;
        }

        SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
        SDL_RenderClear(renderer);

        SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
        for (int y = 0; y < LEVEL_HEIGHT; y++) {
            for (int x = 0; x < LEVEL_WIDTH; x++) {
                if (level[y][x] == 7) {
                    SDL_Rect rect = {x * TILE_SIZE, y * TILE_SIZE, TILE_SIZE, TILE_SIZE};
                    SDL_RenderFillRect(renderer, &rect);
                }
            }
        }

        SDL_SetRenderDrawColor(renderer, 255, 0, 0, 255);
        SDL_Rect playerRect = {playerX, playerY, TILE_SIZE, TILE_SIZE};
        SDL_RenderFillRect(renderer, &playerRect);

        SDL_RenderPresent(renderer);

        SDL_Delay(16);
    }

    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();

    return 0;
}
