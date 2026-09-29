# Source scientific validation

This is the entry point for scientific validation of the current GGEMS 2.0
analytical CountDriven **primary-generation** path. The campaigns cover
geometry, angular distributions, energy distributions, chronology, pose/frame
transforms, and integrated sampler composition, with Philox as the reference RNG.
Claims apply within the tested cases, numerical representations, and execution
configurations.

Source validation measures actual production OpenCL output:

    public GGEMSSource configuration
        -> immutable Source run snapshot
        -> ordinary GGEMSRun primary initialization
        -> raw Observer Source records
        -> shared validation-only CSV exporter
        -> analytical / exact Python analysis
        -> Matplotlib PNG and PDF figures

No validation sampler generates substitute GGEMS results. The shared executable
is `ggems_source_sample_exporter` in `tools/`. It extracts raw Observer records.
Their append order depends on scheduling: analysis uses source_index and source_local_primary_id, with global
IDs retained and checked. Human-readable dumps, rendered traces, and Terminal
positions are not Source samples.

## Domains and implemented capabilities

| Domain | Scientific responsibility | Entry point |
| --- | --- | --- |
| G1 Geometry | Point; planar Rectangle and Ellipse/Circle; uniform-volume Box, Sphere, Cylinder in the canonical frame | [Geometry README](geometry/README.md) |
| A1 Angle | Fixed; global full-sphere Isotropic; bounded equal-solid-angle Isotropic; Focused global target | [Angle README](angle/README.md) |
| E1 Energy | Exact Mono and DiscreteLines; RegularSpectrum with its finite integer ticket-induced law | [Energy README](energy/README.md) |
| T1 Time | CountDriven birth at the committed Run-window start; static/configured chronology and clock-only reset | [Time README](time/README.md) |
| G2/A2 Frame | Source translation/orientation, packed-frame inverse analysis, exact permutation/invariance and global Focused semantics | [Frame README](frame/README.md) |
| I1 Integration | Simultaneous position + bounded angle + spectrum + oblique frame; exact Source draw-budget pairs | [Integration README](integration/README.md) |

Circle is the equal-diameter Ellipse public configuration, not another kernel
sampler. Canonical full-sphere Isotropic is intentionally global and invariant
under Source rotation. Bounded Isotropic is local and rotates with the frame;
Fixed follows stored axis_z. Focused aims from each actual committed global
position toward one global focus point.

The shared exporter reads the executed configuration from the last successful
Source snapshot, so metadata describes the samples actually captured.

## Build and prerequisites

Run commands from the repository root using an already configured GGEMS build.
The `validation_source` target builds the shared exporter without running a
campaign:

```console
cmake --build build --target validation_source
```

The domain examples use `build/validation/source/ggems_source_sample_exporter`.
On Windows, add `.exe`. For a multi-configuration generator, select the build
configuration and include its directory in the exporter path, for example:

```console
cmake --build build --config Release --target validation_source
python validation/source/geometry/run_campaign.py --exporter build/validation/source/Release/ggems_source_sample_exporter.exe --device gpu --cases point rectangle --primaries 256 --workers 64 --output-dir validation/source/results/geometry/example
```

For a single-configuration generator, the profile is selected when configuring
the build; there is no `Release/` executable subdirectory. Adapt `build/` and
`--device` to the local setup. Each rerun needs a new case output directory.

Use Python 3.12 or newer. Geometry, angle, frame, and integration analysis require
NumPy. Energy and time analysis use the standard library. Matplotlib is needed
for requested figures; `--no-plots` skips them.

## Numerical representation and interpretation

Current scientific storage is:

- Positions and Source center: signed int64 pm; geometry dimensions: uint64 pm.
- Directions and actual packed Source axes/angular limits: binary32.
- Energy: uint64 **micro-eV**, including exact table centers and regular-bin width.
- Time: uint64 ps.
- Tabulated energy probabilities: exact cumulative uint32-ticket boundaries
  stored in uint64, ending at 2^32.

GGEMS Units owns physical conversion. CSV integer fields are never rounded for
presentation. Python reconstructs binary32 directions before binary64 analysis.
Oblique position analysis subtracts the exact integer center before applying
the inverse of the actual packed frame, rather than its nominal ideal rotation
or transpose. Binary32 arithmetic and integer-pm commitment can leave measured
support/plane residuals; no silent clipping or arbitrary tolerance hides them.
RegularSpectrum uses an exact finite counting CDF, not an idealized continuous
spectrum as its exact execution authority.

Energy fields already use micro-eV in the current exporter and analyzers.
Captures made with another energy scale are not compatible with this schema;
do not relabel their values as micro-eV.

Philox qualification and Source validation are separate evidence. The
[Random campaign](../random/README.md) qualifies the engine in its tested scope;
Source campaigns measure transformations and their composition. Current RNG
state belongs to workers, and atomic scheduling can change a primary's worker.
Exact cross-run pairs therefore use one worker and one device and pair records
by provenance. Statistical multi-worker captures make no bitwise replay claim.

Exact deterministic, provenance, support/reachability where specified, and
fixed-field contracts fail immediately when violated. Distribution statistics,
correlations and oblique numerical residuals remain descriptive, with
acceptance_thresholds = null. No p-values or arbitrary statistical acceptance
thresholds are introduced. Small correlations do not prove independence.

## Running a campaign and reading its results

Start with the small example in a domain README, then adjust `--primaries`,
`--workers`, and `--seed` for the distributions you want to measure. Exact
Point, Fixed, Mono, and Time checks need few samples. Distribution measurements
benefit from larger populations, at the cost of longer runs and larger captures.
The current Observer capture can be expensive in both runtime and host memory.

Each domain provides `run_campaign.py` to extract and analyze samples and
`analyze.py` to reanalyze an existing capture. Use `--help` on either script for
its options. `--cases` selects individual cases; omitting it runs all cases in
the domain. Frame and integration pairs require a single selected device and
use one worker to reproduce the same random sequence.

Generated files go under `validation/source/results/` by default, or under the
chosen `--output-dir`. Use a new case directory for each run. Keep the raw CSV,
`metadata.json`, and `export.log` together with `summary.json` and any PNG/PDF
figures so that each measurement can be traced to its capture.

Read `summary.json` alongside the figures. Exact field and provenance checks
must pass. Distribution statistics and numerical residuals are measurements,
not an automatic statistical pass/fail decision. Compare results using their
recorded seed, population, worker count, devices, and numerical representation.

## Scope

These campaigns cover analytical CountDriven Source primary generation:
geometry, angular distribution, energy distribution, chronology, frame
transforms, and their composition. They use the initialized Source records,
not the later diagnostic transport positions.

ActivityDriven/radionuclide emission, voxelized and phase-space sources,
multi-source configurations, physical transport, interaction physics, and
clinical accuracy are outside this validation. The 120 kVp spectrum in `data/`
is an available input file, not a validated campaign case.
