#include <stdio.h>
#include <stdlib.h>
#include <SDL.h>
#include "cJSON.h"
#include <SDL_image.h>
#include <SDL_ttf.h>
#include <string.h>
#include <ctype.h> 
#include <dirent.h> 
#include "include/config.h"
#include "include/game.h"
#include "include/menu.h"
#include "src/config.c"
#include "src/game.c" 
#include "src/level.c"
#include "src/collision.c"
#include "src/text.c"
#include "include/text.h"
#include "src/menu.c"
#include "include/menu.h"
#include "src/createlevel.c"

#define TILE_SIZE 16
#define LEVEL_WIDTH 80   
#define LEVEL_HEIGHT 50  
#define MAX_LEVELS 100

typedef struct {
    int x;
    int y;
    int hasCheckpoint;
    int checkpointX;
    int checkpointY;
    int isCheckpointActive;
} PlayerSpawn;

typedef enum {      //etats du jeu
    MENU,
    LEVELS,
    SETTINGS,
    PLAYING,
    PAUSED, 
    CREATE_LEVEL, 
    EXIT
} GameState;


int isMouseOverButton(int mouseX, int mouseY, SDL_Rect buttonRect) {
    return mouseX >= buttonRect.x && mouseX <= buttonRect.x + buttonRect.w &&
           mouseY >= buttonRect.y && mouseY <= buttonRect.y + buttonRect.h;
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

    TTF_Font* font = TTF_OpenFont("oak_woods_v1.0/fonts/KnightWarrior-w16n8.otf", 32);
    if (!font) {
        printf("Erreur lors du chargement de la police : %s\n", TTF_GetError());
        return 1;
    }

    GameTextures textures;
    
    SDL_Color textColor = {255, 255, 255, 255};
    SDL_Color blackColor = {0, 0, 0, 255}; 
    SDL_Color highlightColor = {255, 255, 0, 255}; 
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
    menuTextures.titleText = createTextTexture(font, "LUMINA", textColor, renderer);
    menuTextures.levelsText = createTextTexture(font, "Levels", textColor, renderer);
    menuTextures.settingsText = createTextTexture(font, "Settings", textColor, renderer);
    menuTextures.exitText = createTextTexture(font, "Exit", textColor, renderer);
    menuTextures.menuWallpaper = loadTexture("oak_woods_v1.0/wallapaper2.jpeg", renderer);
    menuTextures.pauseWallpaper = loadTexture("oak_woods_v1.0/wallapaper1.jpeg", renderer);

    if (!menuTextures.titleText || !menuTextures.levelsText || !menuTextures.settingsText || !menuTextures.exitText) {
        printf("Erreur lors de la création des textures de texte du menu\n");
        return 1;
    }

    //Verification du chargement des backgrounds de menu

    if (!menuTextures.menuWallpaper || !menuTextures.pauseWallpaper) {
    printf("Erreur lors du chargement des wallpapers : %s\n", SDL_GetError());
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();
    return 1;
}

    LevelsMenuTextures levelsMenuTextures;
    levelsMenuTextures.createLevelText = createTextTexture(font, "cree ton niveau", textColor, renderer); //ajout du texte de création de niveau
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

    char levelNames[MAX_LEVELS][256]; 
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
    SDL_Rect saveRect; // Supprimer la redéclaration de createLevelRect
    SDL_Texture* saveText = createTextTexture(font, "Save", blackColor, renderer); // Changer la couleur en noir
    int customLevel[LEVEL_HEIGHT][LEVEL_WIDTH] = {0}; // Initialiser une matrice de niveau personnalisé

    SDL_Rect pauseButtonRect = {config.width - 50, 10, 40, 40}; // Position et taille du bouton pause
    SDL_Rect playButtonRect = {config.width / 2 - 50, config.height / 2 - 20, 100, 40}; // Position et taille du bouton play
    SDL_Rect backToMenuButtonRect = {config.width / 2 - 50, config.height / 2 + 30, 100, 40}; // Position et taille du bouton retour au menu

    SDL_Texture* pauseButtonTexture = createTextTexture(font, "Pause", textColor, renderer);
    SDL_Texture* playButtonTexture = createTextTexture(font, "Play", textColor, renderer);
    SDL_Texture* backToMenuButtonTexture = createTextTexture(font, "Back to Menu", textColor, renderer);

    char levelText[256] = "First Level"; // Variable pour stocker le texte du niveau
    SDL_Texture* levelTextTexture = NULL; // Texture pour le texte du niveau

    int cursorX = 0, cursorY = LEVEL_HEIGHT - 1; // Position du curseur pour l'entrée clavier, commencer en bas à gauche

    SDL_Rect backToMenuRect = {config.width - 150, 10, 140, 40}; // Position et taille du bouton retour au menu
    SDL_Texture* backToMenuText = createTextTexture(font, "Back to Menu", blackColor, renderer); // Changer la couleur en noir

    char filename[256] = "";
    int enteringFilename = 0;

    int frame = 0;
    int frameDelay = 100; // Délai entre les images en millisecondes
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
                
                } else if (gameState == CREATE_LEVEL) {
                    if (enteringFilename) {
                        if (event.key.keysym.sym == SDLK_RETURN) {
                            if (isValidFilename(filename)) {
                                saveLevelWithFilename(filename, customLevel);
                                loadLevelsFromDirectory("levels", &levelsMenuTextures, font, renderer, levelNames); // levels menu
                                gameState = LEVELS; // Retourner au menu des niveaux
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
                            customLevel[cursorY][cursorX] = (customLevel[cursorY][cursorX] == 6) ? 0 : 6; //  checkpoint
                        } else if (event.key.keysym.sym == SDLK_7) {
                            customLevel[cursorY][cursorX] = (customLevel[cursorY][cursorX] == 7) ? 0 : 7; //  solid block
                        } else if (event.key.keysym.sym == SDLK_8) {
                            customLevel[cursorY][cursorX] = (customLevel[cursorY][cursorX] == 8) ? 0 : 8; //  spike
                        } else if (event.key.keysym.sym == SDLK_9) {
                            customLevel[cursorY][cursorX] = (customLevel[cursorY][cursorX] == 9) ? 0 : 9; //  finish line
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
                            spawn.hasCheckpoint = 0;
                            spawn.isCheckpointActive = 0;
                            break;
                        }
                    }
                    if (isMouseOverButton(mouseX, mouseY, createLevelRect)) {
                        gameState = CREATE_LEVEL; 
                    } else if (isMouseOverButton(mouseX, mouseY, backRect)) {
                        gameState = MENU;
                    }
                } else if (gameState == SETTINGS) {
                    if (isMouseOverButton(mouseX, mouseY, backRect)) {
                        gameState = MENU;
                    }
                } else if (gameState == PLAYING) {
                    if (isMouseOverButton(mouseX, mouseY, pauseButtonRect)) {
                        gameState = PAUSED; 
                    }
                } else if (gameState == PAUSED) {
                    if (isMouseOverButton(mouseX, mouseY, playButtonRect)) {
                        gameState = PLAYING; 
                    } else if (isMouseOverButton(mouseX, mouseY, backToMenuButtonRect)) {
                        spawn.hasCheckpoint = 0;
                        spawn.isCheckpointActive = 0;
                        
                        playerX = spawn.x;
                        playerY = spawn.y;
                        velocityY = 0;
                        canJump = 2;
                        gameState = MENU; 
                    }
                } else if (gameState == CREATE_LEVEL) {
                    int tileX = mouseX / TILE_SIZE;
                    int tileY = mouseY / TILE_SIZE;
                    if (tileX >= 0 && tileX < LEVEL_WIDTH && tileY >= 0 && tileY < LEVEL_HEIGHT) {
                        customLevel[tileY][tileX] = (customLevel[tileY][tileX] + 1) % 10; // cycle a travers les types de tuiles
                    }
                    if (isMouseOverButton(mouseX, mouseY, saveRect)) {
                        enteringFilename = 1; // entrain de saisir le nom du fichier
                    } else if (isMouseOverButton(mouseX, mouseY, backToMenuRect)) {
                        gameState = MENU; // retourner au menu
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
                SDL_Delay(2000);
                playerX = spawn.x;
                playerY = spawn.y;  
                gameState = MENU;
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

             // Nettoyage du renderer
            SDL_RenderClear(renderer);
            
            // Dessiner le wallpaper de pause en plein écran
            SDL_Rect fullscreen = {0, 0, config.width, config.height};
            SDL_RenderCopy(renderer, menuTextures.pauseWallpaper, NULL, &fullscreen);
            
            // Dessiner les boutons
            SDL_RenderCopy(renderer, playButtonTexture, NULL, &playButtonRect);
            SDL_RenderCopy(renderer, backToMenuButtonTexture, NULL, &backToMenuButtonRect);
            
            // Afficher le tout
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

                                                                                // Libération de la mémoire
    SDL_DestroyTexture(menuTextures.titleText);
    SDL_DestroyTexture(menuTextures.levelsText);
    SDL_DestroyTexture(menuTextures.settingsText);
    SDL_DestroyTexture(menuTextures.exitText);
    SDL_DestroyTexture(textures.background1);
    SDL_DestroyTexture(textures.background2);
    SDL_DestroyTexture(textures.background3);
    SDL_DestroyTexture(menuTextures.menuWallpaper);
    SDL_DestroyTexture(menuTextures.pauseWallpaper);
    SDL_DestroyTexture(textures.ground);
    SDL_DestroyTexture(textures.rock);
    SDL_DestroyTexture(textures.lamp);
    SDL_DestroyTexture(textures.grass);
    SDL_DestroyTexture(textures.levelText);
    SDL_DestroyTexture(levelTextTexture); 
    SDL_DestroyTexture(pauseButtonTexture); 
    SDL_DestroyTexture(playButtonTexture); 
    SDL_DestroyTexture(backToMenuButtonTexture); 
    for (int i = 0; i < levelsMenuTextures.levelCount; i++) {
        SDL_DestroyTexture(levelsMenuTextures.levelTexts[i]);
    }
    SDL_DestroyTexture(levelsMenuTextures.createLevelText);
    SDL_DestroyTexture(levelsMenuTextures.backText);
    for (int i = 0; i < 8; i++) {
        SDL_DestroyTexture(playerTextures.run[i]);
    }
    SDL_DestroyTexture(saveText);
    SDL_DestroyTexture(backToMenuText); 
    SDL_DestroyTexture(playerTextures.jump);
    SDL_DestroyTexture(playerTextures.idle);
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();

    TTF_CloseFont(font);
    TTF_Quit();

    return 0;
}