# MinimalCoupler

A minimal coupling library implementation that exposes the same API as preCICE.
The following configuration is hard-coded into this library:
- Two participants named Fluid and Solid with meshes Fluid-mesh and Solid-mesh respectively.
- Two data vectors for Force and Displacement.
- Vertex mapping carried out using nearest-neighbor mapping.
- Uses serial-implicit coupling with Aitken underrelaxation.

## Build Instructions

### Prerequisites

- CMake 3.22 or higher
- C++ compiler with C++20 support

### Building the Library

1. Create a build directory and navigate to it:
```bash
mkdir build
cd build
```

2. Configure the project with CMake:
```bash
cmake ..
```

3. Build the library:
```bash
make
```

### Build Output
The build process generates:
- `libminimalCoupler.so.3` - The shared library

### Usage Instructions
The library is developed to couple the solvers in the perpendicular-flaps tutorials of preCICE. Run the simulations using library preloading:
```bash
LD_PRELOAD=/path/to/libminimalCoupler.so.3 ./run.sh
```
