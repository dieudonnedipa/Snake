# Snake

A classic Snake game for Windows, available in two versions:

- **Console version**: `snake.c`, built with the Windows console API
- **Graphical version**: `snake_gfx.c`, built with raylib

Both versions support growing gameplay, increasing speed, collision detection, and persistent high scores.

## Included files

- `snake.c` — console implementation
- `snake.exe` — prebuilt console executable
- `snake_gfx.c` — raylib graphical implementation
- `snake_gfx.exe` — prebuilt graphical executable
- `highscore.txt` — saved high score

## Run

### Graphical version

```powershell
.\snake_gfx.exe
```

### Console version

```powershell
.\snake.exe
```

Keep `highscore.txt` in the same directory as the executable so that the high score can be loaded and saved.

## Controls

### Graphical version

- **Arrow keys** or **W/A/S/D**: Move
- **P** or **Space**: Pause/resume
- **Enter** or **Space**: Start
- **Enter** or **R**: Restart after game over or winning
- **Close window**: Quit

### Console version

- **Arrow keys** or **W/A/S/D**: Move
- **P**: Pause
- **Q** or **Escape**: Quit
- **R**: Restart after game over or winning

## Rules

- Eat the food to earn 10 points and make the snake longer.
- The game speed increases as the score rises.
- Hitting a wall or the snake's own body ends the game.
- Filling the board wins the game.

## Build

### Console version

With GCC/MinGW for Windows:

```powershell
gcc -Wall -Wextra snake.c -o snake.exe
```

The console version uses `conio.h` and `windows.h`, so it is intended to run on Windows.

### Graphical version

The graphical version requires raylib. Using MSYS2 UCRT64:

```powershell
gcc -Wall -Wextra snake_gfx.c -o snake_gfx.exe -lraylib -lopengl32 -lgdi32 -lwinmm
```

Run it with:

```powershell
.\snake_gfx.exe
```
