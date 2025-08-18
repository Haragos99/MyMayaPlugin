# MyMayaPlugin — Improved Delta Mush Maya Deformer

This repository contains a Maya **plugin** that implements the method from my paper on handling self-intersections in character animation.  

Paper: [Improving Delta Mush Based Character Animation (CESCG 2025)](https://cescg.org/cescg_submission/improving-delta-mush-based-character-animation/)  

Original repository: [Haragos99/sample-framework](https://github.com/Haragos99/sample-framework)


## Features
- Laplacian Smoothing
- Rest-Delta Computation
- Self-Intersection Handling using CCD-based collision detection

## Results
<img width="560" height="207" alt="image" src="https://github.com/user-attachments/assets/dc59208b-07e3-448e-aebb-10d56f36d3ff" />

## Build
Requirements:
- Maya DevKit (2022–2025)
- CMake ≥ 3.20
- C++17 compiler

```bash
mkdir build && cd build
cmake --build 
