Lumina
----------------------------------------------------------------------
![image](https://github.com/user-attachments/assets/e7d0a8bb-c0d8-4a39-befe-86844e3159fc)
![image](https://github.com/user-attachments/assets/356fafe5-eec6-4aec-a24f-cb348edec001)

Creation level
![image](https://github.com/user-attachments/assets/aece1420-cf92-43f9-b373-6bd37ad8a78c)


# Lumina

Lumina is a game developed in C using the SDL2 library.

## Prerequisites

Before running the game, make sure you have the following installed:
- [SDL2](https://www.libsdl.org/)
- [SDL2_image](https://www.libsdl.org/projects/SDL_image/)
- [SDL2_ttf](https://www.libsdl.org/projects/SDL_ttf/)
- A C compiler (e.g., gcc)
- (Optional) `make` if you want to use a Makefile

## Installation

1. Clone this repository:
   ```bash
   git clone https://github.com/lennyblk/Lumina.git
   cd Lumina
   ```

2. Check that the following files and folders are present:
   - `main.c`
   - `src/` folder
   - `include/` folder
   - `levels/` folder
   - `player_assets/` folder
   - SDL2 `.dll` files (on Windows)

3. Install SDL2 dependencies according to your operating system.

## Compilation

### On Linux/MacOS

```bash
gcc -o lumina main.c src/*.c -Iinclude -lSDL2 -lSDL2_image -lSDL2_ttf
```

Or, if a Makefile is provided:
```bash
make
```

### On Windows

Make sure that `SDL2.dll`, `SDL2_image.dll`, and `SDL2_ttf.dll` are in the same folder as the executable. Compile using MinGW or a compatible IDE.

## Running the Game

After compiling, run the game with:

```bash
./lumina
```
or on Windows:
```bash
lumina.exe
```

## Project Structure

- `src/`: game source files
- `include/`: header files
- `levels/`: game levels
- `player_assets/`: player graphic assets
- `lib/`: other resources and libraries
- `config.json`: game configuration

## Authors

- [lennyblk](https://github.com/lennyblk)
- [sarahgarciap](https://github.com/sarahgarciap)

---

Feel free to open an issue if you have any questions or encounter a bug!
