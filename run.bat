if "%~1" == "" goto game
if "%~1" == "game" goto game
if "%~1" == "test" goto test

echo "Unrecognized argument, exiting..."
goto eof

:test
cmake -S . -B build -DBUILD_GAME=OFF -DBUILD_TESTING=ON
cmake --build build
cd build && ctest --output-on-failure
goto eof

:game
cmake -S . -B build -DBUILD_GAME=ON -DBUILD_TESTING=OFF
cmake --build build
cd build && .\enchanted_grotto.exe
goto eof

:eof
::pause