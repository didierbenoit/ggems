# GGEMS Random Number Validation

This directory contains the scientific validation workflow for GGEMS random number generators.

The current GGEMS OpenCL random engines are:

- JKISS;
- PCG32;
- Philox 4x32.

The goal is not to replace GoogleTest. Fast GoogleTest-based checks remain in `tests/` and validate compilation, kernel execution, reproducibility, state advancement, and basic output invariants.

This directory is used for heavier statistical validation workflows intended for scientific reporting and reproducible publication results.

## Validation principles

Random validation must record enough metadata to regenerate a stream:

- GGEMS commit hash;
- random engine;
- seed;
- number of generated values;
- stream layout;
- OpenCL platform;
- OpenCL device;
- OpenCL driver version;
- OpenCL build options;
- operating system;
- compiler information;
- validation tool name and version.

## Stream types

Two kinds of streams must be distinguished:

- raw `uint32_t` streams, used for serious statistical batteries;
- converted `float` uniform streams in `[0, 1)`, used to validate GGEMS simulation-facing random values.

The raw `uint32_t` stream is the primary source for statistical batteries such as PractRand, TestU01, and Dieharder.

The `float` stream validates the GGEMS conversion layer and simulation-facing API.

## Git policy

Large generated streams and raw validation outputs are ignored by Git.

Tracked files include:

- validation configurations;
- validation scripts;
- validation kernels;
- README files;
- small summary files under `results/summary/`.

Ignored files include:

- binary streams;
- large raw logs;
- large intermediate result files.
