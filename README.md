# MinimalCoupler

A minimal coupling library implementation that implements the preCICE coupling API standard.

## Build Instructions

### Prerequisites

- Cmake 3.28 or higher
- C++ compiler with C++23 support

### Building the Library

#### Option 1: Using the build script
Simply run the provided script:
```bash
./run.sh
```

This script will automatically clean any existing build, create a fresh build directory and compile the library

#### Option 2: Building Manually
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
