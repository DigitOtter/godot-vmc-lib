#!/bin/bash

INSTALL_BASE_DIR="bin"
CMAKE_ARGS="-DCMAKE_BUILD_TYPE=Release"

mkdir -p "build"
touch "build/.gdignore"

BUILD_DIR="build/lin_64"
mkdir -p $BUILD_DIR
cmake -S . -B $BUILD_DIR $CMAKE_ARGS \
	-DVMC_LIB_INSTALL_DIR="$PWD/$INSTALL_BASE_DIR/lin_64" \
        -GNinja

cmake --build $BUILD_DIR
cmake --install $BUILD_DIR

#BUILD_DIR="build/lin_32"
#mkdir -p $BUILD_DIR
#cmake -S . -B $BUILD_DIR $CMAKE_ARGS \
#	-DCMAKE_TOOLCHAIN_FILE="CI/linux_i686.toolchain.cmake" \
#	-GNinja
#
#cmake --build $BUILD_DIR
#cmake --install $BUILD_DIR

BUILD_DIR="build/win_64"
mkdir -p $BUILD_DIR
x86_64-w64-mingw32-cmake -S . -B $BUILD_DIR $CMAKE_ARGS \
	-DVMC_LIB_INSTALL_DIR="$PWD/$INSTALL_BASE_DIR/win_64"

cmake --build $BUILD_DIR
cmake --install $BUILD_DIR

BUILD_DIR="build/win_32"
mkdir -p $BUILD_DIR
i686-w64-mingw32-cmake -S . -B $BUILD_DIR $CMAKE_ARGS \
	-DVMC_LIB_INSTALL_DIR="$PWD/$INSTALL_BASE_DIR/win_32"

cmake --build $BUILD_DIR
cmake --install $BUILD_DIR

