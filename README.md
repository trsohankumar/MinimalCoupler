# MinimalCoupler

A minimal coupling library implementation that implements the preCICE coupling API standard.
It couples the simulation between two participants fluid and solid. The type of coupling carried out is a serial implicit coupling accelerated with aitken relaxation. Mapping between vertices from solid to fluid are carried out Nearest Neighbor mapping.

## Usage

As the library extends the preCICE API, it can be utilized in the preCICE software stack in place of the preCICE library. To this end to use the project with any coupling simulation where preCICE can be use simply load the library using LD_PRELOAD while executing the coupling script.

## Build Instructions

### Prerequisites

- Cmake 3.22 or higher
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
The buld process generates:
- `libminimalCoupler.so.3` - The shared library
