# ParticleLife

A GPU-accelerated particle simulation where colored species interact through attraction and repulsion to create emergent life-like patterns.

## Description

ParticleLife simulates thousands of particles that belong to different species (represented by colors). Each species has unique attraction/repulsion relationships with other species, creating complex emergent behaviors similar to biological systems. The simulation runs entirely on the GPU using OpenGL compute shaders for real-time performance.

## Features

- **GPU Acceleration**: Compute shaders handle particle physics for 10,000+ particles at 60+ FPS
- **8 Particle Species**: Each color represents a different species with unique interaction rules
- **Emergent Behaviors**: Complex patterns emerge from simple attraction/repulsion rules
- **Real-time Visualization**: Multi-pass rendering with glow effects for beautiful visuals
- **Interactive Controls**:
  - `P` - Pause/Resume simulation
  - `R` - Restart with new random positions
  - `ESC` - Exit

## Requirements

### System Requirements
- **GPU**: OpenGL 4.3+ compatible (NVIDIA/AMD/Intel)
- **OS**: Windows 10/11 (Linux/Mac with minor modifications)
- **RAM**: 4GB minimum

### Build Dependencies
- **C++ Compiler**: C++17 compatible (MSVC 2019+, GCC 9+, Clang 10+)
- **CMake**: 3.10 or later
- **OpenGL**: 4.3+ (usually comes with graphics drivers)
- **GLFW**: 3.3+ (window management)
- **GLAD**: OpenGL loader

## Building the Project

### Windows with vcpkg (Recommended)

1. **Install vcpkg** (if not already installed):
```bash
git clone https://github.com/Microsoft/vcpkg.git
cd vcpkg
./bootstrap-vcpkg.bat
./vcpkg integrate install
```

2. **Install dependencies**:
```bash
./vcpkg install glfw3:x64-windows glad:x64-windows
```

3. **Clone and build the project**:
```bash
git clone <repository-url>
cd ParticleSim

# Using CMake presets (recommended)
cmake --preset windows-msvc-vcpkg
cmake --build --preset windows-debug

# Or manually
mkdir build
cd build
cmake .. -DCMAKE_TOOLCHAIN_FILE="[path-to-vcpkg]/scripts/buildsystems/vcpkg.cmake"
cmake --build . --config Release
```

### Linux

```bash
# Install dependencies
sudo apt-get install cmake libglfw3-dev libgl1-mesa-dev

# Build
mkdir build && cd build
cmake ..
make -j4
```

### macOS

```bash
# Install dependencies
brew install cmake glfw

# Build
mkdir build && cd build
cmake ..
make -j4
```

## Running

After building, run the executable from the build directory:

```bash
# Windows
./build/Release/ParticleSim.exe

# Linux/Mac
./build/ParticleSim
```

The simulation starts immediately in fullscreen mode.

## Project Structure

### Core Components

#### `main.cpp`
- Entry point and main loop
- GLFW window setup and OpenGL context creation
- Particle initialization and death/birth cycle
- Input handling (pause, restart, exit)
- Timing and frame management

#### `Renderer.h/cpp`
- **GPU Buffer Management**: Manages particle data as Shader Storage Buffer Objects (SSBO)
- **Compute Shader**: Handles particle physics (attraction/repulsion forces)
- **Vertex/Fragment Shaders**: Multi-pass rendering with glow effects
- **Attraction Matrix**: Texture-based lookup for species interactions

#### `GPUParticle.h`
- Defines the particle data structure (64 bytes, std430 layout):
  - Position (vec2)
  - Velocity (vec2)
  - Radius & Mass (float)
  - Color (RGBA)
  - Species ID (int)

#### `Color.h`
- Species definitions (8 colors: Red, Green, Blue, Yellow, Cyan, Magenta, Purple, Orange)
- Color-to-species mapping
- Attraction matrix (species interaction rules)
- Randomizable attraction coefficients

#### `Geometry.h`
- Basic math structures (Vec2, Color)
- Utility functions for 2D operations

### Supporting Files

#### `randomize_attractions.py`
Python script to generate random attraction matrices for varied behaviors:
```bash
python randomize_attractions.py
```

#### `CMakeLists.txt`
Build configuration with:
- C++17 standard
- OpenGL, GLFW, GLAD package finding
- Compiler warnings enabled

#### `CMakePresets.json`
Predefined build configurations for Windows with vcpkg toolchain

## Technical Details

### GPU Architecture

1. **Compute Shader Pipeline**:
   - Each particle calculates forces from all others within range
   - O(N²) complexity optimized with distance cutoffs
   - Mass-weighted forces with attraction/repulsion based on species
   - Velocity damping for stability

2. **Rendering Pipeline**:
   - **Pass 1**: Outer glow (3x radius, soft falloff)
   - **Pass 2**: Inner glow (1.5x radius, bright core)
   - **Pass 3**: Solid particle core with anti-aliasing

3. **Memory Layout**:
   - Single buffer serves as both SSBO (compute) and VBO (rendering)
   - Zero-copy architecture for GPU-only data flow
   - std430 layout ensures proper alignment

### Physics Model

- **Attraction Force**: `F = k * m₁ * m₂ / r²` (gravity-like)
- **Repulsion**: Activates when particles are within contact distance
- **Damping**: 8% velocity reduction per frame for stability
- **Mass**: Proportional to radius³

## Customization

### Modifying Attraction Rules

Edit the `attractionMatrix` in `Color.h`:
```cpp
attractionMatrix = {
    {RED, GREEN, 0.5f},    // Red attracts to Green
    {RED, RED, -0.3f},     // Red repels other Reds
    // ... more rules
};
```

Or use the Python script for random configurations:
```bash
python randomize_attractions.py > new_matrix.txt
```

### Adjusting Simulation Parameters

In `Renderer.cpp::dispatchComputeShader()`:
- `uMaxDist`: Maximum interaction distance (default: 200px)
- `uRepelDist`: Repulsion activation distance (default: 30px)
- `uDamping`: Velocity damping factor (default: 0.08)
- `uForceScale`: Global force multiplier (default: 0.3)

### Changing Particle Count

In `main.cpp`:
```cpp
constexpr int numPoints = 10000;  // Adjust as needed
```

## Performance

- **10,000 particles**: 60+ FPS on GTX 1060 or better
- **50,000 particles**: 30+ FPS on RTX 3070 or better
- **100,000 particles**: Requires high-end GPU (RTX 4070+)

## Troubleshooting

### Black Screen
- Check OpenGL version: `glxinfo | grep "OpenGL version"` (Linux) or use GPU-Z (Windows)
- Update graphics drivers

### Build Errors
- Ensure vcpkg toolchain path is correct in `CMakePresets.json`
- Verify GLFW and GLAD are installed: `vcpkg list`

### Low FPS
- Reduce particle count in `main.cpp`
- Increase `uMaxDist` to reduce calculation overhead
- Check if running on integrated GPU instead of dedicated

## Acknowledgments

Inspired by:
- Jeffrey Ventrella's Clusters
- Digital Genius' Simulating Particle Life
- Tom Mohr's Particle Life
- Conway's Game of Life