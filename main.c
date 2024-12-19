#include <stdio.h>
#include <stdlib.h>
#include <SDL.h>
#include "cJSON.h"
#include <SDL_image.h>
#include <SDL_ttf.h>
#include <string.h>

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

typedef struct {
    int saveX;
    int saveY;
} SaveConfig;

typedef struct {
    int x;
    int y;
    int hasCheckpoint;
    int checkpointX;
    int checkpointY;
    int isCheckpointActive;
} PlayerSpawn;

typedef struct {
    SDL_Texture* background1;
    SDL_Texture* background2;
    SDL_Texture* background3;
    SDL_Texture* player;
    SDL_Texture* ground;
    SDL_Texture* rock;
    SDL_Texture* lamp;
    SDL_Texture* grass;
    SDL_Texture* levelText;
    SDL_Texture* messageText;
} GameTextures;

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
    SDL_Surface *surface = IMG_Load(filePath);
    if (!surface) {
        printf("Erreur lors du chargement de l'image %s : %s\n", filePath, IMG_GetError());
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

//check colision by malo 4pixels de marge
int checkCollision4P(int x, int y, int level[LEVEL_HEIGHT][LEVEL_WIDTH], int tileType) {
    int tileXP = ((x+4) / TILE_SIZE)-1;
    int tileXM = ((x-4) / TILE_SIZE)-1;
    int tileY = y / TILE_SIZE;

   
    return level[tileY][tileXP] == tileType || level[tileY][tileXP] == tileType;
   
    
}


int checkAboveCollision(int x, int y, int level[LEVEL_HEIGHT][LEVEL_WIDTH], int tileType) {
    int tileX = x / TILE_SIZE;
    int tileY = (y - TILE_SIZE) / TILE_SIZE;

    if (tileX >= 0 && tileX < LEVEL_WIDTH && tileY >= 0 && tileY < LEVEL_HEIGHT) {
        return level[tileY][tileX] == tileType;
    }
    return 0;
}

SDL_Texture* createTextTexture(TTF_Font* font, const char* text, SDL_Color color, SDL_Renderer* renderer) {
    SDL_Surface* textSurface = TTF_RenderText_Solid(font, text, color);
    if (!textSurface) {
        return NULL;
    }
    SDL_Texture* texture = SDL_CreateTextureFromSurface(renderer, textSurface);
    SDL_FreeSurface(textSurface);
    return texture;
}

void renderText(SDL_Renderer* renderer, TTF_Font* font, const char* text, SDL_Color color, int x, int y) {
    SDL_Texture* textTexture = createTextTexture(font, text, color, renderer);
    if (textTexture) {
        int textWidth, textHeight;
        SDL_QueryTexture(textTexture, NULL, NULL, &textWidth, &textHeight);
        SDL_Rect textRect = {x, y, textWidth, textHeight};
        SDL_RenderCopy(renderer, textTexture, NULL, &textRect);
        SDL_DestroyTexture(textTexture);
    }
}

int main(int argc, char *argv[]) {
    GameConfig config = loadConfig("config.json");
    int try = 0;
    char tryString[] ={'0','0','0','0',' ', ':', ' ','t','e','n','t','a','t','i','v','e','s','\0'};
    int level[LEVEL_HEIGHT][LEVEL_WIDTH];
    loadLevel("levels/level1.txt", level);

    if (SDL_Init(SDL_INIT_VIDEO) < 0) {
        printf("Erreur lors de l'initialisation de SDL : %s\n", SDL_GetError());
        return 1;
    }

    if (!(IMG_Init(IMG_INIT_PNG) & IMG_INIT_PNG)) {
        printf("Erreur lors de l'initialisation de SDL_image : %s\n", IMG_GetError());
        return 1;
    }

    if (TTF_Init() < 0) {
        printf("Erreur lors de l'initialisation de SDL_ttf : %s\n", TTF_GetError());
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

    TTF_Font* font = TTF_OpenFont("oak_woods_v1.0/fonts/KnightWarrior-w16n8.otf", 24);
    if (!font) {
        printf("Erreur lors du chargement de la police : %s\n", TTF_GetError());
        return 1;
    }

    GameTextures textures;
    
    SDL_Color textColor = {255, 255, 255, 255};
    SDL_Surface* textSurface = TTF_RenderText_Solid(font, "First Level", textColor);

    if (!textSurface) {
        printf("Erreur lors de la création du texte : %s\n", TTF_GetError());
        return 1;
    }
    textures.levelText = SDL_CreateTextureFromSurface(renderer, textSurface);
    SDL_FreeSurface(textSurface);

    textures.background1 = loadTexture("oak_woods_v1.0/background/background_layer_1.png", renderer);
    textures.background2 = loadTexture("oak_woods_v1.0/background/background_layer_2.png", renderer);
    textures.background3 = loadTexture("oak_woods_v1.0/background/background_layer_3.png", renderer);
    textures.player = loadTexture("oak_woods_v1.0/character/malo.png", renderer);
    textures.ground = loadTexture("oak_woods_v1.0/decorations/sol1.png", renderer);
    textures.rock = loadTexture("oak_woods_v1.0/decorations/rock_3.png", renderer);
    textures.lamp = loadTexture("oak_woods_v1.0/decorations/lamp.png", renderer);
    textures.grass = loadTexture("oak_woods_v1.0/decorations/grass_2.png", renderer);

    if (!textures.background1 || !textures.background2 || !textures.background3 || 
        !textures.player || !textures.ground || !textures.rock || !textures.lamp || 
        !textures.grass || !textures.levelText) {
        SDL_DestroyRenderer(renderer);
        SDL_DestroyWindow(window);
        SDL_Quit();
        return 1;
    }

    PlayerSpawn spawn = {
        .x = 0,
        .y = config.height - TILE_SIZE,
        .hasCheckpoint = 0,
        .checkpointX = 0,
        .checkpointY = 0,
        .isCheckpointActive = 0
    };

    int playerX = spawn.x;
    int playerY = spawn.y;
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
                velocityY = -TILE_SIZE / 1.2;
                canJump--;
            }
        }

        velocityY += 1;
        playerY += velocityY;

        // le probleme de velocity qui traverse la plateforme est la 
        //en bas à gauche checkCollision(playerX, playerY + TILE_SIZE - 1, level, 7)
        //en bas à droite checkCollision(playerX + TILE_SIZE - 1, playerY + TILE_SIZE - 1, level, 7)
        //en haut à gauche checkCollision(playerX, playerY, level, 7)
        //en haut à droite checkCollision(playerX + TILE_SIZE - 1, playerY, level, 7)

        //check haut
        if (1) { //if (velocityY = 0)
            while (
                checkCollision(playerX, playerY + TILE_SIZE - 1, level, 7) &&
                checkCollision4P(playerX, playerY, level,7)
            ) 
            {
                playerY--;
                velocityY = 0;
                canJump = 2;
            }
        }

        if (1) { //if (velocityY = 0)
            while (
                checkCollision(playerX + TILE_SIZE - 1, playerY + TILE_SIZE - 1, level, 7) &&
                checkCollision4P(playerX + TILE_SIZE - 1, playerY, level,7)
            ) 
            {
                playerY--;
                velocityY = 0;
                canJump = 2;
            }
        }
        
 //check bas
        if (1) { //if (velocityY = 0)
            while (((checkCollision(playerX, playerY + TILE_SIZE - 1, level, 7) || 
            checkCollision(playerX + TILE_SIZE - 1, playerY + TILE_SIZE -1, level, 7))) &&
            !((checkCollision(playerX, playerY, level, 7) || 
            checkCollision(playerX + TILE_SIZE - 1, playerY , level, 7)))
            )
             {
                playerY--;
                velocityY = 0;
                canJump = 2;
            }
        }

    

       

        if (checkCollision(playerX, playerY, level, 8) || 
            checkCollision(playerX + TILE_SIZE - 1, playerY, level, 8) || 
            checkCollision(playerX, playerY + TILE_SIZE - 1, level, 8) || 
            checkCollision(playerX + TILE_SIZE - 1, playerY + TILE_SIZE - 1, level, 8)) {  
            SDL_Color redColor = {255, 0, 0, 255};
            renderText(renderer, font, "GAME OVER", redColor, config.width/2 - 80, config.height/2);
            try++;
            tryString[3] = (char)(try%10)+48;
            tryString[2] = (char)((try/10)%10)+48;
            tryString[1] = (char)((try/100)%10)+48;
            tryString[0] = (char)((try/1000)%10)+48;
            SDL_RenderPresent(renderer);
            

            SDL_Delay(2000);  // Affiche le message pendant 2 secondes
            
            if (spawn.hasCheckpoint && spawn.isCheckpointActive) {
                playerX = spawn.checkpointX;
                playerY = spawn.checkpointY;
            } else {
                playerX = spawn.x;
                playerY = spawn.y;
            }
            velocityY = 0;
        }

        if (checkCollision(playerX, playerY, level, 9)) {
            SDL_Color greenColor = {0, 255, 0, 255};
            renderText(renderer, font, "YOU WIN!", greenColor, config.width/2 - 80, config.height/2);
            SDL_RenderPresent(renderer);
            SDL_Delay(2000);  // Affiche le message pendant 2 secondes
            running = 0;
        }

        if (checkCollision(playerX, playerY, level, 6)) {
            spawn.checkpointX = playerX;
            spawn.checkpointY = playerY;
            spawn.hasCheckpoint = 1;
            if (!spawn.isCheckpointActive) {
                spawn.isCheckpointActive = 1;
                SDL_Color blueColor = {0, 255, 255, 255};
                renderText(renderer, font, "Checkpoint!", blueColor, config.width/2, config.height/2);
                SDL_RenderPresent(renderer);
                SDL_Delay(500);
            }
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

        // Dessiner les backgrounds
        SDL_Rect bgRect = {0, 0, config.width, config.height};
        SDL_RenderCopy(renderer, textures.background1, NULL, &bgRect);
        SDL_RenderCopy(renderer, textures.background2, NULL, &bgRect);
        SDL_RenderCopy(renderer, textures.background3, NULL, &bgRect);

        for (int y = 0; y < LEVEL_HEIGHT; y++) {
            for (int x = 0; x < LEVEL_WIDTH; x++) {
                SDL_Rect destRect = {x * TILE_SIZE, y * TILE_SIZE, TILE_SIZE, TILE_SIZE};
                if (level[y][x] == 7) {
                    SDL_RenderCopy(renderer, textures.ground, NULL, &destRect);
                } else if (level[y][x] == 8) {
                    SDL_RenderCopy(renderer, textures.rock, NULL, &destRect);
                } else if (level[y][x] == 9) {
                    SDL_RenderCopy(renderer, textures.grass, NULL, &destRect);
                } else if (level[y][x] == 6) {
                    SDL_RenderCopy(renderer, textures.lamp, NULL, &destRect);
                }
            }
        }

        SDL_Rect playerRect = {
            playerX,
            playerY,
            TILE_SIZE,
            TILE_SIZE
        };

        if (facingRight) {
            SDL_RenderCopyEx(renderer, textures.player, NULL, &playerRect, 0, NULL, SDL_FLIP_NONE);
        } else {
            SDL_RenderCopyEx(renderer, textures.player, NULL, &playerRect, 0, NULL, SDL_FLIP_HORIZONTAL);
        }

        // Dessiner le texte en dernier pour qu'il soit au premier plan
        int textWidth, textHeight;
        SDL_Color Color = {200, 200, 200, 255};
        SDL_QueryTexture(textures.levelText, NULL, NULL, &textWidth, &textHeight);
        renderText(renderer, font, tryString, Color, config.width/3 - 400, config.height/42);
        SDL_RenderPresent(renderer);
        SDL_Rect textRect = {
            (config.width - textWidth) / 2,
            20,
            textWidth,
            textHeight
        };
        SDL_RenderCopy(renderer, textures.levelText, NULL, &textRect);

        // Debug
        if (textures.levelText == NULL) {
            printf("La texture du texte est NULL\n");
        }

        SDL_RenderPresent(renderer);

        SDL_Delay(16);
    }

    SDL_DestroyTexture(textures.background1);
    SDL_DestroyTexture(textures.background2);
    SDL_DestroyTexture(textures.background3);
    SDL_DestroyTexture(textures.player);
    SDL_DestroyTexture(textures.ground);
    SDL_DestroyTexture(textures.rock);
    SDL_DestroyTexture(textures.lamp);
    SDL_DestroyTexture(textures.grass);
    SDL_DestroyTexture(textures.levelText);
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();

    TTF_CloseFont(font);
    TTF_Quit();

    return 0;
}