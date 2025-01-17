#!/bin/sh

ENGINE="game.cpp keyboard.cpp main.cpp sprite.cpp map.cpp"
FILES="init.cpp mouse.cpp"

c++ --std=c++20 -o editor $ENGINE map_editor.cpp $(pkg-config --cflags --libs sdl3)
c++ --std=c++20 -o main $ENGINE $FILES $(pkg-config --cflags --libs sdl3)
