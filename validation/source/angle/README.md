# Source / Angle: A1 canonical angular validation

A1 measures the angular state emitted by the production GGEMS CountDriven Source
against its requested analytical laws. The path is ordinary `GGEMSSource`
configuration -> `GGEMSRun` -> production OpenCL primary initialization -> raw
Observer `Source` records -> the shared sample exporter -> NumPy -> Matplotlib.
There is no alternative angular sampler or production API in this validation.

All cases use one Analytic CountDriven Gamma source, center `(0, 0, 0) pm`,
default identity frame, Mono `511 keV`, static time `0 ps`, weight `1`, and Philox.
The prior Random qualification applies within its tested scope; A1 measures the
angular transformations consuming those outputs. It is not another RNG campaign.

| Case | Geometry | Angular configuration | Position / angular Uniform4 calls |
|---|---|---|---|
| `A1_fixed` | Point | Exact stored default +Z | 0 / 0 |
| `A1_isotropic_full_sphere` | Point | No-argument Isotropic API | 0 / 1 |
| `A1_isotropic_bounded` | Point | Theta 20..60 deg, phi -45..90 deg | 0 / 1 |
| `A1_focused_rectangle` | Rectangle, full width 40 mm and height 20 mm | Global focus (0, 0, 100) mm | 1 / 0 |

CountDriven static time, Mono energy, and source lookup consume no RNG. Isotropic
uses X/Y of one angular Uniform4 and discards Z/W. Focused consumes no angular
random value: its direction is determined by the sampled, committed position.
These draw budgets come from the current production helpers and existing draw
tests; A1 does not instrument RNG states or duplicate the sampling formulas.

## Build and run

Use the configured GGEMS toolchain and existing dependencies. The target remains
`ggems_source_sample_exporter`; no additional executable or CMake option is needed.
The `validation_source` and aggregate `validation` targets build without running
a scientific campaign. Build and generated data can stay entirely in scratch:

```console
cmake -S . -B codex_scratch/build
cmake --build codex_scratch/build --target ggems_source_sample_exporter
python validation/source/angle/run_campaign.py --exporter codex_scratch/build/validation/source/ggems_source_sample_exporter --device 0 --primaries 256 --workers 64 --seed 77777 --output-dir codex_scratch/a1_smoke
```

Use the `.exe` suffix on Windows, and the configuration subdirectory when using
a multi-configuration generator. Supply the same dependency/toolchain settings
as the existing GGEMS environment when configuring a separate build. The runner
requires Python 3.12+, NumPy, and Matplotlib. It never installs packages. SciPy
is unnecessary. `--no-plots` explicitly measures with NumPy alone; otherwise a
missing Matplotlib dependency is an error. For a headless environment, select
Matplotlib's `Agg` backend with `MPLBACKEND=Agg`.

`--device` is required and forwarded unchanged to `GGEMSOpenCL::SelectDevices()`.
The metadata identifies the actual selected devices in selection order.
`--cases` selects one or more exact case names from the table. `cases.py` owns
the four configurations and modest development defaults: 4,096 primaries,
4,096 workers, seed 77,777. Publication sample sizes remain undecided.

The default output is the ignored `validation/source/results/angle/`. A supplied
`--output-dir` is also supported. Each case directory must be new, preventing a
failed rerun from leaving an apparently current summary or figure. Successful
cases contain `samples.csv`, `metadata.json`, `export.log`, `summary.json`, and,
except Fixed or `--no-plots`, `angle.png` and `angle.pdf`. Generated data and
figures do not belong in the source patch.

Standalone bounded extraction and analysis:

```console
codex_scratch/build/validation/source/ggems_source_sample_exporter --device 0 --geometry point --case-name A1_isotropic_bounded --angular bounded-isotropic --theta-min-deg 20 --theta-max-deg 60 --phi-min-deg -45 --phi-max-deg 90 --primaries 256 --workers 64 --seed 77777 --output samples.csv --metadata metadata.json
python validation/source/angle/analyze.py --samples samples.csv --metadata metadata.json --output-dir analysis
```

Angular options are explicit: `--angular fixed|isotropic|bounded-isotropic|focused`.
Bounded requires all four degree bounds; Focused requires all three
`--focus-x-mm`, `--focus-y-mm`, `--focus-z-mm` coordinates, including zeros.
Parameters belonging to another mode are rejected. Units conversion and
configuration validation delegate to the central GGEMS Units and Source APIs.
Frame/rotation controls are documented in the [G2/A2 domain](../frame/README.md);
A1 keeps the default identity frame. Specify `--case-name` for standalone A1
analysis; the exporter's historical default remains `G1_<geometry>`.

## Extraction and backward compatibility

Existing G1 invocations without angular options still mean Fixed +Z and retain
their exact-direction check. Geometry scripts and semantics are unchanged.
The CSV remains:

```text
source_index,source_local_primary_id,global_primary_id,x_pm,y_pm,z_pm,direction_x,direction_y,direction_z,energy_micro_eV,time_ps,weight,record_kind
```

Integer fields remain exact decimal integers. Direction and weight serialization
retains `numeric_limits<float>::max_digits10`. A1 reconstructs each binary32
direction before promoting it to binary64, so the decimal approximation itself
does not contaminate norm/error measurements. Observer human-readable output is
never scientific input. Source records precede diagnostic projection; Terminal
positions do not enter the analysis.

Capture is complete: the exporter checks zero overflow and the exact current
Source/Terminal count, then exports Source only, sorted by slot and local ID.
Python checks the exact CSV header, fields, count, sorted unique provenance,
slot 0, source-local/global IDs `[0, N)`, Mono energy, static time, weight, and
finite directions. Point positions and the Rectangle Z=0 plane are exact checks.
Malformed data are errors, not silently filtered or repaired. A zero direction
cannot define a ray and is also an error.

The current Observer needs capacity `2*N <= UINT32_MAX`. Both device and host
capacities are set before initialization. Practical capture sizes are much
smaller: the current Run still constructs its human-readable dump even when
logging is filtered, including expensive per-history rescans. A1 preserves that
production path without redesigning Observer or Output.

Metadata keeps G1 fields and adds the requested angular configuration,
`requested_bounded_degrees`, all four actual packed `isotropic_cos_theta_*` and
`isotropic_phi_*_rad` fields, and exact `focus_position_pm` plus its central Units
conversion to mm. Existing `frame_axes` and `fixed_direction` remain the actual
stored axes. The runner adds the checkout `git_commit` when available through
the same lightweight `git rev-parse HEAD` path as G1. This is checkout provenance,
not an executable fingerprint or qualification certificate.

## Analytical measurements

Every case reports min/max/mean observed norm and maximum `|norm - 1|`, computed
before analytical normalization. Exact Fixed reports mismatch count and maximum
component difference against the actual stored +Z. Any Fixed mismatch fails;
no statistical test or mandatory figure is needed.

Full-sphere Isotropic explicitly uses the no-argument API. Its exact packed
domain selects the canonical global kernel branch; an equivalent shifted phi
interval would instead exercise the bounded branch and is not a substitute.
Use observed `u_cos = (dz + 1)/2` and `u_phi = atan2(dy, dx)/(2*pi)`, adding
`2*pi` to negative azimuths. At `dx == dy == 0`, phi is undefined: exclude that
row only from phi statistics and paired correlations/density, and report north
and south pole counts. Cosine statistics still include it.

Bounded Isotropic uses actual stored binary32 bounds from metadata:

```text
u_cos = (dz - cos_theta_lower) / (cos_theta_upper - cos_theta_lower)
u_phi = (atan2(dy, dx) - phi_min) / (phi_max - phi_min)
```

This selected shifted interval lies inside atan2's ordinary `[-pi, pi]` range.
Theta is measured from local +Z; phi starts at local +X toward +Y. Theta itself
is not uniform. The ideal laws are independent Uniform[0,1] in cosine and phi.
Requested degrees identify the case; they are never used to recompute executable
cosine/phi bounds. Binary32 interpolation, trigonometry, and bounded-direction
normalization may produce small endpoint excursions. Report counts below/above
each bound and the maximum normalized excursion separately, without clipping
or introducing a tolerance. Exact poles, if present, are reported and excluded
from phi here as well.

Uniform statistics contain count, min/max, mean, population variance, ideal
mean 1/2 and variance 1/12, and the maximum empirical-CDF deviation. The NumPy
implementation evaluates both sides of every empirical jump, including the
ideal CDF's constant tails outside [0,1]. It produces neither p-values nor
statistical PASS/FAIL. Undefined statistics are JSON `null`. Pearson correlation
uses common non-pole rows and reports its count; it measures linear dependence
and does not establish joint independence. Full sphere additionally reports
first, second, and cross Cartesian moments, with ideal values 0, 1/3, and 0.

Focused uses the exact committed integer-pm position and configured integer-pm
focus. Subtract them as Python integers before binary64 conversion. Normalize
this reference by its Euclidean length for analysis only; do not reproduce the
OpenCL scaling/normalization algorithm. Compare it with the observed unit
direction using stable `atan2(|cross|, dot)` angular error. Report minimum dot,
mean/max angular error in radians, and mean/max/median/95th-percentile miss in pm:

```text
miss = |(focus - position) x observed_direction| / |observed_direction|
```

Also report nonforward directions and unique committed positions. The cross
formula measures distance to the supporting line; it is the ray's closest
approach when the focus is forward, as required by this canonical configuration.
Inspect the nonforward count together with miss and angular errors. Focused is
a deterministic geometric constraint, not an independent probability density.

## Figures, interpretation, and open decisions

The two Isotropic cases produce a compact equal-solid-angle `u_phi` versus
`u_cos` density, both uniform marginals, and both ECDF comparisons. Rectangle
areas in these transformed coordinates correspond to equal solid angle.
Automatic data limits retain excursions; no naive theta-versus-phi latitude map
is used. Focused shows Source XY positions, physical focus miss, and angular-error
ECDF. Every generated figure is saved as PNG and PDF. Fixed needs only its exact
summary. `summary.json` embeds metadata, representation notes, structural status,
measurements, exclusions, support, figure paths/status, and
`acceptance_thresholds = null`.

Binary32 direction/trigonometric arithmetic and finite high-24-bit uniform grids
are compared with ideal continuous laws. Picometer storage does not imply
picometer directional accuracy over a 100 mm flight. No norm, angular, support,
miss-distance, or statistical acceptance tolerance is frozen here. Structural
failures remain immediate errors. Suitable scientific sample counts, uncertainty
reporting, accepted error budgets, device coverage, and publication criteria
remain open. Scheduling and worker/device choices can alter primary-to-stream
assignment; sorted output is not a bitwise per-primary reproducibility guarantee.

A1 stops at the default identity frame. A2 rotations/source frames, Energy,
Time, ActivityDriven/radionuclides, external Monte Carlo comparisons, and physical
transport validation are outside this slice.
