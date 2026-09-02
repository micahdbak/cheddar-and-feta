#!/bin/bash

for dir in ./build/bin ./build-rel/bin; do
    mkdir -p "$dir"
    cp -Rf ./assets/* "$dir/"
done
