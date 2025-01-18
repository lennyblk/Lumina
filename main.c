#include <stdio.h>
#include <stdlib.h>
#include <SDL.h>
#include "cJSON.h"
#include <SDL_image.h>
#include <SDL_ttf.h>
#include <string.h>
#include <ctype.h> // Include this header for isalnum
#include <dirent.h> // Include this header for directory operations

#define TILE_SIZE 16
#define LEVEL_WIDTH 80   
#define LEVEL_HEIGHT 50  
#define MAX_LEVELS 100

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

typedef struct {
    SDL_Texture* run[8];
    SDL_Texture* jump;
    SDL_Texture* idle;
} PlayerTextures;

typedef enum {
    MENU,
    LEVELS,
    SETTINGS,
    PLAYING,
    PAUSED, // Add new game state
    CREATE_LEVEL, // Add new game state
    EXIT
} GameState;

typedef struct {
    SDL_Texture* titleText;
    SDL_Texture* levelsText;
    SDL_Texture* settingsText;
    SDL_Texture* exitText;
} MenuTextures;

typedef struct {
    SDL_Texture* levelTexts[MAX_LEVELS];
    SDL_Texture* createLevelText; // Add new texture for create level
    SDL_Texture* backText;
    int levelCount;
} LevelsMenuTextures;

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
    int tileX = x / TILE_SIZE;
    int tileYM = (y / TILE_SIZE);
    int tileYP = ((y+4) / TILE_SIZE)+1;


   
    return level[tileYP][tileX] == tileType || level[tileYM][tileX] == tileType;
   
    
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

void renderTextWithBackground(SDL_Renderer* renderer, TTF_Font* font, const char* text, SDL_Color textColor, SDL_Color bgColor, int x, int y) {
    SDL_Surface* textSurface = TTF_RenderText_Solid(font, text, textColor);
    if (!textSurface) {
        return;
    }
    SDL_Texture* textTexture = SDL_CreateTextureFromSurface(renderer, textSurface);
    int textWidth, textHeight;
    SDL_QueryTexture(textTexture, NULL, NULL, &textWidth, &textHeight);
    SDL_Rect textRect = {x, y, textWidth, textHeight};
    SDL_SetRenderDrawColor(renderer, bgColor.r, bgColor.g, bgColor.b, bgColor.a);
    SDL_RenderFillRect(renderer, &textRect);
    SDL_RenderCopy(renderer, textTexture, NULL, &textRect);
    SDL_DestroyTexture(textTexture);
    SDL_FreeSurface(textSurface);
}

int isMouseOverButton(int mouseX, int mouseY, SDL_Rect buttonRect) {
    return mouseX >= buttonRect.x && mouseX <= buttonRect.x + buttonRect.w &&
           mouseY >= buttonRect.y && mouseY <= buttonRect.y + buttonRect.h;
}

void renderMenu(SDL_Renderer* renderer, MenuTextures* menuTextures, SDL_Rect* levelsRect, SDL_Rect* settingsRect, SDL_Rect* exitRect) {
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
    SDL_RenderClear(renderer);

    int textWidth, textHeight;

    SDL_QueryTexture(menuTextures->titleText, NULL, NULL, &textWidth, &textHeight);
    SDL_Rect titleRect = {300, 100, textWidth, textHeight};
    SDL_RenderCopy(renderer, menuTextures->titleText, NULL, &titleRect);

    SDL_QueryTexture(menuTextures->levelsText, NULL, NULL, &textWidth, &textHeight);
    *levelsRect = (SDL_Rect){350, 200, textWidth, textHeight};
    SDL_RenderCopy(renderer, menuTextures->levelsText, NULL, levelsRect);

    SDL_QueryTexture(menuTextures->settingsText, NULL, NULL, &textWidth, &textHeight);
    *settingsRect = (SDL_Rect){350, 300, textWidth, textHeight};
    SDL_RenderCopy(renderer, menuTextures->settingsText, NULL, settingsRect);

    SDL_QueryTexture(menuTextures->exitText, NULL, NULL, &textWidth, &textHeight);
    *exitRect = (SDL_Rect){350, 400, textWidth, textHeight};
    SDL_RenderCopy(renderer, menuTextures->exitText, NULL, exitRect);

    SDL_RenderPresent(renderer);
}

void renderLevelsMenu(SDL_Renderer* renderer, LevelsMenuTextures* levelsMenuTextures, SDL_Rect* levelRects, SDL_Rect* createLevelRect, SDL_Rect* backRect) {
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
    SDL_RenderClear(renderer);

    int textWidth, textHeight;

    for (int i = 0; i < levelsMenuTextures->levelCount; i++) {
        SDL_QueryTexture(levelsMenuTextures->levelTexts[i], NULL, NULL, &textWidth, &textHeight);
        levelRects[i] = (SDL_Rect){350, 200 + i * 50, textWidth, textHeight};
        SDL_RenderCopy(renderer, levelsMenuTextures->levelTexts[i], NULL, &levelRects[i]);
    }

    SDL_QueryTexture(levelsMenuTextures->createLevelText, NULL, NULL, &textWidth, &textHeight); // Add new create level text
    *createLevelRect = (SDL_Rect){350, 200 + levelsMenuTextures->levelCount * 50, textWidth, textHeight};
    SDL_RenderCopy(renderer, levelsMenuTextures->createLevelText, NULL, createLevelRect);

    SDL_QueryTexture(levelsMenuTextures->backText, NULL, NULL, &textWidth, &textHeight);
    *backRect = (SDL_Rect){350, 200 + (levelsMenuTextures->levelCount + 1) * 50, textWidth, textHeight};
    SDL_RenderCopy(renderer, levelsMenuTextures->backText, NULL, backRect);

    SDL_RenderPresent(renderer);
}

void renderSettings(SDL_Renderer* renderer, TTF_Font* font, GameConfig* config, SDL_Texture* backText, SDL_Rect* backRect) {
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
    SDL_RenderClear(renderer);

    SDL_Color textColor = {255, 255, 255, 255};
    char buffer[256];
    int textWidth, textHeight;

    snprintf(buffer, sizeof(buffer), "Volume: %d", config->volume);
    renderText(renderer, font, buffer, textColor, 100, 100);

    snprintf(buffer, sizeof(buffer), "Resolution: %dx%d", config->width, config->height);
    renderText(renderer, font, buffer, textColor, 100, 150);

    snprintf(buffer, sizeof(buffer), "Jump Key: %d", config->jumpKey);
    renderText(renderer, font, buffer, textColor, 100, 200);

    snprintf(buffer, sizeof(buffer), "Dash Key: %d", config->dashKey);
    renderText(renderer, font, buffer, textColor, 100, 250);

    snprintf(buffer, sizeof(buffer), "Move Left Key: %d", config->moveLeftKey);
    renderText(renderer, font, buffer, textColor, 100, 300);

    snprintf(buffer, sizeof(buffer), "Move Right Key: %d", config->moveRightKey);
    renderText(renderer, font, buffer, textColor, 100, 350);

    SDL_QueryTexture(backText, NULL, NULL, &textWidth, &textHeight);
    *backRect = (SDL_Rect){350, 400, textWidth, textHeight};
    SDL_RenderCopy(renderer, backText, NULL, backRect);

    SDL_RenderPresent(renderer);
}

void renderCreateLevel(SDL_Renderer* renderer, int level[LEVEL_HEIGHT][LEVEL_WIDTH], SDL_Texture* saveText, SDL_Rect* saveRect, int cursorX, int cursorY, SDL_Texture* backToMenuText, SDL_Rect* backToMenuRect, TTF_Font* font, GameConfig* config) {
    SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255); // White background
    SDL_RenderClear(renderer);

    for (int y = 0; y < LEVEL_HEIGHT; y++) {
        for (int x = 0; x < LEVEL_WIDTH; x++) {
            SDL_Rect destRect = {x * TILE_SIZE, y * TILE_SIZE, TILE_SIZE, TILE_SIZE};
            if (x == cursorX && y == cursorY) {
                SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255); // Black for cursor
            } else if (level[y][x] == 7) {
                SDL_SetRenderDrawColor(renderer, 139, 69, 19, 255); // Brown for solid blocks
            } else if (level[y][x] == 8) {
                SDL_SetRenderDrawColor(renderer, 255, 0, 0, 255); // Red for spikes
            } else if (level[y][x] == 9) {
                SDL_SetRenderDrawColor(renderer, 0, 255, 0, 255); // Green for finish line
            } else if (level[y][x] == 6) {
                SDL_SetRenderDrawColor(renderer, 0, 0, 255, 255); // Blue for checkpoint
            } else {
                SDL_SetRenderDrawColor(renderer, 200, 200, 200, 255); // Light grey for empty space
            }
            SDL_RenderFillRect(renderer, &destRect);
            SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255); // Black border
            SDL_RenderDrawRect(renderer, &destRect);
        }
    }

    SDL_QueryTexture(saveText, NULL, NULL, &saveRect->w, &saveRect->h);
    saveRect->x = (config->width - saveRect->w) / 2; // Center horizontally
    saveRect->y = 10; // Top of the window
    SDL_Color highlightColor = {255, 255, 0, 255}; // Yellow highlight
    SDL_Color blackColor = {0, 0, 0, 255};
    renderTextWithBackground(renderer, font, "Save", blackColor, highlightColor, saveRect->x, saveRect->y);
    SDL_RenderCopy(renderer, saveText, NULL, saveRect);

    SDL_QueryTexture(backToMenuText, NULL, NULL, &backToMenuRect->w, &backToMenuRect->h);
    renderTextWithBackground(renderer, font, "Back to Menu", blackColor, highlightColor, backToMenuRect->x, backToMenuRect->y);
    SDL_RenderCopy(renderer, backToMenuText, NULL, backToMenuRect);

    renderTextWithBackground(renderer, font, "- 6 for checkpoint", blackColor, highlightColor, 10, 10);
    renderTextWithBackground(renderer, font, "- 7 for solid blocks", blackColor, highlightColor, 10, 40);
    renderTextWithBackground(renderer, font, "- 8 for spikes", blackColor, highlightColor, 10, 70);
    renderTextWithBackground(renderer, font, "- 9 for finish line", blackColor, highlightColor, 10, 100);

    SDL_RenderPresent(renderer);
}

void saveConfig(const char *filePath, GameConfig *config) {
    cJSON *json = cJSON_CreateObject();
    cJSON_AddNumberToObject(json, "volume", config->volume);
    cJSON_AddNumberToObject(json, "resolution_width", config->width);
    cJSON_AddNumberToObject(json, "resolution_height", config->height);
    cJSON_AddNumberToObject(json, "jumpKey", config->jumpKey);
    cJSON_AddNumberToObject(json, "dashKey", config->dashKey);
    cJSON_AddNumberToObject(json, "moveLeftKey", config->moveLeftKey);
    cJSON_AddNumberToObject(json, "moveRightKey", config->moveRightKey);

    char *jsonString = cJSON_Print(json);
    FILE *file = fopen(filePath, "w");
    if (file == NULL) {
        perror("Erreur lors de l'ouverture du fichier de configuration pour écriture");
        exit(1);
    }
    fputs(jsonString, file);
    fclose(file);

    cJSON_Delete(json);
    free(jsonString);
}

void saveLevel(const char *filePath, int level[LEVEL_HEIGHT][LEVEL_WIDTH]) {
    FILE *file = fopen(filePath, "w");
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

void loadLevelsFromDirectory(const char *directory, LevelsMenuTextures *levelsMenuTextures, TTF_Font *font, SDL_Renderer *renderer, char levelNames[MAX_LEVELS][256]) {
    DIR *dir;
    struct dirent *ent;
    SDL_Color textColor = {255, 255, 255, 255};
    levelsMenuTextures->levelCount = 0;

    if ((dir = opendir(directory)) != NULL) {
        while ((ent = readdir(dir)) != NULL) {
            if (strstr(ent->d_name, ".txt") != NULL) {
                char levelName[256];
                snprintf(levelName, sizeof(levelName), "%s", ent->d_name);
                levelName[strlen(levelName) - 4] = '\0'; // Remove the ".txt" extension
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
    SDL_Color blackColor = {0, 0, 0, 255}; // Define black color
    SDL_Color highlightColor = {255, 255, 0, 255}; // Yellow highlight
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
    textures.ground = loadTexture("oak_woods_v1.0/decorations/sol1.png", renderer);
    textures.rock = loadTexture("oak_woods_v1.0/decorations/rock_3.png", renderer);
    textures.lamp = loadTexture("oak_woods_v1.0/decorations/lamp.png", renderer);
    textures.grass = loadTexture("oak_woods_v1.0/decorations/grass_2.png", renderer);

    if (!textures.background1 || !textures.background2 || !textures.background3 || !textures.ground || !textures.rock || !textures.lamp || 
        !textures.grass || !textures.levelText) {
        SDL_DestroyRenderer(renderer);
        SDL_DestroyWindow(window);
        SDL_Quit();
        return 1;
    }

    MenuTextures menuTextures;
    menuTextures.titleText = createTextTexture(font, "Menu Principal", textColor, renderer);
    menuTextures.levelsText = createTextTexture(font, "Levels", textColor, renderer);
    menuTextures.settingsText = createTextTexture(font, "Settings", textColor, renderer);
    menuTextures.exitText = createTextTexture(font, "Exit", textColor, renderer);

    if (!menuTextures.titleText || !menuTextures.levelsText || !menuTextures.settingsText || !menuTextures.exitText) {
        printf("Erreur lors de la création des textures de texte du menu\n");
        return 1;
    }

    LevelsMenuTextures levelsMenuTextures;
    levelsMenuTextures.createLevelText = createTextTexture(font, "Create your own level", textColor, renderer); // Add new create level text
    levelsMenuTextures.backText = createTextTexture(font, "Back", textColor, renderer);

    if (!levelsMenuTextures.createLevelText || !levelsMenuTextures.backText) {
        printf("Erreur lors de la création des textures de texte du menu des niveaux\n");
        return 1;
    }

    PlayerTextures playerTextures;
        for (int i = 0; i < 8; i++) {
            char filePath[256];
            snprintf(filePath, sizeof(filePath), "player_assets/run/run-frame%d.png", i + 1);
            playerTextures.run[i] = loadTexture(filePath, renderer);
        }
        playerTextures.jump = loadTexture("player_assets/jump/jump-frame1.png", renderer);
        playerTextures.idle = loadTexture("player_assets/idle/debout-frame1.png", renderer);

    // Store level names for later use
    char levelNames[MAX_LEVELS][256]; // Array to store level names
    loadLevelsFromDirectory("levels", &levelsMenuTextures, font, renderer, levelNames);

    GameState gameState = MENU;

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
    int facingRight = 0;
    

    int keys[SDL_NUM_SCANCODES] = {0};

    SDL_Event event;
    int running = 1;
    
    SDL_Rect levelsRect, settingsRect, exitRect;
    SDL_Rect levelRects[MAX_LEVELS], createLevelRect, backRect;
    SDL_Rect saveRect; // Remove redeclaration of createLevelRect
    SDL_Texture* saveText = createTextTexture(font, "Save", blackColor, renderer); // Change color to black
    int customLevel[LEVEL_HEIGHT][LEVEL_WIDTH] = {0}; // Initialize custom level matrix

    SDL_Rect pauseButtonRect = {config.width - 50, 10, 40, 40}; // Position and size of the pause button
    SDL_Rect playButtonRect = {config.width / 2 - 50, config.height / 2 - 20, 100, 40}; // Position and size of the play button
    SDL_Rect backToMenuButtonRect = {config.width / 2 - 50, config.height / 2 + 30, 100, 40}; // Position and size of the back to menu button

    SDL_Texture* pauseButtonTexture = createTextTexture(font, "Pause", textColor, renderer);
    SDL_Texture* playButtonTexture = createTextTexture(font, "Play", textColor, renderer);
    SDL_Texture* backToMenuButtonTexture = createTextTexture(font, "Back to Menu", textColor, renderer);

    char levelText[256] = "First Level"; // Variable pour stocker le texte du niveau
    SDL_Texture* levelTextTexture = NULL; // Texture pour le texte du niveau

    int cursorX = 0, cursorY = LEVEL_HEIGHT - 1; // Cursor position for keyboard input, start at bottom left

    SDL_Rect backToMenuRect = {config.width - 150, 10, 140, 40}; // Position and size of the back to menu button
    SDL_Texture* backToMenuText = createTextTexture(font, "Back to Menu", blackColor, renderer); // Change color to black

    char filename[256] = "";
    int enteringFilename = 0;

    int frame = 0;
    int frameDelay = 100; // Delay between frames in milliseconds
    Uint32 frameStart = SDL_GetTicks();

    while (running) {
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT) {
                running = 0;
            } else if (event.type == SDL_KEYDOWN) {
                keys[event.key.keysym.scancode] = 1;
                if (gameState == MENU) {
                    switch (event.key.keysym.sym) {
                        case SDLK_1:
                            gameState = LEVELS;
                            break;
                        case SDLK_2:
                            gameState = SETTINGS;
                            break;
                        case SDLK_3:
                            running = 0;
                            break;
                    }
                } else if (gameState == LEVELS) {
                    // ...existing code for level selection...
                } else if (gameState == SETTINGS) {
                    // ...existing code for settings...
                } else if (gameState == PAUSED) {
                    // Handle key events in paused state if needed
                } else if (gameState == CREATE_LEVEL) {
                    if (enteringFilename) {
                        if (event.key.keysym.sym == SDLK_RETURN) {
                            if (isValidFilename(filename)) {
                                saveLevelWithFilename(filename, customLevel);
                                loadLevelsFromDirectory("levels", &levelsMenuTextures, font, renderer, levelNames); // Refresh levels menu
                                gameState = LEVELS; // Return to LEVELS state
                            } else {
                                printf("Invalid filename. Use only alphanumeric characters, underscores, and hyphens.\n");
                            }
                            enteringFilename = 0;
                        } else if (event.key.keysym.sym == SDLK_BACKSPACE && strlen(filename) > 0) {
                            filename[strlen(filename) - 1] = '\0';
                        } else if (event.key.keysym.sym >= SDLK_a && event.key.keysym.sym <= SDLK_z) {
                            char c = (char)event.key.keysym.sym;
                            strncat(filename, &c, 1);
                        }
                    } else {
                        if (event.key.keysym.sym == SDLK_UP && cursorY > 0) {
                            cursorY--;
                        } else if (event.key.keysym.sym == SDLK_DOWN && cursorY < LEVEL_HEIGHT - 1) {
                            cursorY++;
                        } else if (event.key.keysym.sym == SDLK_LEFT && cursorX > 0) {
                            cursorX--;
                        } else if (event.key.keysym.sym == SDLK_RIGHT && cursorX < LEVEL_WIDTH - 1) {
                            cursorX++;
                        } else if (event.key.keysym.sym == SDLK_6) {
                            customLevel[cursorY][cursorX] = (customLevel[cursorY][cursorX] == 6) ? 0 : 6; // Toggle checkpoint
                        } else if (event.key.keysym.sym == SDLK_7) {
                            customLevel[cursorY][cursorX] = (customLevel[cursorY][cursorX] == 7) ? 0 : 7; // Toggle solid block
                        } else if (event.key.keysym.sym == SDLK_8) {
                            customLevel[cursorY][cursorX] = (customLevel[cursorY][cursorX] == 8) ? 0 : 8; // Toggle spike
                        } else if (event.key.keysym.sym == SDLK_9) {
                            customLevel[cursorY][cursorX] = (customLevel[cursorY][cursorX] == 9) ? 0 : 9; // Toggle finish line
                        }
                    }
                }
            } else if (event.type == SDL_KEYUP) {
                keys[event.key.keysym.scancode] = 0;
            } else if (event.type == SDL_MOUSEBUTTONDOWN) {
                int mouseX = event.button.x;
                int mouseY = event.button.y;
                if (gameState == MENU) {
                    if (isMouseOverButton(mouseX, mouseY, levelsRect)) {
                        gameState = LEVELS;
                    } else if (isMouseOverButton(mouseX, mouseY, settingsRect)) {
                        gameState = SETTINGS;
                    } else if (isMouseOverButton(mouseX, mouseY, exitRect)) {
                        running = 0;
                    }
                } else if (gameState == LEVELS) {
                    for (int i = 0; i < levelsMenuTextures.levelCount; i++) {
                        if (isMouseOverButton(mouseX, mouseY, levelRects[i])) {
                            loadLevel(levelNames[i], level);
                            gameState = PLAYING; // Change to PLAYING state
                            strcpy(levelText, levelNames[i]); // Mettre à jour le texte du niveau
                            if (levelTextTexture) {
                                SDL_DestroyTexture(levelTextTexture);
                            }
                            levelTextTexture = createTextTexture(font, levelText, textColor, renderer);
                            break;
                        }
                    }
                    if (isMouseOverButton(mouseX, mouseY, createLevelRect)) {
                        gameState = CREATE_LEVEL; // Change to CREATE_LEVEL state
                    } else if (isMouseOverButton(mouseX, mouseY, backRect)) {
                        gameState = MENU;
                    }
                } else if (gameState == SETTINGS) {
                    if (isMouseOverButton(mouseX, mouseY, backRect)) {
                        gameState = MENU;
                    }
                } else if (gameState == PLAYING) {
                    if (isMouseOverButton(mouseX, mouseY, pauseButtonRect)) {
                        gameState = PAUSED; // Change to PAUSED state
                    }
                } else if (gameState == PAUSED) {
                    if (isMouseOverButton(mouseX, mouseY, playButtonRect)) {
                        gameState = PLAYING; // Change back to PLAYING state
                    } else if (isMouseOverButton(mouseX, mouseY, backToMenuButtonRect)) {
                        // Reset player position
                        playerX = spawn.x;
                        playerY = spawn.y;
                        velocityY = 0;
                        canJump = 2;
                        gameState = MENU; // Change to MENU state
                    }
                } else if (gameState == CREATE_LEVEL) {
                    int tileX = mouseX / TILE_SIZE;
                    int tileY = mouseY / TILE_SIZE;
                    if (tileX >= 0 && tileX < LEVEL_WIDTH && tileY >= 0 && tileY < LEVEL_HEIGHT) {
                        customLevel[tileY][tileX] = (customLevel[tileY][tileX] + 1) % 10; // Cycle through tile types
                    }
                    if (isMouseOverButton(mouseX, mouseY, saveRect)) {
                        enteringFilename = 1; // Start entering filename
                    } else if (isMouseOverButton(mouseX, mouseY, backToMenuRect)) {
                        gameState = MENU; // Return to MENU state
                    }
                }
            } 
        }

        if (gameState == MENU) {
            renderMenu(renderer, &menuTextures, &levelsRect, &settingsRect, &exitRect);
        } else if (gameState == LEVELS) {
            renderLevelsMenu(renderer, &levelsMenuTextures, levelRects, &createLevelRect, &backRect);
            SDL_RenderPresent(renderer);
        } else if (gameState == SETTINGS) {
            renderSettings(renderer, font, &config, levelsMenuTextures.backText, &backRect);
        } else if (gameState == PLAYING) { 
            if (keys[SDL_GetScancodeFromKey(config.moveLeftKey)]) {
                playerX -= TILE_SIZE / 4;
                facingRight = 0;
            }
            if (keys[SDL_GetScancodeFromKey(config.moveRightKey)]) {
                playerX += TILE_SIZE / 4;
                facingRight = 1;
            }

            if (keys[SDL_GetScancodeFromKey(config.jumpKey)] && canJump > 0) {
                if (!checkAboveCollision(playerX, playerY, level, 7)) { 
                    velocityY = -TILE_SIZE / 1.2;
                    canJump--;
                }
            }
            if(velocityY<10){
            velocityY += 1;
            }
            playerY += velocityY;

            // le probleme de velocity qui traverse la plateforme est la 
            //en bas à gauche checkCollision(playerX, playerY + TILE_SIZE - 1, level, 7)
            //en bas à droite checkCollision(playerX + TILE_SIZE - 1, playerY + TILE_SIZE - 1, level, 7)
            //en haut à gauche checkCollision(playerX, playerY, level, 7)
            //en haut à droite checkCollision(playerX + TILE_SIZE - 1, playerY, level, 7)

            if (1) { // check mur gauche
                while (
                    checkCollision(playerX, playerY, level, 7) &&
                    checkCollision(playerX, playerY + TILE_SIZE - 1, level, 7)
                ) 
                {
                    playerX ++;
                }
            }
            
            if (1) { // check mur droit
                while (
                    checkCollision(playerX + TILE_SIZE - 1, playerY, level, 7) &&
                    checkCollision(playerX + TILE_SIZE - 1, playerY + TILE_SIZE - 1, level, 7)
                ) 
                {
                    playerX --;
                }
            }

            if (1) { // check plafond
                while (
                    (checkCollision(playerX, playerY, level, 7) &&
                    !(checkCollision(playerX, playerY + TILE_SIZE - 1, level, 7)) ||
                    checkCollision(playerX + TILE_SIZE - 1, playerY, level, 7) &&
                    !(checkCollision(playerX + TILE_SIZE - 1, playerY + TILE_SIZE - 1, level, 7))
                    )
                ) 
                {
                    playerY++;
                    velocityY = 0;

                }
            }

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

            if (checkCollision(playerX, playerY, level, 8) || 
                checkCollision(playerX + TILE_SIZE - 1, playerY, level, 8) || 
                checkCollision(playerX, playerY + TILE_SIZE - 1, level, 8) || 
                checkCollision(playerX + TILE_SIZE - 1, playerY + TILE_SIZE - 1, level, 8)) {  
                SDL_Color redColor = {255, 0, 0, 255};
                renderText(renderer, font, "GAME OVER", redColor, config.width/2 - 80, config.height/2);
                try++;
                tryString[3] = (char)(try%10)+48; // calcul pour les tentative
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

            SDL_Texture* currentTexture = playerTextures.idle;
            if (keys[SDL_GetScancodeFromKey(config.moveLeftKey)] || keys[SDL_GetScancodeFromKey(config.moveRightKey)]) {
                if (SDL_GetTicks() - frameStart >= frameDelay) {
                    frame = (frame + 1) % 8;
                    frameStart = SDL_GetTicks();
                }
                currentTexture = playerTextures.run[frame];
            } else if (keys[SDL_GetScancodeFromKey(config.jumpKey)] && canJump > 0) {
                currentTexture = playerTextures.jump;
            }

            SDL_Rect playerRect = {
                playerX,
                playerY,
                TILE_SIZE,
                TILE_SIZE
            };

            if (facingRight) {
                SDL_RenderCopyEx(renderer, currentTexture, NULL, &playerRect, 0, NULL, SDL_FLIP_NONE);
            } else {
                SDL_RenderCopyEx(renderer, currentTexture, NULL, &playerRect, 0, NULL, SDL_FLIP_HORIZONTAL);
            }

            // Dessiner le bouton de pause
            SDL_RenderCopy(renderer, pauseButtonTexture, NULL, &pauseButtonRect);

            // Dessiner le texte en dernier pour qu'il soit au premier plan
            int textWidth, textHeight;
            SDL_Color Color = {200, 200, 200, 255};
            SDL_QueryTexture(levelTextTexture, NULL, NULL, &textWidth, &textHeight);
            SDL_Rect textRect = {
                (config.width - textWidth) / 2,
                20,
                textWidth,
                textHeight
            };
            SDL_RenderCopy(renderer, levelTextTexture, NULL, &textRect);

            renderText(renderer, font, tryString, Color, config.width/3 - 400, config.height/42);
            SDL_RenderPresent(renderer);

            // Debug
            if (levelTextTexture == NULL) {
                printf("La texture du texte est NULL\n");
            }

            SDL_RenderPresent(renderer);

            SDL_Delay(16);
        } else if (gameState == PAUSED) {
            // Render the pause menu
            SDL_SetRenderDrawColor(renderer, 0, 0, 0, 120);
            SDL_RenderClear(renderer);

            // Render the play button
            SDL_RenderCopy(renderer, playButtonTexture, NULL, &playButtonRect);

            // Render the back to menu button
            SDL_RenderCopy(renderer, backToMenuButtonTexture, NULL, &backToMenuButtonRect);

            SDL_RenderPresent(renderer);
        } else if (gameState == CREATE_LEVEL) {
            renderCreateLevel(renderer, customLevel, saveText, &saveRect, cursorX, cursorY, backToMenuText, &backToMenuRect, font, &config);
            if (enteringFilename) {
                int textWidth, textHeight;
                SDL_QueryTexture(saveText, NULL, NULL, &textWidth, &textHeight);
                int centerX = (config.width - textWidth) / 2;
                int centerY = (config.height - textHeight) / 2;
                renderTextWithBackground(renderer, font, "Enter filename: ", blackColor, highlightColor, centerX - 100, centerY);
                renderTextWithBackground(renderer, font, filename, blackColor, highlightColor, centerX + 100, centerY);
                SDL_RenderPresent(renderer);
            }
        }
    }

    SDL_DestroyTexture(menuTextures.titleText);
    SDL_DestroyTexture(menuTextures.levelsText);
    SDL_DestroyTexture(menuTextures.settingsText);
    SDL_DestroyTexture(menuTextures.exitText);
    SDL_DestroyTexture(textures.background1);
    SDL_DestroyTexture(textures.background2);
    SDL_DestroyTexture(textures.background3);
    SDL_DestroyTexture(textures.ground);
    SDL_DestroyTexture(textures.rock);
    SDL_DestroyTexture(textures.lamp);
    SDL_DestroyTexture(textures.grass);
    SDL_DestroyTexture(textures.levelText);
    SDL_DestroyTexture(levelTextTexture); // Détruire la texture du texte du niveau
    SDL_DestroyTexture(pauseButtonTexture); // Détruire la texture du bouton de pause
    SDL_DestroyTexture(playButtonTexture); // Détruire la texture du bouton de reprise
    SDL_DestroyTexture(backToMenuButtonTexture); // Détruire la texture du bouton de retour au menu
    for (int i = 0; i < levelsMenuTextures.levelCount; i++) {
        SDL_DestroyTexture(levelsMenuTextures.levelTexts[i]);
    }
    SDL_DestroyTexture(levelsMenuTextures.createLevelText); // Destroy create level text texture
    SDL_DestroyTexture(levelsMenuTextures.backText);
    for (int i = 0; i < 8; i++) {
        SDL_DestroyTexture(playerTextures.run[i]);
    }
    SDL_DestroyTexture(saveText); // Destroy save button texture
    SDL_DestroyTexture(backToMenuText); // Destroy back to menu button texture
    SDL_DestroyTexture(playerTextures.jump);
    SDL_DestroyTexture(playerTextures.idle);
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();

    TTF_CloseFont(font);
    TTF_Quit();

    return 0;
}