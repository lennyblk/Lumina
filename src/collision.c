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

int checkAboveCollision(int x, int y, int level[LEVEL_HEIGHT][LEVEL_WIDTH], int tileType) {
    int tileX = x / TILE_SIZE;
    int tileY = (y - TILE_SIZE) / TILE_SIZE;

    if (tileX >= 0 && tileX < LEVEL_WIDTH && tileY >= 0 && tileY < LEVEL_HEIGHT) {
        return level[tileY][tileX] == tileType;
    }
    return 0;
}