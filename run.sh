cd build
cmake --build .
sudo ./rgb-matrix-4096 --led-rows=64 --led-cols=64 --led-slowdown-gpio=2
cd ..