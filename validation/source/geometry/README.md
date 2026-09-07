# Source / Geometry: G1 canonical frame

This development campaign measures the positions emitted by the current GGEMS
CountDriven Source against continuous analytical geometry laws. The authority is
`GGEMSSource` configuration -> ordinary `GGEMSRun` production OpenCL primary
initialization -> raw Observer `Source` records -> CSV -> NumPy -> Matplotlib.
The exporter contains no alternative position sampler. The diagnostic one-meter
projection and its `Terminal` positions are not geometry samples.

Only geometry varies: one CountDriven Analytic Gamma source, center exactly
`(0, 0, 0) pm`, default identity frame, Fixed direction `+Z`, Mono `511 keV`,
weight `1`, static time `0 ps`, and Philox. Point consumes no position random
draw; every other geometry consumes one position `Uniform4`. Fixed direction,
Mono energy, static CountDriven time, and source lookup consume no random draw.
The prior Philox qualification applies within its tested Random scope; this
campaign measures the transformations that consume its output.

G1 does not validate rotations/translations, angular distributions, energy
distributions, chronology, ActivityDriven sources, radionuclides, or physical
transport. Exact fixed-field checks only establish the G1 extraction contract.

## Build and run

Use the existing GGEMS CMake configuration and dependencies. Source validation
is available on every supported GGEMS host platform and is separate from the
Linux-only Random external test suites. It adds no production library or API.

```console
cmake -S . -B build
cmake --build build --target ggems_source_sample_exporter
python validation/source/geometry/run_campaign.py --exporter build/validation/source/ggems_source_sample_exporter --device gpu
```

On Windows use the `.exe` suffix; multi-configuration generators may also place
the executable in `Debug/` or `Release/`. The `validation_source` and aggregate
`validation` targets build the exporter without running a campaign. The script
requires Python 3.12 or newer, NumPy, and Matplotlib in the selected environment.
It never installs packages. SciPy is not required.

`--no-plots` on the runner or standalone analyzer performs the measurements with
NumPy alone and explicitly records that figures were not requested. The default
requires Matplotlib and generates both formats; missing plotting dependencies
are reported as errors rather than silently replacing or omitting figures.

The device selector is required and passed unchanged to
`GGEMSOpenCL::SelectDevices()`. Use a selector valid for the local GGEMS runtime;
no device index or vendor is assumed by the campaign. A selected set of devices
is supported, and metadata records their actual names in selection order.

For a small infrastructure smoke run:

```console
python validation/source/geometry/run_campaign.py --exporter build/validation/source/ggems_source_sample_exporter --device 0 --cases point rectangle --primaries 256 --workers 64 --seed 77777 --output-dir codex_scratch/g1_smoke
```

Counts, workers, seed, and output location are configurable. `cases.py` owns the
complete dimensions and the development defaults: 4,096 primaries, 4,096 workers,
and seed 77,777. Those counts are for implementation inspection, not article
statistics. Use `--primaries 100000` when deliberately increasing the sample
size after inspecting the extraction cost.

The default generated location is `validation/source/results/geometry/`, which
is ignored by Git. Each selected `G1_<geometry>` directory must be new; choose a
different `--output-dir` for a rerun. This prevents stale summaries or figures
from appearing to describe a failed new extraction. Each case contains
`samples.csv`, `metadata.json`, `export.log`, `summary.json`, and its figures.
Only code and analytical definitions belong in the source patch.

The standalone extraction boundary is also usable directly:

```console
build/validation/source/ggems_source_sample_exporter --device 0 --geometry rectangle --primaries 256 --workers 64 --seed 77777 --size-x-mm 40 --size-y-mm 20 --size-z-mm 0 --output samples.csv --metadata metadata.json
python validation/source/geometry/analyze.py --samples samples.csv --metadata metadata.json --output-dir analysis
```

Dimension options represent complete X/Y/Z dimensions. Circle requires equal
X/Y diameters and zero Z; Sphere repeats its diameter in all three axes;
Cylinder repeats its diameter in X/Y and uses Z for its full height. Point has
all dimensions zero. Dimensional values are converted once through GGEMS Units.
The exporter records the actual committed pm values and their central Units
conversion to mm; no independent mm-to-pm conversion is used in Python.

## Capture and file contract

The current production transport emits one `Source` and one `Terminal` record
per primary. The exporter checks `2 * primaries` before narrowing to the current
Observer's `uint32_t` capacities, sets both device and host retention capacities,
and requests every source-local primary. This extraction tool consequently
supports at most `floor(UINT32_MAX / 2)` primaries per invocation. This is an
Observer capture limit, not a production Run-total limit; memory and runtime
will impose much smaller practical limits.

The current `GGEMSRun` constructs its human-readable Observer dump even when
logging is filtered. Its per-primary track-map construction rescans the records, so
large full captures can be expensive in time and host memory. This campaign
suppresses printing through the existing Logger detail filter but cannot suppress that work
through the current public API. It does not redesign Observer or bypass Run.

Overflow, missing records, duplicate provenance, wrong slot/kind, malformed
files, and violations of the exact G1 configuration fail execution. Source
records are sorted by `(source_index, source_local_primary_id)` before export.
The single fresh Run has slot `0`, source-local IDs `[0, N)`, and corresponding
global IDs starting at `0`. CSV fields are:

```text
source_index,source_local_primary_id,global_primary_id,x_pm,y_pm,z_pm,direction_x,direction_y,direction_z,energy_meV,time_ps,weight,record_kind
```

Scientific integer fields remain decimal integers, with no display rounding.
`record_kind` is the literal `Source`. Binary32 direction and weight values use
sufficient decimal precision for round-trip representation. Python reads integer
columns as integers before constructing temporary numerical analysis arrays.
The CSV is a validation boundary, not a GGEMS public production Output format.

Metadata includes case, geometry, complete committed dimensions, count/workers,
Philox seed, selector and selected device names, center, frame axes, direction,
energy, chronology, weight, provenance origin, and Observer counts. The runner
adds `git_commit` when the checkout commit is available; it identifies the
checkout and is not an executable attestation. No fingerprints or certification
metadata are introduced.

Sorting gives a deterministic row order, not bitwise sample reproducibility.
The production path assigns histories to persistent worker streams through
concurrent atomic scheduling. Worker count, device set/order, scheduling, and
Observer instrumentation can change which stream supplies each primary.

## Analytical measurements

Dimensions are full widths/diameters/heights. In the identity frame, exported
global coordinates are the local geometry coordinates.

| Case | Complete dimensions (mm) | Variables compared with ideal Uniform[0,1] |
|---|---|---|
| Point | 0, 0, 0 | Exact `x = y = z = 0`; no distribution test |
| Rectangle | 40, 20, 0 | `u_x = x/W + 1/2`, `u_y = y/H + 1/2` |
| Ellipse | 40, 20, 0 | `q = (x/a)^2 + (y/b)^2`, normalized `atan2(y/b, x/a)` |
| Circle | 30, 30, 0 | Same Ellipse public configuration with `a = b` |
| Box | 40, 20, 10 | `u_x`, `u_y`, `u_z = z/D + 1/2` |
| Sphere | 30, 30, 30 | `q_r = (r/R)^3`, `u_cos = (z/r + 1)/2`, normalized azimuth |
| Cylinder | 30, 30, 40 | `q_r = (r/R)^2`, `u_z = z/H + 1/2`, normalized azimuth |

Here `a` and `b` are ellipse semiaxes, `R` is radius, and azimuth is reduced
modulo `2*pi` and divided by `2*pi`. Sphere uses the three-dimensional radius;
Cylinder uses the transverse radius. Zero-radius samples are excluded from
statistics requiring an angle, with exclusion counts retained. Sphere azimuth
is also undefined at a nonzero pole (`x = y = 0`); those samples are excluded
from azimuth-only statistics and counted separately. Their cosine remains valid.
Correlations use the common valid rows for each pair and report their counts.

For each transformed variable, report count, min/max, mean, population variance
(`ddof=0`), and empirical-CDF maximum deviation. Ideal mean is `1/2` and variance
is `1/12`. The CDF calculation evaluates both sides of every empirical jump
against the analytical Uniform CDF, including its constant tails outside
`[0,1]`; samples themselves are never clipped. Correlation describes pairwise
linear dependence, not a proof of joint independence. Undefined statistics are
JSON `null`, not invented zero values or nonstandard JSON NaN.

Every case reports ideal-support excursions, maximum support excess, and
repeated integer lattice coordinates. Point reports exact nonzero-coordinate
count and maximum absolute coordinate. Planar cases also check exact `z == 0`.
Cartesian, radial, and axial support measurements remain separate where useful.

## Numerical interpretation and figures

Host dimensions and center are exact canonical integers. OpenCL geometry
arithmetic uses binary32; the calculated displacement is then committed to the
integer-pm lattice. Analytical transforms in Python use binary64 intermediates,
while the CSV retains the original integers. Integer-pm storage does not imply
one-picometer sampling accuracy. The continuous ideal law is a reference,
not an exact description of every attainable finite-precision point.
Support membership uses exact integer polynomial comparisons. Radial excess
subtracts exact integers before binary64 division, preserving tiny excursions
that would disappear in a direct floating radius-minus-boundary subtraction.

Report tiny radial or endpoint excursions as measurements. Do not clip, repair,
label them automatically as bugs, or impose an unapproved tolerance. No p-value,
KS threshold, or statistical PASS/FAIL is defined. Exact Point and structural
contract failures do fail. Final scientific acceptance criteria, useful sample
sizes, supported numerical domain, and article statistics remain open for review.

Matplotlib saves compact PNG and PDF figures using its default color cycle.
Density projections, transformed marginal distributions/CDFs, and radial or
axial plots use actual exported samples and analytical reference curves only.
Point needs no scientific plot. Figure paths and all measured quantities are
written to each case's machine-readable `summary.json`, with a concise terminal
summary. Review the figures together with support and exclusion metrics; a
projection alone cannot establish uniform volume sampling.
