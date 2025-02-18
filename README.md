## Enchanted Grotto

### Build & Run Instructions

#### Running the Game

Linux/macOS:
Option 1: Bash Script
```bash
chmod +x run_game.sh
./run_game.sh
```

Option 2: Using Command Line
```bash
cmake -S . -B build -DBUILD_GAME=ON -DBUILD_TESTING=OFF
cmake --build build
./build/enchanted_grotto
```

Windows (UNVERIFIED):

Option 1: Using Visual Studio?

Option 2: Using Command Line (Ninja)
```bash
cmake -S . -B build -DBUILD_GAME=ON -DBUILD_TESTING=OFF -GNinja
cmake --build build
build\enchanted_grotto.exe
```

#### Running Tests

Linux/macOS:

Option 1: Bash Script
```bash
chmod +x run_tests.sh
./run_tests.sh
```

Option 2: Using Command Line
```bash
cmake -S . -B build -DBUILD_GAME=OFF -DBUILD_TESTING=ON
cmake --build build
cd build && ctest --output-on-failure
```

Windows (UNVERIFIED):

Option 1: Using Visual Studio?

Option 2: Using Command Line (Ninj)
```bash
cmake -S . -B build -DBUILD_GAME=OFF -DBUILD_TESTING=ON -GNinja
cmake --build build
cd build && ctest --output-on-failure
```