#include <SDL.h>
#include <stdio.h>

int main(int argc, char* argv[]) {
    if (SDL_Init(SDL_INIT_VIDEO) < 0) {
        printf("Erreur lors de l'initialisation de SDL : %s\n", SDL_GetError());
        return 1;
    }

    printf("SDL2 initialisé correctement !\n");

    SDL_Window* window = SDL_CreateWindow(
        "Ma première fenêtre SDL",
        SDL_WINDOWPOS_CENTERED,
        SDL_WINDOWPOS_CENTERED,
        800, 600, SDL_WINDOW_SHOWN
    );

    if (!window) {
        printf("Erreur lors de la création de la fenêtre : %s\n", SDL_GetError());
        SDL_Quit();
        return 1;
    }

    printf("Fenêtre créée avec succès !\n");

    SDL_Event event;
    int running = 1;
    while (running) {
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT || (event.type == SDL_KEYDOWN && event.key.keysym.sym == SDLK_ESCAPE)) {
                running = 0;
            }
        }
    }

    SDL_DestroyWindow(window);
    SDL_Quit();

    return 0;
}
