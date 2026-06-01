#ifndef COLLISION_H
#define COLLISION_H

#include <SDL.h>

#ifndef TILE_SIZE
#define TILE_SIZE 32
#endif
#ifndef LEVEL_WIDTH
#define LEVEL_WIDTH 40
#endif
#ifndef LEVEL_HEIGHT
#define LEVEL_HEIGHT 25
#endif

int checkCollision(int x, int y, int level[LEVEL_HEIGHT][LEVEL_WIDTH], int tileType);
int checkAboveCollision(int x, int y, int level[LEVEL_HEIGHT][LEVEL_WIDTH], int tileType);

#endif // COLLISION_H
