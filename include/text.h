#ifndef TEXT_H
#define TEXT_H

#include <SDL.h>
#include <SDL_ttf.h>

void renderText(SDL_Renderer* renderer, TTF_Font* font, const char* text, SDL_Color color, int x, int y);
void renderTextWithBackground(SDL_Renderer* renderer, TTF_Font* font, const char* text, SDL_Color textColor, SDL_Color bgColor, int x, int y);
SDL_Texture* loadTexture(const char* filePath, SDL_Renderer* renderer);
SDL_Texture* createTextTexture(TTF_Font* font, const char* text, SDL_Color color, SDL_Renderer* renderer);
#endif