# GodotVMCLib

Implements a VMC receiver in Godot

## Build

First initialize submodules, specifically the 4.1 branch of `godot-cpp`:
```bash
git submodule update --init --recursive
cd third_party/godot-cpp
git pull https://github.com/godotengine/godot-cpp 4.1
cd ../..
```
Then build GodotVMCLib using CMake:
```bash
mkdir build
cd build
cmake ..
cmake --build .
cmake --install .
```

## Using GodotVMCLib in a project

To use GodotVMCLib in a Godot project, add the repository under `addons/godot-vmc-lib` in the project folder and build it. An example project can be found under: https://github.com/DigitOtter/godot-vmc

