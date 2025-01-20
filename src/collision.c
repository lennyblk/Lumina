#include "../include/collision.h"
#include "../cJSON.h"
#include <stdio.h>

int checkCollision(int x, int y, int level[LEVEL_HEIGHT][LEVEL_WIDTH], int tileType) {
    int tileX = x / TILE_SIZE;
    int tileY = y / TILE_SIZE;

    if (tileX >= 0 && tileX < LEVEL_WIDTH && tileY >= 0 && tileY < LEVEL_HEIGHT) {
        return level[tileY][tileX] == tileType;
    }
    return 0;
}

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