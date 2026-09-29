# Source frame / pose: G2 Geometry and A2 Angle

G1/A1 measure canonical sampler laws. G2/A2 measures how production Source
translation and orientation carry those laws into global space. Samples follow
this path: public GGEMSSource configuration -> immutable Source run snapshot ->
ordinary CountDriven GGEMSRun and production OpenCL primary initialization ->
raw Observer Source records -> the existing validation exporter -> NumPy
exact/analytical analysis -> Matplotlib.

The campaign analyzes samples from the production Source path using the shared
exporter.

## Frame configuration

The public pose interface is:

    source.SetPositionPicoMeter(center_x_pm, center_y_pm, center_z_pm);
    source.SetOrientation(direction, up_reference); // std::array<double, 3>

Central MakeQuantity<PositionCoordinate>(value, "mm") converts CLI centers to
exact signed pm. SetOrientation constructs the frame in production: normalize
the direction as local Z; construct X from up_reference cross direction; construct
Y from direction cross X. The production geometry helpers normalize the axes
before binary32 packing. Up is a reference, not an independently stored axis.
The public Source API rejects near-parallel inputs and invalid frames.
Changing the pose does not select an angular mode or consume random draws.

The exporter reads actual axes and center from
GetLastSourceRunSnapshot()->GetRecords()[0] after execution. It neither
reconstructs them from the request nor writes packed fields behind the API.
The existing frame_axes JSON array lists X, Y, Z vectors in that order.
Production uses those vectors as **columns** of M:

    global_delta = axis_x * local_x + axis_y * local_y + axis_z * local_z
    M = [axis_x axis_y axis_z]

The analyzer requires frame_matrix_convention = "axes_as_columns". It verifies
metadata shape, finite exact binary32 components, nonsingularity, and
right-handedness. Identity/cyclic axes are also checked exactly against their
mathematical fixtures. Host frame validation belongs to GGEMS; the analyzer does
not introduce a second orthogonality tolerance. It reports axis norms, pairwise
dots, determinant, and condition number in binary64 without hiding residuals.

| Frame | direction | up_reference | Mathematical target axes X; Y; Z |
| --- | --- | --- | --- |
| Cyclic | (1,0,0) | (0,0,1) | (0,1,0); (0,0,1); (1,0,0) |
| Oblique | (4,-4,-7) | (8,1,4) | (1,8,-4)/9; (8,1,4)/9; (4,-4,-7)/9 |

Both are proper rotations in real arithmetic. Cyclic packing must be exact.
The oblique target is descriptive: analysis always uses the actual packed axes.
Requested direction/up are separate in requested_frame; the requested center
is in requested_center_mm. Central Units exports position_display_unit_pm for
plots and simple fixture checks. Python has no local unit conversion constant.

## Eight cases

Every invocation uses one Analytic CountDriven Gamma Source, Philox, Mono 511 keV
(511000000000 micro-eV), and static time exactly 0 ps. Seed defaults to
77777. Cases and dimensions belong in cases.py. "Translated" below means center
(12,-7,25) mm. Bounded Isotropic requests theta 20..60 and phi -45..90 degrees.

| Case | Geometry and pose | Angular mode | Main measurement |
| --- | --- | --- | --- |
| G2_translated_point | Point, translated/oblique | Fixed | Every position exactly equals the actual center |
| G2_box_signed_permutation | Box 24 x 16 x 10 mm, origin/identity reference and translated/cyclic run | Fixed | Exact paired integer positions; recovered Box law |
| G2_rectangle_oblique | Rectangle 40 x 20 mm, translated/oblique | Fixed | Recovered Rectangle law, plane residuals, global projections |
| A2_fixed_oblique | Point, translated/oblique | Fixed | Every component exactly equals packed axis_z |
| A2_full_sphere_frame_invariant | Point at origin, identity/oblique pair | Canonical no-bounds Isotropic | Exact global direction invariance; A1 diagnostics on reference |
| A2_bounded_signed_permutation | Point at origin, identity/cyclic pair | Bounded Isotropic | Exact component permutation; recovered bounded law |
| A2_bounded_oblique | Point, translated/oblique | Bounded Isotropic | Recovered equal-solid-angle coordinates and scale/norm diagnostics |
| A2_focused_transformed_rectangle | Rectangle 40 x 20 mm, translated/oblique | Focused | Aim from actual global position to global focus (75,-40,140) mm; recovered Rectangle law |

The angular contracts deliberately differ:

- Fixed returns stored axis_z without another normalization.
- Canonical full-sphere Isotropic uses the special **global** branch and bypasses
  the frame. Rotating the Source must leave corresponding directions identical.
- Bounded Isotropic samples locally, multiplies by the frame, then calls OpenCL
  normalize on the global vector, including when the frame is identity.
- Focused uses the already committed **global integer** primary position and
  one **global integer** focus. It does not aim from an ideal local point.

## Paired replay and its limits

The three paired cases require **one worker and one device**. The runner prints
and applies workers=1 for both members. The --workers option controls unpaired
cases. Actual device selection is checked through exporter metadata; the runner
never duplicates GGEMS selector parsing.

GGEMS keeps one random stream per worker and schedules primaries dynamically.
Equal seeds and worker counts therefore do not guarantee per-primary replay
with multiple workers. One worker on one device processes consecutive local
IDs using the same stream in both captures. Each paired case consumes one
Philox block per primary, so the streams remain aligned.

CSV records are indexed by verified source_local_primary_id, never paired by
append/row order. Each run requires slot 0, unique complete local/global IDs
[0,N), and fresh Run provenance. Pairs additionally check provenance arrays,
seed, workers, actual devices, counts, geometry, packed bounds, energy and time.

Expected Box positions use only Python integer arithmetic:

    expected = center_pm + (z_reference_pm, x_reference_pm, y_reference_pm)

All component differences must be zero. Full-sphere directions must match
exactly. Bounded cyclic directions must equal (dz_reference, dx_reference,
dy_reference) after CSV binary32 reconstruction.

The bounded branch normalizes after frame multiplication. Its cyclic pair
requires exact component permutation and reports a mismatch as an error.
This comparison applies within the selected runtime; it does not establish
bitwise equivalence across different devices or compilers.

## Analytical and numerical definitions

Positions remain decimal int64 pm. Center/focus subtraction uses Python integers,
with a check that the resulting displacement remains in binary64's exact integer
domain. Only then does NumPy perform binary64 analysis:

    delta_pm = exact_integer(global_position_pm - actual_center_pm)
    local_estimate_pm = inverse(actual_M) @ binary64(delta_pm)

The inverse is not replaced by a transpose: packed oblique axes are slightly
non-orthogonal. For each recovered Rectangle/Box width W, u = local/W + 0.5.
Measurements are min/max, mean, population variance (ddof=0), empirical CDF
maximum deviation from continuous Uniform[0,1], and Pearson correlations.
The ECDF calculation examines both sides of empirical jumps and the reference
tails; ties and support excursions are retained. Ideal mean is 0.5 and variance
is 1/12.

Rectangle reports signed local-Z statistics, mean/maximum absolute residual,
and nonzero count. Lateral excursions and zero-thickness ideal-plane excursions
are reported separately. Nonzero plane residuals are measured, not immediately
labeled bugs. The executed path includes binary32 local displacement and frame
arithmetic, then half-away-from-zero integer-pm commitment of the displacement
before exact center addition. Inverse analysis adds binary64 reconstruction
error. No point is clipped, projected back onto a plane, or repaired. Repeated
committed positions are counted and are expected for Point.

For bounded directions, recover local_raw = inverse(M) @ global_direction,
record its norm/scale, and normalize it in binary64. Then:

    u_cos = (local_unit_z - packed_cos_lower) / (packed_cos_upper - packed_cos_lower)
    u_phi = (atan2(local_unit_y, local_unit_x) - packed_phi_min)
            / (packed_phi_max - packed_phi_min)

The actual packed sector lies strictly inside [-pi,pi]; no wrapping or clipping
is needed. Undefined azimuths are excluded only from phi statistics and phi
correlations, with an explicit exclusion count. Full-sphere descriptive checks
use the same A1 global definitions on observed directions: u_cos=(dz+1)/2,
u_phi=(atan2(dy,dx) mod 2*pi)/(2*pi), Cartesian first/second and cross moments.
These measurements never replace the exact paired comparison.

Focused normalizes exact integer focus-minus-position in binary64. It normalizes
the observed direction for comparison and reports its original norm separately.
Angular error is atan2(norm(cross), dot), without acos clipping. Miss distance is
norm(cross(displacement, observed_unit_direction)), the perpendicular distance
to the forward ray. Nonforward rays and undefined directions are failures.
Output includes minimum unit dot, mean/maximum angular error, and
mean/median/p95/maximum miss distance.

Every summary.json contains structural status, full executed metadata, requested
and actual frame information, diagnostics, measurements, reference/pair
information when applicable, and figure paths. acceptance_thresholds remains
null. Missing/duplicate provenance, malformed input, overflow, exact contract
violations, and nonforward Focused directions stop execution. Statistical
deviations and oblique residuals have no arbitrary p-value or epsilon threshold.

## Build, run, and artifacts

See the [shared build and prerequisites](../README.md#build-and-prerequisites)
for Windows paths, build configurations, and Python dependencies.

Build the existing ggems_source_sample_exporter target with the configured
project toolchain:

    cmake --build build --target ggems_source_sample_exporter
    python validation/source/frame/run_campaign.py --exporter build/validation/source/ggems_source_sample_exporter --device 0 --exact-primaries 256 --statistical-primaries 1024 --workers 64 --seed 77777 --output-dir validation/source/results/frame/example

Use the executable suffix/location appropriate to the platform and configuration.
--device is mandatory and forwarded unchanged. --cases accepts any of the eight
complete names. Defaults are 256 primaries per exact case, 1024 per descriptive
oblique case, and 64 workers for unpaired runs. The current Run constructs an
expensive Observer dump, so check runtime and memory use before increasing the
population.

Python 3.12+, NumPy, and Matplotlib support the complete campaign. No SciPy or new
dependencies are installed automatically. MPLBACKEND=Agg supports headless use.
--no-plots explicitly skips figures; otherwise missing Matplotlib is an error.

Each new case directory contains transformed/samples.csv, metadata.json and
export.log in that same subdirectory. Paired cases also have reference/ files.
The case-level summary.json is written only after validation and requested
figures complete. Existing summaries/case directories are rejected to exclude
stale evidence after a failed rerun.

The three descriptive cases produce frame.png and frame.pdf:

- Rectangle: global XY/XZ/YZ projections, recovered local XY density, normalized
  marginals and ECDFs. The plane is visible without relying on 3D perspective.
- Bounded oblique: recovered equal-solid-angle density, marginals and ECDFs.
- Focused: recovered local XY density, miss-distance histogram and angular-error CDF.

Exact cases report their comparisons in JSON without figures.
Generated CSV, JSON, PNG/PDF figures, and logs go under the results directory
or the selected `--output-dir`.

Standalone paired reanalysis:

    python validation/source/frame/analyze.py --samples validation/source/results/frame/example/G2_box_signed_permutation/transformed/samples.csv --metadata validation/source/results/frame/example/G2_box_signed_permutation/transformed/metadata.json --reference-samples validation/source/results/frame/example/G2_box_signed_permutation/reference/samples.csv --reference-metadata validation/source/results/frame/example/G2_box_signed_permutation/reference/metadata.json --output-dir reanalysis/G2_box_signed_permutation

Exporter pose flags:

    --center-x-mm 12 --center-y-mm -7 --center-z-mm 25
    --frame-direction-x 4 --frame-direction-y -4 --frame-direction-z -7
    --frame-up-x 8 --frame-up-y 1 --frame-up-z 4

Supply all three center components together and all six direction/up components
together. With neither group, origin and default identity remain unchanged.
GGEMS checks orientation validity before OpenCL setup. Point cases compare
each position with the actual snapshot center.

## Scope

G2/A2 covers frame translation and rotation, exact paired transformations,
and descriptive oblique geometry and angle measurements. Statistical and
oblique residuals are reported without an automatic acceptance threshold.
ActivityDriven/radionuclide emission, realistic 120 kVp spectra, multi-source
configurations, physical transport, and combined position/angle/energy sampling
are outside this campaign. See [I1](../integration/README.md) for the combined
sampling cases.
