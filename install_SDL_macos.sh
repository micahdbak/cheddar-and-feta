#!/bin/sh

SDL=$(pwd)/SDL

mkdir -p $SDL/build
cd $SDL/build
cmake -DCMAKE_BUILD_TYPE=Release -DCMAKE_OSX_ARCHITECTURES="x86_64;arm64" ..
cmake --build . --config Release --parallel
sudo cmake --install . --config Release

echo "---- pkg-config:"
echo $(pkg-config --cflags --libs sdl3)
