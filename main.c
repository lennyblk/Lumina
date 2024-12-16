#include <stdio.h>
#include <stdlib.h>
#include <SDL.h>
#include "cJSON.h"

// Configuration de la taille des tiles
#define TILE_SIZE 16
#define LEVEL_WIDTH 80   // Largeur de la matrice (80 tiles de large)
#define LEVEL_HEIGHT 50  // Hauteur de la matrice (50 tiles de hauteur)

typedef struct {
    int volume;
    int width;
    int height;
    SDL_Keycode jumpKey;
    SDL_Keycode dashKey;
    SDL_Keycode moveLeftKey;
    SDL_Keycode moveRightKey;
} GameConfig;

// Charger le fichier de configuration JSON
GameConfig loadConfig(const char *filePath) {
    GameConfig config;
    FILE *file = fopen(filePath, "r");
    if (file == NULL) {
        perror("Erreur lors de l'ouverture du fichier de configuration");
        exit(1);
    }

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

// Charger un niveau à partir d'un fichier texte
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

// Charger une texture depuis une image
SDL_Texture *loadTexture(const char *filePath, SDL_Renderer *renderer) {
    SDL_Surface *surface = SDL_LoadBMP(filePath);
    if (!surface) {
        printf("Erreur lors du chargement de l'image %s : %s\n", filePath, SDL_GetError());
        return NULL;
    }
    SDL_Texture *texture = SDL_CreateTextureFromSurface(renderer, surface);
    SDL_FreeSurface(surface); // Libérer la surface après création de la texture
    return texture;
}

// Vérifier la collision avec un type de bloc
int checkCollision(int x, int y, int level[LEVEL_HEIGHT][LEVEL_WIDTH], int tileType) {
    int tileX = x / TILE_SIZE;
    int tileY = y / TILE_SIZE;

    if (tileX >= 0 && tileX < LEVEL_WIDTH && tileY >= 0 && tileY < LEVEL_HEIGHT) {
        return level[tileY][tileX] == tileType;
    }
    return 0;
}

// Fonction de vérification de collision au-dessus du personnage sur toute sa largeur
int checkAboveCollision(int x, int y, int level[LEVEL_HEIGHT][LEVEL_WIDTH], int tileType) {
    int tileXStart = x / TILE_SIZE; // Position X de départ du personnage
    int tileXEnd = (x + TILE_SIZE - 1) / TILE_SIZE; // Position X de fin du personnage

    int tileY = (y - TILE_SIZE) / TILE_SIZE; // Vérifier la ligne juste au-dessus du personnage

    // Vérifier sur toute la largeur du personnage
    for (int tileX = tileXStart; tileX <= tileXEnd; tileX++) {
        if (tileX >= 0 && tileX < LEVEL_WIDTH && tileY >= 0 && tileY < LEVEL_HEIGHT) {
            if (level[tileY][tileX] == tileType) {
                return 1; // Collision avec un bloc au-dessus
            }
        }
    }
    return 0; // Pas de collision au-dessus
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

    // Charger l'image lumine.bmp pour le personnage
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
    int facingRight = 1; // 1 = droite, 0 = gauche

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
            facingRight = 1; // Tourner à gauche
        }
        if (keys[SDL_GetScancodeFromKey(config.moveRightKey)]) {
            playerX += TILE_SIZE / 4;
            facingRight = 0; // Tourner à droite
        }

        if (keys[SDL_GetScancodeFromKey(config.jumpKey)] && canJump > 0) {
            // Vérifier qu'il n'y a pas de blocs juste au-dessus avant de sauter
            if (!checkAboveCollision(playerX, playerY, level, 7)) {  // Par exemple, les blocs solides ont l'ID 7
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

        // Vérifier la collision avec un bloc rouge sur tous les côtés du joueur
        if (checkCollision(playerX, playerY, level, 8) || 
            checkCollision(playerX + TILE_SIZE - 1, playerY, level, 8) || // Côté droit
            checkCollision(playerX, playerY + TILE_SIZE - 1, level, 8) || // Côté bas
            checkCollision(playerX + TILE_SIZE - 1, playerY + TILE_SIZE - 1, level, 8)) {  // Coin bas droit
            // Si collision avec un bloc rouge, réinitialiser la position
            playerX = 0;
            playerY = config.height - TILE_SIZE;  // Position de départ
            velocityY = 0;
        }



        if (checkCollision(playerX, playerY, level, 9)) {
            // Si collision avec la fin de niveau, arrêter le jeu
            printf("Niveau terminé !\n");
            running = 0;
        }

        if (playerY >= config.height - TILE_SIZE) {
            playerY = config.height - TILE_SIZE;
            velocityY = 0;
            canJump = 2;
        }

        SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
        SDL_RenderClear(renderer);

        // Dessiner les blocs
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

        // Dessiner le joueur avec la texture
        SDL_Rect playerRect = {playerX, playerY, TILE_SIZE, TILE_SIZE};
        SDL_RenderCopyEx(renderer, playerTexture, NULL, &playerRect, 0, NULL,
                         facingRight ? SDL_FLIP_NONE : SDL_FLIP_HORIZONTAL);

        SDL_RenderPresent(renderer);

        SDL_Delay(16);
    }

    SDL_DestroyTexture(playerTexture);
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();

    return 0;
}