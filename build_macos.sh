#!/bin/sh

c++ --std=c++20 -o main main.cpp game.cpp keyboard.cpp sprite.cpp $(pkg-config --cflags --libs sdl3)
