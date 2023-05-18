#!/bin/bash
current_directory=$(pwd)
echo $current_directory
python assets/shaders/compile.py $current_directory "assets/shaders"