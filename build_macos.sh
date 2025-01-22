#!/bin/sh

ENGINE="font.cpp game.cpp keyboard.cpp main.cpp map.cpp sprite.cpp"
FILES="src/*.cpp"

c++ --std=c++20 -o editor $ENGINE map_editor.cpp -I. $(pkg-config --cflags --libs sdl3)
c++ --std=c++20 -o main $ENGINE $FILES -I. $(pkg-config --cflags --libs sdl3)
