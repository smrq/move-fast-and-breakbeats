#!/bin/sh
cmake -S . -B build &&
cmake --build build &&
(cd build; ./cvis --endframe ../media/endframe.png 452.684 --silence 0.75 ../media/tetrik.flac ../media/tetrik.mp4)
