# MinimalCoupler

## Motivation

[preCICE](https://precice.org) is a coupling library for partitioned multi-physics simulations. This project is a minimal reimplementation of the preCICE C++ API, intended as a drop-in replacement for studying and reproducing the behaviour of preCICE's serial-implicit coupling scheme.

The library is hardcoded for the [perpendicular-flap tutorial](https://precice.org/tutorials-perpendicular-flap.html), coupling a `Fluid` and a `Solid` participant. It reproduces preCICE's serial-implicit coupling with Aitken underrelaxation and nearest-neighbour data mapping.

## How It Works

The public headers in `include/precice/` are copied verbatim from preCICE, so the API exposed to solvers is identical. The internal implementation is a complete rewrite that replaces preCICE's general coupling machinery with a minimal, hardcoded configuration for the perpendicular-flap setup.

Because the exported symbols match, any solver already compiled against preCICE can use MinimalCoupler at runtime via `LD_PRELOAD` without recompilation.

The following configuration is hardcoded and must match the `precice-config.xml` used with the tutorial:

- Two participants: `Fluid` and `Solid`, with meshes `Fluid-Mesh` and `Solid-Mesh`.
- Two data fields: `Force` and `Displacement`.
- Serial-implicit coupling with Aitken underrelaxation.
- Data mapping using nearest-neighbour interpolation.

The numeric parameters are defined in `constants.hpp` and correspond to these preCICE XML attributes:

| Constant | Value | Corresponding preCICE XML attribute |
|---|---|---|
| `MAX_TIME` | `5.0` | `max-time` |
| `TIME_WINDOW_SIZE` | `0.01` | `time-window-size` |
| `INITIAL_RELAXATION` | `0.5` | `relaxation:initial-relaxation` |
| `MAX_ITERATIONS` | `50` | `max-iterations` |
| `CONVERGENCE_TOLERANCE` | `5e-3` | `relative-convergence-measure:limit` |
| `MESH_DIMENSIONS` | `2` | mesh `dimensions` attribute |

## File Structure

```
include/precice/   # Public headers, copied verbatim from preCICE
src/
  precice/         # Thin wrappers forwarding to ParticipantImpl
  ParticipantImpl  # Core coupling logic: mesh registration, data exchange, checkpointing
  Mesh             # Per-participant mesh and data field storage
  Aitken           # Aitken underrelaxation algorithm
  Utils            # Helper utilities
  constants.hpp    # All hardcoded configuration values (must match precice-config.xml)
  logger           # Logging support
tests/
  solverdummy.cpp  # Minimal solver dummy for local testing
```

## Prerequisites

- CMake 3.16 or higher
- C++ compiler with C++20 support

## Building

Standard CMake build:

```bash
cmake -B build
cmake --build build
```

## Using with LD_PRELOAD

When a solver binary is already compiled against preCICE, MinimalCoupler can intercept all preCICE API calls at runtime without recompilation:

```bash
LD_PRELOAD=/path/to/libminimalCoupler.so.3 ./run.sh
```

`LD_PRELOAD` instructs the dynamic linker to search the specified library for symbols before any other shared library. Because MinimalCoupler exports the same symbols as preCICE (identical headers), all `precice::Participant` calls resolve to MinimalCoupler's implementation. The file does not need to be named `libprecice.so` — `LD_PRELOAD` matches on symbol names, not on the library filename used by `-l` flags at compile time.

## How to Test

### Solver Dummies

The `tests/` directory contains a minimal solver dummy that exercises the full coupling loop without real physics solvers.

The solver dummy is built as part of the main build. Run both participants in separate terminals:

```bash
# Terminal 1
./build/tests/solverdummy Fluid

# Terminal 2
./build/tests/solverdummy Solid
```

### Perpendicular-Flap Tutorial

To run the full [perpendicular-flap](https://precice.org/tutorials-perpendicular-flap.html) tutorial with MinimalCoupler instead of preCICE:

1. Set up the tutorial as described in the preCICE documentation.
2. Ensure `constants.hpp` matches the `precice-config.xml` used by the tutorial.
3. Preload MinimalCoupler when launching each participant:

```bash
LD_PRELOAD=/path/to/libminimalCoupler.so.3 ./run.sh
```
