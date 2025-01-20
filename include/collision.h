#include <SDL.h>

#define TILE_SIZE 16
#define LEVEL_WIDTH 80   
#define LEVEL_HEIGHT 50  

int checkCollision(int x, int y, int level[LEVEL_HEIGHT][LEVEL_WIDTH], int tileType);

int checkCollision4P(int x, int y, int level[LEVEL_HEIGHT][LEVEL_WIDTH], int tileType);
int checkAboveCollision(int x, int y, int level[LEVEL_HEIGHT][LEVEL_WIDTH], int tileType);
