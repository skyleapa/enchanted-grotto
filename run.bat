echo "Building rlottie..."
cd ext\RmlUi\Dependencies\rlottie
cmake -B build -S . -DBUILD_SHARED_LIBS=OFF
cmake --build build --target rlottie --config Debug
cmake --build build --target rlottie --config Release
cd ..
cd ..

echo "Building RmlUi..."
cmake -B Build -S . -DBUILD_SHARED_LIBS=OFF -DRMLUI_LOTTIE_PLUGIN=ON
cmake --build Build -j
cd ..
cd ..

if "%~1" == "test" goto test
goto game

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