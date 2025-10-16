# Gaussian Splat Viewer

A 3D Gaussian Splatting visualization tool based on OpenGL, designed for rendering and interactively viewing Gaussian point cloud data in PLY format.

## System Requirements

- CMake 3.16+
- C++17 compiler
- OpenGL 4.2+
- Linux

### Dependencies

- **GLFW3** - Window management and input handling
- **GLM** - Math library
- **OpenGL** - Graphics rendering
- **GLAD** - OpenGL loader (included)
- **TinyPLY** - PLY file parser (included)

## Build Instructions

### Install Dependencies

On Ubuntu/Debian:

```bash
sudo apt-get update
sudo apt-get install libglfw3-dev libglm-dev libgl1-mesa-dev cmake build-essential
```

### Build the Project

```bash
# Create build directory
mkdir build
cd build

# Configure and build
cmake ..
cmake --build .
```

After successful compilation, the executable `gsplat_viewer` will be generated in the `build` directory.

## Usage

```bash
./gsplat_viewer <ply_file>
```

### Controls

| Action                | Description         |
|-----------------------|--------------------|
| **Left Mouse Drag**   | Rotate camera      |
| **Middle/Right Drag** | Pan camera         |
| **Mouse Wheel**       | Zoom view          |
| **ESC**               | Exit program       |
