#!/bin/bash

SCRIPT_DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" &> /dev/null && pwd )"

# DOWNLOAD GLFW
git clone -b 3.3-stable https://github.com/glfw/glfw.git "$SCRIPT_DIR/../vendor/glfw"

# DOWNLOAD GLAD
git clone -b 4.6 https://github.com/al7aro/glad.git "$SCRIPT_DIR/../vendor/glad"

# DOWNLOAD GLM
git clone https://github.com/g-truc/glm.git "$SCRIPT_DIR/../vendor/glm"

# DOWNLOAD STB
git clone https://github.com/al7aro/stb.git "$SCRIPT_DIR/../vendor/stb"

mkdir -p "${SCRIPT_DIR}/../build"

cmake -S "${SCRIPT_DIR}/.." -B "${SCRIPT_DIR}/../build"

make -C "${SCRIPT_DIR}/../build"