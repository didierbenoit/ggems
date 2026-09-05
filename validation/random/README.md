# GGEMS Random Number Validation

This directory contains the scientific statistical-validation workflow for the
random-number generators used by GGEMS.

Random validation is deliberately separate from the ordinary GoogleTest suite.
The tests under `tests/` protect implementation correctness, state progression,
OpenCL integration, ABI contracts, and focused invariants. The workflows in this
directory exercise long generated streams with independent statistical
batteries and are intended to provide reproducible scientific evidence for GGEMS.

The current validation stack uses three complementary external tools:

- [PractRand](#practrand);
- [Dieharder](#dieharder);
- [TestU01](#testu01).

The current GGEMS random engines are:

- Philox 4x32;
- PCG32;
- JKISS.

## Current validation status

The September 2026 reference campaign supports the following GGEMS policy:

| Engine | GGEMS role | Statistical-validation status |
| --- | --- | --- |
| Philox 4x32 | Default engine | Validated for the tested GGEMS multi-stream configurations |
| PCG32 | Alternative engine | Validated for the tested GGEMS multi-stream configurations |
| JKISS | Legacy GGEMS 1.3 engine | Retained for compatibility/reference; not validated for the current GGEMS interleaved multi-stream use |

JKISS is intentionally retained. It was present in GGEMS 1.3 and provides a
useful legacy reference. Its reproducible failures also demonstrate that the
validation workflow is capable of discriminating between random engines rather
than mechanically producing passing results.

"Validated" here does not mean mathematically proven random, cryptographically
secure, or universally valid for every possible seed, device, stream count, or
future execution policy. It means that the current GGEMS implementation passed
the fixed statistical campaigns described below without a recurring severe
failure pattern.

## Scope

This validation answers questions about the random streams produced by the
current GGEMS implementation, including:

- raw `uint32_t` output quality;
- the GGEMS high-24-bit conversion used for binary32 uniforms in `[0, 1)`;
- correlations visible when many worker streams are interleaved;
- within-stream behavior visible in worker-major layouts;
- sensitivity to several fixed seeds;
- sensitivity to shifted stream-identifier ranges;
- scalar versus vectorized simulation-facing random extraction where covered by
  the PractRand campaign.

The validated object is therefore not only an abstract RNG algorithm. It is the
combination of:

```text
GGEMS random engine
+ GGEMS state initialization
+ seed
+ stream identifier
+ worker-stream population
+ OpenCL generation path
+ stream layout
+ simulation-facing conversion when applicable
```

The workflow does not claim history-by-history transport reproducibility. GGEMS
currently assigns complete histories dynamically to persistent worker streams,
so scheduling and device partitioning may change which worker stream transports
a given history.

## Random engines

### Philox 4x32

Philox is the current GGEMS default random engine.

Its compact GGEMS state occupies 24 bytes. GGEMS uses a deterministic
`seed + stream identifier` initialization contract, with 64-bit stream
identifiers.

Philox is the primary validated engine for current GGEMS simulation work.

### PCG32

PCG32 is the validated alternative to Philox.

Its compact GGEMS state occupies 16 bytes. It also uses the GGEMS
`seed + stream identifier` initialization model with 64-bit stream identifiers.

### JKISS

JKISS is the legacy engine inherited from GGEMS 1.3.

Its compact GGEMS state occupies 20 bytes. The current host initialization
contract uses the JKISS 32-bit seed/stream domain.

JKISS remains available as a legacy/reference engine, but the current
multi-stream implementation shows reproducible statistical defects when a large
population of worker streams is observed in interleaved order. These defects
are detected independently by PractRand, Dieharder, and TestU01.

The presence of JKISS in the source tree must therefore not be interpreted as a
claim that it is statistically equivalent to Philox or PCG32.

## Stream model

Each validation case initializes a fixed population of independent GGEMS worker
states. The model is conceptually:

```text
simulation seed + stream identifier -> deterministic initial worker state
```

A stream identifier selects one initialized worker sequence. It is not a
random-access position inside a sequence.

The reference campaigns use `2^20` (`1,048,576`) workers per case.

### Stream layouts

Two layouts are used because they answer different statistical questions.

#### `interleaved`

For each sample depth, values from all workers are emitted consecutively:

```text
worker 0 sample 0
worker 1 sample 0
...
worker N-1 sample 0
worker 0 sample 1
worker 1 sample 1
...
```

This view is intentionally sensitive to cross-stream structure and correlation
between worker streams initialized from neighboring stream identifiers.

#### `worker_major`

All requested samples from one worker are emitted before moving to the next
worker:

```text
worker 0 sample 0
worker 0 sample 1
...
worker 0 sample D-1
worker 1 sample 0
...
```

This view emphasizes the longitudinal behavior of individual worker streams.

A generator may look acceptable in one layout and fail in the other. Such a
difference is scientifically meaningful and must not be hidden by selecting only
the favorable layout.

## Stream types

### `raw_uint32`

The primary statistical-validation stream.

Each sample is one canonical little-endian 32-bit unsigned value produced by
`GGEMS_RndmUInt32`.

PractRand, Dieharder, and TestU01 all use raw `uint32_t` streams as their main
input.

### `uniform24_scalar`

A simulation-facing conversion stream produced through the scalar GGEMS uniform
path.

GGEMS binary32 uniforms are constructed from 24 random bits. For file-based
validation, each reconstructed 24-bit integer is packed into exactly three
little-endian bytes.

The current fixed PractRand campaign does not use this stream type directly,
but the generator supports it for focused experiments.

### `uniform24_vector4`

A simulation-facing conversion stream produced through `GGEMS_RndmUniform4`.

The fixed PractRand campaign exercises 2, 3, and 4 used lanes from each vector
call. This checks the conversion layer and the vector-consumption policy used by
GGEMS source sampling.

Each 24-bit sample is packed into three little-endian bytes and is presented to
PractRand through its byte-oriented input mode.

## Directory layout

Tracked source structure:

```text
validation/random/
├── README.md
├── CMakeLists.txt
├── kernels/
│   ├── random_uint32_stream.cl
│   ├── random_uniform24_scalar_stream.cl
│   └── random_uniform24_vector4_stream.cl
├── tools/
│   ├── GGEMSRandomStreamGenerator.cc
│   ├── GGEMSRandomStreamPipeProducer.cc
│   ├── GGEMSRandomUInt32ChunkProducer.cc
│   └── GGEMSRandomUInt32ChunkProducer.hh
├── practrand/
│   ├── run_campaign.py
│   ├── run_practrand.py
│   └── aggregate_campaign.py
├── dieharder/
│   ├── run_campaign.py
│   ├── run_dieharder.py
│   └── aggregate_campaign.py
└── testu01/
    ├── CMakeLists.txt
    ├── GGEMSTestU01Consumer.c
    ├── run_campaign.py
    ├── run_testu01.py
    └── aggregate_campaign.py
```

Generated directories are created automatically when needed:

```text
validation/random/
├── streams/
│   ├── practrand_campaign/
│   └── dieharder/
└── results/
    ├── practrand/
    │   └── campaign/
    │       ├── manifests/
    │       ├── <case>.json
    │       └── aggregate/
    │           ├── campaign_summary.json
    │           ├── cases.csv
    │           └── anomalies.csv
    ├── dieharder/
    │   └── campaign/
    │       ├── manifests/
    │       ├── <case>.json
    │       └── aggregate/
    │           ├── campaign_summary.json
    │           ├── cases.csv
    │           └── anomalies.csv
    └── testu01/
        └── campaign/
            ├── requests/
            ├── <case>.json
            └── aggregate/
                ├── campaign_summary.json
                ├── cases.csv
                └── anomalies.csv
```

`validation/random/streams/` and `validation/random/results/` are runtime
artifacts and are ignored by Git. A clean checkout is not expected to contain
either directory.

## Core validation tools

### `ggems_random_stream_generator`

Built from `tools/GGEMSRandomStreamGenerator.cc`.

It generates finite binary streams for PractRand and Dieharder and writes a JSON
manifest describing the scientific stream configuration.

The generator supports:

- `jkiss`, `pcg32`, and `philox`;
- raw and 24-bit uniform stream types;
- worker-major and interleaved layouts;
- stream offsets;
- configurable worker/sample counts;
- bounded OpenCL chunks;
- normal GGEMS OpenCL device selectors.

Device-selection semantics belong to `GGEMSOpenCL::SelectDevices()`. The
validation code passes the selector to the central GGEMS OpenCL authority and
must not maintain a private interpretation of values such as `gpu`, `cpu`,
`nvidia`, `amd`, `intel`, `all`, indices, or ranges.

### `ggems_random_stream_pipe_producer`

Built from `tools/GGEMSRandomStreamPipeProducer.cc`.

It produces raw `uint32_t` data directly to standard output and is used by the
TestU01 streaming path.

The producer generates bounded OpenCL chunks. The pipe provides natural
backpressure, so a large logical stream does not require a correspondingly large
RAM allocation or temporary file.

### `GGEMSRandomUInt32ChunkProducer`

Validation-private C++ infrastructure shared by the finite-file generator and
the TestU01 pipe producer.

It reuses the normal GGEMS random/OpenCL authorities and must not become an
independent random implementation.

## Build and prerequisites

Random statistical validation is currently enabled on Linux.

Build the validation executables from the repository root:

```bash
cmake --build build --target validation_random
```

The aggregate `validation` target may also be used:

```bash
cmake --build build --target validation
```

External statistical tools are not GGEMS runtime dependencies.

Reference versions used for the September 2026 campaign were:

| Tool | Reference version |
| --- | --- |
| PractRand | 0.96 |
| Dieharder | 3.31.1 |
| TestU01 | 1.2.3 |

PractRand requires `RNG_test` to be available in `PATH` or supplied explicitly
with `--rng-test`.

Dieharder requires the `dieharder` executable to be available in `PATH` or
supplied explicitly with `--dieharder`.

TestU01 requires its development headers and libraries at CMake configure time.
If they are unavailable, GGEMS simply does not build the TestU01 consumer. The
TestU01 dependency is validation-only and does not affect the GGEMS runtime
library.

## Campaign policy

Official results come from the fixed campaign definitions in each
`run_campaign.py`.

Do not modify seeds, layouts, stream offsets, battery order, or case selection
after observing results in order to improve an outcome.

Do not rerun a statistically suspect case merely to replace it with a more
favorable result. Suspects and failures are scientific results and must be kept
and reported.

`--force` exists for explicit operational regeneration/rerun work. It is not a
mechanism for statistical cherry-picking.

The high-level campaign runners are the preferred interface. In particular,
TestU01 request JSON files under `results/` are generated execution artifacts;
users should not manually construct or edit them for the official campaign.

## PractRand

### Purpose

PractRand is the broadest fixed GGEMS campaign. It tests both raw engine output
and the 24-bit simulation-facing uniform conversion.

The September 2026 reference campaign uses:

- 102 fixed cases;
- `2^20` workers per case;
- 32.02734375 GiB per case;
- PractRand `core` test set;
- multithreaded execution.

### Fixed series

| Series | Stream | Seeds | Engines | Layout | Extra purpose |
| --- | --- | ---: | --- | --- | --- |
| A | `raw_uint32` | 12 | all 3 | interleaved | Main raw multi-stream campaign |
| B | `uniform24_vector4`, 2 lanes | 12 | all 3 | interleaved | Simulation-facing vector conversion |
| C | `uniform24_vector4`, 3 lanes | 3 targeted | all 3 | interleaved | Vector lane-consumption control |
| D | `uniform24_vector4`, 4 lanes | 3 targeted | all 3 | interleaved | Full-vector lane-consumption control |
| E | `raw_uint32` | 3 targeted | all 3 | worker-major | Longitudinal per-worker control |
| F | `raw_uint32` | A01 seed | all 3 | interleaved | Shifted stream identifiers (`+2^20`) |

The targeted seed positions are the first, sixth, and twelfth seeds from the
fixed 12-seed list.

### Usage

List the complete fixed campaign:

```bash
python validation/random/practrand/run_campaign.py --list
```

Preview commands without executing:

```bash
python validation/random/practrand/run_campaign.py --all --device gpu --dry-run
```

Run the complete campaign:

```bash
python validation/random/practrand/run_campaign.py --all --device gpu
```

Run one case:

```bash
python validation/random/practrand/run_campaign.py --case A01-philox --device gpu
```

Keep the generated binary stream for a focused single-case investigation:

```bash
python validation/random/practrand/run_campaign.py --case A01-philox --device gpu --keep-stream
```

`--keep-stream` is intentionally not accepted with `--all`.

By default, completed cases are skipped, so `--all` can resume a campaign.

### Single-case wrapper

`run_practrand.py` runs PractRand from an existing GGEMS manifest. It is useful
for focused work, but the fixed campaign should normally be driven through
`run_campaign.py`.

### Aggregation

After the campaign:

```bash
python validation/random/practrand/aggregate_campaign.py
```

Print without writing aggregate files:

```bash
python validation/random/practrand/aggregate_campaign.py --no-write
```

The aggregate classifies cases as:

- `passed_no_anomalies`;
- `attention_required`;
- `failed`;
- `tool_error`.

PractRand anomaly severity is preserved rather than reduced to a single
pass/fail bit.

## Dieharder

### Purpose

Dieharder provides an independent battery and a much larger finite raw stream per
case than PractRand.

The reference campaign uses:

- 13 fixed cases;
- raw `uint32_t` only;
- `2^20` workers per case;
- 61,036 samples per worker;
- 238.421875 GiB per case;
- 114 assessments per case;
- 106 primary assessments per case.

### Fixed matrix

The campaign contains:

- three fixed seeds for Philox, PCG32, and JKISS in interleaved layout:
  9 cases;
- the same three JKISS seeds in worker-major layout:
  3 cases;
- one JKISS shifted-stream interleaved control at offset `2^20`:
  1 case.

This compact matrix deliberately includes layout and shifted-stream controls
while keeping the very large per-case disk requirement manageable.

### Usage

List the campaign:

```bash
python validation/random/dieharder/run_campaign.py --list
```

Dry run:

```bash
python validation/random/dieharder/run_campaign.py --all --device gpu --dry-run
```

Run all cases:

```bash
python validation/random/dieharder/run_campaign.py --all --device gpu
```

Run one case:

```bash
python validation/random/dieharder/run_campaign.py --case A01-philox --device gpu
```

Keep one generated stream:

```bash
python validation/random/dieharder/run_campaign.py --case A01-philox --device gpu --keep-stream
```

The campaign runner checks disk capacity before generating a Dieharder stream.
Streams are normally removed after their case completes.

Completed cases are skipped automatically when the campaign is resumed.

### Single-case wrapper

`run_dieharder.py` consumes an existing raw GGEMS manifest and writes one
structured Dieharder summary.

The wrapper records every assessment rather than reporting only the final case
classification.

### Aggregation

```bash
python validation/random/dieharder/aggregate_campaign.py
```

The aggregate separates:

- `PASSED`;
- `WEAK`;
- `FAILED`;

and distinguishes the primary assessment set from auxiliary/implementation-caveat
entries.

A few isolated `WEAK` assessments are not treated as equivalent to a reproducible
`FAILED` result. Recurrence across seeds/layouts is part of the interpretation.

## TestU01

### Purpose

TestU01 provides the SmallCrush, Crush, and BigCrush academic batteries through
a dedicated C consumer.

The TestU01 path is streamed:

```text
GGEMS OpenCL chunk producer
        |
        | raw little-endian uint32
        v
bounded pipe
        |
        v
GGEMS TestU01 C consumer
        |
        v
TestU01 battery
```

This avoids creating multi-terabyte temporary files.

### Batteries

The consumer expects the standard TestU01 result counts:

| Battery | Expected result slots |
| --- | ---: |
| SmallCrush | 15 |
| Crush | 144 |
| BigCrush | 160 |

GGEMS classifies a computed TestU01 p-value as a suspect when:

```text
p < 0.001 or p > 0.999
```

A suspect is retained as a scientific observation. It is not automatically a
generator failure. Recurrence, extremeness, related tests, independent seeds,
layouts, and agreement with other batteries all matter.

### Fixed campaign

The current campaign contains 81 battery executions:

- 60 SmallCrush;
- 18 Crush;
- 3 BigCrush.

Series:

| Series | Configuration |
| --- | --- |
| A | Interleaved, offset 0; 12 seeds for SmallCrush, first 4 for Crush, first seed for BigCrush |
| E | Worker-major, offset 0; first 4 seeds for SmallCrush, first seed for Crush |
| F | Interleaved, A01 seed, four shifted stream offsets; all for SmallCrush, first shifted offset for Crush |

All three engines are included symmetrically inside each selected case layer.

Logical per-worker sample depths are battery-specific:

| Battery | Samples/worker | Logical capacity |
| --- | ---: | ---: |
| SmallCrush | 256 | 1 GiB |
| Crush | 34,816 | 136 GiB |
| BigCrush | `2^20` | 4 TiB |

The BigCrush value is a logical capacity, not a disk allocation. The producer
generates only bounded live chunks and stops when the consumer no longer
requests data. For example, the September 2026 Philox BigCrush consumed about
1.299 TiB from a 4 TiB logical capacity.

### Usage

List all fixed cases:

```bash
python validation/random/testu01/run_campaign.py --list
```

Dry run:

```bash
python validation/random/testu01/run_campaign.py --all --device gpu --dry-run
```

Run the complete campaign:

```bash
python validation/random/testu01/run_campaign.py --all --device gpu
```

Run one battery case:

```bash
python validation/random/testu01/run_campaign.py --case A01-philox-bigcrush --device gpu
```

Override the watchdog for a focused run:

```bash
python validation/random/testu01/run_campaign.py --case A01-philox-bigcrush --device gpu --timeout-seconds 43200
```

The default live OpenCL chunk limit is 256 MiB and can be changed for an
operational experiment with `--max-chunk-mib`.

Completed statistically complete cases are skipped when `--all` is resumed.

### Single-case supervisor

`run_testu01.py` supervises one TestU01 case.

It supports:

- an existing finite raw-stream manifest;
- a generated OpenCL pipe request.

The official fixed campaign uses generated pipe requests and does not require
users to create request JSON manually.

### C consumer

`GGEMSTestU01Consumer.c` is a strict C17 adapter between the raw GGEMS word
stream and TestU01.

It provides TestU01 with one `uint32_t` word at a time, records the battery
result slots, and preserves the raw p-values required for later aggregation.

### Aggregation

```bash
python validation/random/testu01/aggregate_campaign.py
```

The aggregate reports:

- clean cases;
- cases with one or more suspect slots;
- incomplete cases;
- technical errors;
- low/high suspect counts;
- recurrent suspect tests.

Incomplete or technical execution is never silently interpreted as statistical
success.

## Campaign execution and resume behavior

The three campaign drivers follow the same high-level command contract:

```text
--list
--case CASE_ID
--all
--dry-run
--device DEVICE
--local-size LOCAL_SIZE
--force
```

Tool-specific options are documented by each script's `--help`.

PractRand and Dieharder generate finite binary streams and normally delete the
large stream after the corresponding statistical tool finishes. Their manifests
and summaries remain under `results/`.

TestU01 generates its requests under `results/` and streams the random data
directly through a pipe.

All required `streams/` and `results/` directories are created automatically.
Their absence in a fresh checkout is normal.

## Result interpretation

A statistical battery produces evidence, not a proof of randomness.

The validation policy is:

1. keep every official result;
2. never replace an unfavorable result with a favorable rerun;
3. distinguish isolated weak/suspect p-values from recurrent failures;
4. compare recurrence across independent seeds;
5. compare interleaved and worker-major layouts;
6. use shifted stream identifiers to distinguish a local stream-range accident
   from a broader initialization/stream-correlation effect;
7. compare conclusions across PractRand, Dieharder, and TestU01;
8. report technical failures separately from statistical findings.

The three batteries overlap statistically and are not independent experiments in
a strict mathematical sense. Agreement between them is nevertheless valuable
because they implement different test families and expose different views of the
same generated streams.

## September 2026 reference outcome

Reference environment:

```text
OpenCL device : NVIDIA GeForce RTX 4060 Laptop GPU
PractRand     : 0.96
Dieharder     : 3.31.1
TestU01       : 1.2.3
```

### PractRand

| Engine | Cases | Clean | Attention | Failed |
| --- | ---: | ---: | ---: | ---: |
| Philox | 34 | 25 | 9 | 0 |
| PCG32 | 34 | 20 | 14 | 0 |
| JKISS | 34 | 9 | 12 | 13 |

The 12 standard raw interleaved JKISS seed cases and the shifted-stream JKISS
case failed. The dominant failures were concentrated in low-bit views. The
worker-major JKISS raw controls did not fail.

### Dieharder

Primary assessments:

| Engine | Cases | PASSED | WEAK | FAILED |
| --- | ---: | ---: | ---: | ---: |
| Philox | 3 | 311 | 7 | 0 |
| PCG32 | 3 | 313 | 5 | 0 |
| JKISS | 7 | 716 | 22 | 4 |

All four JKISS interleaved cases failed `diehard_operm5` with `p = 0`. The three
JKISS worker-major cases produced no failed primary assessment.

### TestU01

| Engine | Cases | Clean | Suspect cases | Suspect slots |
| --- | ---: | ---: | ---: | ---: |
| Philox | 27 | 25 | 2 | 2 |
| PCG32 | 27 | 26 | 1 | 1 |
| JKISS | 27 | 21 | 6 | 9 |

Philox and PCG32 BigCrush completed without suspect slots.

JKISS BigCrush contained four suspect slots, including a very small `Run`
p-value and additional RandomWalk/Fourier/AppearanceSpacings findings.

### Combined interpretation

The three batteries support the same project-level conclusion:

- Philox shows no recurring severe statistical defect in the fixed GGEMS
  campaigns and remains the validated default;
- PCG32 shows no recurring severe statistical defect and remains the validated
  alternative;
- JKISS shows reproducible multi-stream defects, particularly in interleaved
  output, and is retained only as a legacy/reference engine.

The campaigns identify the failure of the current `GGEMS + JKISS + multi-stream
initialization/layout` combination. They do not, by themselves, prove whether
the root cause belongs to the abstract JKISS recurrence, GGEMS stream
initialization, or their interaction. That distinction would require a separate
focused investigation and is not necessary for the current GGEMS engine policy.

## Reproducing the full validation

From a configured Linux GGEMS checkout:

```bash
cmake --build build --target validation_random
```

Then run the campaigns:

```bash
python validation/random/practrand/run_campaign.py --all --device gpu
```

```bash
python validation/random/dieharder/run_campaign.py --all --device gpu
```

```bash
python validation/random/testu01/run_campaign.py --all --device gpu
```

Aggregate them:

```bash
python validation/random/practrand/aggregate_campaign.py
```

```bash
python validation/random/dieharder/aggregate_campaign.py
```

```bash
python validation/random/testu01/aggregate_campaign.py
```

Review all three `campaign_summary.json` files and the corresponding
`anomalies.csv` files before drawing a project-level conclusion.

## Relationship to later GGEMS validation

Random validation is the first layer of the current scientific-validation
sequence:

```text
Random
  -> generic Source distributions
  -> Radionuclide source emission
  -> later transport/physics validation
```

Passing random validation does not validate source geometry, angular
distributions, energy spectra, radionuclide yields, decay timing, particle
transport, physical processes, navigation, scoring, or clinical observables.
Those are separate validation responsibilities built on top of this foundation.
