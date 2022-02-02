#!/usr/bin/env bash
mkdir build
cd build
cmake ..
intercept-build make
mv compile_commands.json ../compile_commands.json
