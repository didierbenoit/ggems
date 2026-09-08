# Source scientific validation

This is the entry point for scientific validation of the current GGEMS 2.0
analytical CountDriven **primary-generation** path. The retained campaign covers
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
is ggems_source_sample_exporter in tools/. Observer is temporary extraction
infrastructure. Its scheduling-dependent append order is not scientific
provenance: analysis uses source_index and source_local_primary_id, with global
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

The exporter composes existing public Source options and reads executed
configuration from the last successful immutable snapshot. It remains
validation-only; it adds no production Source/Output API.

## Numerical representation and interpretation

Current scientific storage is:

- Positions and Source center: signed int64 pm; geometry dimensions: uint64 pm.
- Directions and actual packed Source axes/angular limits: binary32.
- Energy: uint64 **meV**, including exact table centers and regular-bin width.
- Time: uint64 ps; weight: binary32.
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

The project-wide Energy/EnergyChange migration to uint64/int64 micro-eV (ueV)
was approved on September 8, 2026. It remains separate future implementation:
this checkpoint executes meV. Focused E1 and I1 reruns are required after that
migration; the two scales must never be mixed.

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

## Reference campaigns, generated evidence, and boundary

Keep deterministic cases small: exact Point/Fixed/Mono/Time and exact frame pairs
gain little from large captures. The retained **150000-primary statistical
G1/A1/E1/G2-A2 results** are the higher-statistics reference campaign; individual
domain development defaults are smaller infrastructure smokes. I1 is the compact
final composition checkpoint: 8192 primaries/64 workers for the integrated case
and 2048 primaries/one worker per exact-pair member, seed 20260908 by default.
Do not rerun publication-scale captures merely to test an implementation patch.

Each domain README documents its explicit device selection, case CLI, metadata,
analysis and figure outputs. Build the common target with the configured
toolchain; all runners use installed Python/NumPy/Matplotlib dependencies.
Generated CSV/JSON/PNG/PDF/logs belong under the ignored results/ directory or
caller-selected output directories. They are evidence, not ordinary source
files. The current Observer path constructs expensive diagnostic dumps; later
high-statistics reruns should use performant scientific Output when available.

The precise checkpoint statement is:

> The current analytical CountDriven Source primary-generation path has been
> validated within the tested scope for geometry, angular distribution, energy
> distribution, chronology, pose/frame transforms, and integrated sampler
> composition using Philox as the reference RNG.

After Source initialization, current production transport is still the
diagnostic one-meter projection. It does not establish physical transport,
Navigation, interaction physics, clinical accuracy, or time of flight.
ActivityDriven execution exists but ActivityDriven/radionuclide scientific
validation is separate and is not covered by this checkpoint.
Voxelized Source and PhaseSpace Source remain future unimplemented families.
Realistic 120 kVp, multi-source campaigns, external-reference radionuclide
validation, and future performant Output campaigns are separate work. The useful
120 kVp file in data/ is preserved as input data, not a claim of validation.
