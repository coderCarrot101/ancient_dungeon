#!/bin/bash

    g++ -o $1 $2 -I/usr/include/SDL2 -I/usr/include/SDL2/SDL2_ttf-2.20.2 -L/usr/lib/x86_64-linux-gnu -lSDL2 -lSDL2_ttf -lm