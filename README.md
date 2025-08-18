# MyMayaPlugin — Improved Delta Mush Maya Deformer

This is a Maya **deformer plugin** implementing the method from my paper:
[Improving Delta Mush Based Character Animation (CESCG 2020)](https://cescg.org/cescg_submission/improving-delta-mush-based-character-animation/)

## Features
-Laplacian Smoothing
-Rest-Delta Computation
-Self-Intersection Handling using CCD-based collision detection


## Build
Requirements:
- Maya DevKit (2022–2025)
- CMake ≥ 3.20
- C++17 compiler

```bash
mkdir build && cd build
cmake -DMAYA_DEVKIT_ROOT="$MAYA_DEVKIT_ROOT" -DMAYA_VERSION=2025 ..
cmake --build . --config Release
