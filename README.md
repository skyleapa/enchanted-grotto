# Enchanted Grotto

## Build & Run Instructions
Both systems take in a `game|test` argument:
- `game` builds and runs the full game
- `test` builds and runs tests for item serialization and potion comparison mechanics
- If no arguments are supplied, the scripts default to `game`

### Windows:
```bash
.\run [game|test]
```
Note that on some Windows configurations, CMake may place the game executable in the `/build/Debug` folder. If the script fails, the executable is most likely there and can be run directly.

### Linux/macOS:
Requires `freetype` to be installed. On macOS, use `brew install freetype`.
```bash
chmod +x run.sh
./run.sh [game|test]
```
progress 1 video: https://www.youtube.com/watch?v=Tv99njp9Pmo
progress 2 video: https://youtu.be/Qz-uyPb_oww
progress 3 video: https://www.youtube.com/watch?v=bLyz9KP83AY
