#include <stdio.h>
#include <stdlib.h>
#include <SDL.h>
#include "cJSON.h"

#define TILE_SIZE 16
#define LEVEL_WIDTH 80   
#define LEVEL_HEIGHT 50  

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

    char buffer[1024] = {0};
    
    if (fread(buffer, 1, sizeof(buffer) - 1, file) == 0) {
        perror("Erreur lors de la lecture du fichier");
        fclose(file);
        exit(1);
    }
    fclose(file);

    cJSON *json = cJSON_Parse(buffer);
    if (!json) {
        printf("Erreur de parsing JSON: %s\n", cJSON_GetErrorPtr());
        exit(1);
    }

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

SDL_Texture *loadTexture(const char *filePath, SDL_Renderer *renderer) {
    SDL_Surface *surface = SDL_LoadBMP(filePath);
    if (!surface) {
        printf("Erreur lors du chargement de l'image %s : %s\n", filePath, SDL_GetError());
        return NULL;
    }
    SDL_Texture *texture = SDL_CreateTextureFromSurface(renderer, surface);
    SDL_FreeSurface(surface); 
    return texture;
}

int checkCollision(int x, int y, int level[LEVEL_HEIGHT][LEVEL_WIDTH], int tileType) {
    int tileX = x / TILE_SIZE;
    int tileY = y / TILE_SIZE;

    if (tileX >= 0 && tileX < LEVEL_WIDTH && tileY >= 0 && tileY < LEVEL_HEIGHT) {
        return level[tileY][tileX] == tileType;
    }
    return 0;
}

int checkAboveCollision(int x, int y, int level[LEVEL_HEIGHT][LEVEL_WIDTH], int tileType) {
    int tileX = x / TILE_SIZE;
    int tileY = (y - TILE_SIZE) / TILE_SIZE;

    if (tileX >= 0 && tileX < LEVEL_WIDTH && tileY >= 0 && tileY < LEVEL_HEIGHT) {
        return level[tileY][tileX] == tileType;
    }
    return 0;
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
        printf("Erreur lors du lancement du jeu : %s\n", SDL_GetError());
        SDL_Quit();
        return 1;
    }

    SDL_Renderer *renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);

    SDL_Texture *playerTexture = loadTexture("lumina.bmp", renderer);
    if (!playerTexture) {
        SDL_DestroyRenderer(renderer);
        SDL_DestroyWindow(window);
        SDL_Quit();
        return 1;
    }

    int playerX = 0;
    int playerY = config.height - TILE_SIZE;
    int velocityY = 0;
    int canJump = 2;
    int facingRight = 1;

    int keys[SDL_NUM_SCANCODES] = {0};

    SDL_Event event;
    int running = 1;
    while (running) {
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT) {
                running = 0;
            } else if (event.type == SDL_KEYDOWN) {
                keys[event.key.keysym.scancode] = 1;
            } else if (event.type == SDL_KEYUP) {
                keys[event.key.keysym.scancode] = 0;
            }
        }

        if (keys[SDL_GetScancodeFromKey(config.moveLeftKey)]) {
            playerX -= TILE_SIZE / 4;
            facingRight = 1;
        }
        if (keys[SDL_GetScancodeFromKey(config.moveRightKey)]) {
            playerX += TILE_SIZE / 4;
            facingRight = 0;
        }

        if (keys[SDL_GetScancodeFromKey(config.jumpKey)] && canJump > 0) {
            
            if (!checkAboveCollision(playerX, playerY, level, 7)) { 
                velocityY = -TILE_SIZE;
                canJump--;
            }
        }

        velocityY += 1;
        playerY += velocityY;

        if (velocityY > 0) {
            while (checkCollision(playerX, playerY + TILE_SIZE, level, 7)) {
                playerY--;
                velocityY = 0;
                canJump = 2;
            }
        }

        if (checkCollision(playerX, playerY, level, 8) || 
            checkCollision(playerX + TILE_SIZE - 1, playerY, level, 8) || 
            checkCollision(playerX, playerY + TILE_SIZE - 1, level, 8) || 
            checkCollision(playerX + TILE_SIZE - 1, playerY + TILE_SIZE - 1, level, 8)) {  
            playerX = 0;
            playerY = config.height - TILE_SIZE;  
            velocityY = 0;
            printf("GAME OVER !\n");
        }



        if (checkCollision(playerX, playerY, level, 9)) {
            printf("GAME !\n");
            running = 0;
        }

        // pour pas que le joueur sorte de la map horizontalement
        if (playerX < 0) {
            playerX = 0;
        } else if (playerX > config.width - TILE_SIZE) {
            playerX = config.width - TILE_SIZE;
        }

        // pour pas que le joueur sorte de la map verticalement
        if (playerY < 0) {
            playerY = 0;
            velocityY = 0;  
        } else if (playerY >= config.height - TILE_SIZE) {
            playerY = config.height - TILE_SIZE;
            velocityY = 0;
            canJump = 2;
        }

        SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
        SDL_RenderClear(renderer);

        for (int y = 0; y < LEVEL_HEIGHT; y++) {
            for (int x = 0; x < LEVEL_WIDTH; x++) {
                SDL_Rect rect = {x * TILE_SIZE, y * TILE_SIZE, TILE_SIZE, TILE_SIZE};
                if (level[y][x] == 7) {
                    SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
                } else if (level[y][x] == 8) {
                    SDL_SetRenderDrawColor(renderer, 255, 0, 0, 255);
                } else if (level[y][x] == 9) {
                    SDL_SetRenderDrawColor(renderer, 255, 255, 0, 255);
                } else {
                    continue;
                }
                SDL_RenderFillRect(renderer, &rect);
            }
        }

    SDL_Rect playerRect = {
            playerX,
            playerY,
            TILE_SIZE,
            TILE_SIZE
    };

    if (facingRight) {
        SDL_RenderCopyEx(renderer, playerTexture, NULL, &playerRect, 0, NULL, SDL_FLIP_NONE);
    } else {
        SDL_RenderCopyEx(renderer, playerTexture, NULL, &playerRect, 0, NULL, SDL_FLIP_HORIZONTAL);
    }
        SDL_RenderPresent(renderer);

        SDL_Delay(16);
    }

    SDL_DestroyTexture(playerTexture);
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();

    return 0;
}