# MyMayaPlugin

MyMayaPlugin is a C++ plugin for Autodesk Maya. It adds custom functionality to Maya using the Maya API.

## ✅ Features

- Written in modern C++
- Exposes custom commands to Maya
- Can be loaded and unloaded from Python or MEL

---

## 🧰 Requirements

- Autodesk Maya 
- Visual Studio 2019 or newer
- Maya Developer Kit (from Maya installation)
- Set environment variables for include and lib directories

---

## 🔧 Building the Plugin

1. Clone or copy this project.
2. Open the `.sln` file in Visual Studio.
3. Set up the include/lib directories:
Include: C:\Program Files\Autodesk\Maya2024\include
Library: C:\Program Files\Autodesk\Maya2024\lib
4. Build the project in `Release` or `Debug` mode.

If successful, the `.mll` file will be created in:
<ProjectDir>\x64\Release\MyMayaPlugin.mll

