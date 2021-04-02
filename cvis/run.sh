#!/bin/sh
cmake -S . -B build &&
cmake --build build &&
# (cd build; ./cvis tetrik ../media/tetrik.mp4)
(cd build; ./cvis nwwia ../media/nwwia.mp4)
