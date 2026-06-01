Lumina
----------------------------------------------------------------------
![image](https://github.com/user-attachments/assets/e7d0a8bb-c0d8-4a39-befe-86844e3159fc)
![image](https://github.com/user-attachments/assets/356fafe5-eec6-4aec-a24f-cb348edec001)

Creation level
![image](https://github.com/user-attachments/assets/aece1420-cf92-43f9-b373-6bd37ad8a78c)


# Lumina

Lumina is a game developed in C using the SDL2 library.

## Prerequisites

- [SDL2](https://www.libsdl.org/)
- [SDL2_image](https://www.libsdl.org/projects/SDL_image/)
- [SDL2_ttf](https://www.libsdl.org/projects/SDL_ttf/)
- GCC
- Make

On macOS (Homebrew):
```bash
brew install sdl2 sdl2_image sdl2_ttf
```

## Installation

```bash
git clone https://github.com/lennyblk/Lumina.git
cd Lumina
```

## Compilation

### macOS / Linux

```bash
make
```

### Windows

Make sure `SDL2.dll`, `SDL2_image.dll`, and `SDL2_ttf.dll` are in the same folder as the executable. Compile using MinGW or a compatible IDE.

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
