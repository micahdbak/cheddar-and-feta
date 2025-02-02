#!/bin/sh

ENGINE="engine/*.cpp"
FILES="src/*.cpp"

c++ --std=c++20 -o editor $ENGINE map_editor.cpp -Iengine $(pkg-config --cflags --libs sdl3) -lm
c++ --std=c++20 -o main $ENGINE $FILES -Iengine $(pkg-config --cflags --libs sdl3) -lm
