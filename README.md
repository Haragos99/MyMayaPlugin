# MyMayaPlugin - Improved Delta Mush Maya Deformer

This repository contains a Maya **plugin** that implements the method from my paper on handling self-intersections in character animation.  

Paper: [Improving Delta Mush Based Character Animation (CESCG 2025)](https://cescg.org/cescg_submission/improving-delta-mush-based-character-animation/)  

Original repository: [Haragos99/sample-framework](https://github.com/Haragos99/sample-framework)


## Features
- Laplacian Smoothing
- Rest-Delta Computation
- Self-Intersection Handling using CCD-based collision detection

## Results
On each figure, the result of the default Delta Mush deformation is shown on the left, while the result using the proposed CCD-based scaling of delta vectors is shown on the right (This result is from the paper).

<img width="560" height="207" alt="image" src="https://github.com/user-attachments/assets/dc59208b-07e3-448e-aebb-10d56f36d3ff" />
<img width="560" height="207" alt="image" src="https://github.com/user-attachments/assets/1768675f-4729-45ba-aa01-58a80c1fe340" />
<img width="560" height="207" alt="image" src="https://github.com/user-attachments/assets/8357a237-c136-400b-bfb6-5a3c7b6c21bb" />




## Build
Requirements:
- Maya DevKit (2022–2025)
- CMake ≥ 3.16
- C++17 compiler

```bash
mkdir build && cd build
cmake --build 
