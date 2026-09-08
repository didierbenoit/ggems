# Source Integration: I1

I1 closes the current analytical CountDriven Source checkpoint by exercising
position, angle, energy, and frame together. G1, A1, E1, T1, and G2/A2 establish
the separate responsibilities; I1 measures their composition through the same
production OpenCL primary-initialization path.

The authority remains public GGEMSSource configuration -> immutable Source run
snapshot -> ordinary CountDriven GGEMSRun -> raw Observer Source records ->
the existing ggems_source_sample_exporter -> exact/analytical Python analysis.
The exporter is unchanged. There is no replacement Source sampler, Python
Philox implementation, raw RNG exporter, or hidden-state reconstruction.

## Cases and invocation

Every run uses one Analytic CountDriven Gamma Source, static time exactly 0 ps,
weight exactly 1, and Philox. Common center is (12,-7,25) mm. Orientation uses
the existing public SetOrientation request already tested in G2/A2:
direction (4,-4,-7), up_reference (8,1,4). Actual packed binary32 axes and the
exact integer-pm center are read from exporter metadata.

RegularSpectrum reuses E1: centers 25,35,45,55 keV, full bin width 10 keV,
relative weights 1,1,2,4. DiscreteLines reuses E1: energies 20,40,60,80 keV,
weights 1,0,1,2. Both retain the current uint64 meV representation.
Bounded Isotropic A requests theta 20..60 degrees, phi -45..90 degrees;
B requests theta 10..35 degrees, phi 20..160 degrees.

| Case | Reference / comparison | Required exact equality |
| --- | --- | --- |
| I1_integrated_oblique | Rectangle 40 x 20 mm + bounded A + RegularSpectrum | Structural fields; distributions are descriptive |
| I1_pair_geometry_same_draw_count | Rectangle 40 x 20 mm / Box 40 x 20 x 10 mm; bounded A + RegularSpectrum | Directions, energies, times |
| I1_pair_angle_same_draw_count | Rectangle + bounded A / bounded B; RegularSpectrum | Positions, energies, times |
| I1_pair_draw_owner_swap | Rectangle + Fixed / Point + bounded A; RegularSpectrum | Energies, times |
| I1_pair_energy_configuration_order | Rectangle + bounded A; RegularSpectrum / DiscreteLines | Positions, directions, times |

Pose, seed, source slot, count, and chronology are identical within each pair.
The fields deliberately changed by a pair are not required to match.
The principal run defaults to 8192 primaries and 64 workers; every pair defaults
to 2048 primaries per member and **exactly one worker**. Seed defaults to 20260908.
cases.py owns these definitions. The runner permits count/seed overrides and
an integrated worker override; it deliberately offers no multi-worker pair mode.

Build only the existing exporter with the configured GGEMS toolchain. For a
patch workflow, execute these commands from a scratch mirror:

    cmake --build build --target ggems_source_sample_exporter
    python validation/source/integration/run_campaign.py --exporter build/validation/source/ggems_source_sample_exporter.exe --device 0 --integrated-primaries 8192 --pair-primaries 2048 --workers 64 --seed 20260908 --output-dir validation/source/results/integration/smoke

Use the executable suffix/location appropriate to the platform and generator.
--device is required and forwarded unchanged to GGEMSOpenCL::SelectDevices.
Pairs also require exactly one actual selected device, checked in returned
metadata without parsing the selector in Python. --cases selects complete names.
No device marketing name is hard-coded. NumPy and Matplotlib must already be
installed with Python 3.12+; nothing is installed automatically.

The integrated directory contains samples.csv, metadata.json, export.log,
summary.json, integration.png, and integration.pdf. Pair directories contain
reference/ and comparison/ captures plus one summary.json. Exact pairs need no
figure. --no-plots explicitly disables the integrated figure; otherwise a
missing plotting dependency is an error. MPLBACKEND=Agg supports headless use.
Case directories must be new, preventing stale evidence after a failed rerun.
Generated files stay in the already ignored results area or caller-selected
scratch directories, never in the source patch.

Standalone reanalysis uses a new output directory:

    python validation/source/integration/analyze.py --case I1_integrated_oblique --case-dir validation/source/results/integration/smoke/I1_integrated_oblique --output-dir codex_scratch/i1_reanalysis
    python validation/source/integration/analyze.py --case I1_pair_draw_owner_swap --case-dir validation/source/results/integration/smoke/I1_pair_draw_owner_swap --output-dir codex_scratch/i1_pair_reanalysis

## Production draw ownership and exact-pair scope

The current code, checked against the September 7 Source audit and the
September 5 Random validation report, establishes:

| Source responsibility | Calls per primary | Philox blocks |
| --- | --- | --- |
| CountDriven birth time, source lookup, center/frame | None | 0 |
| Point position | None | 0 |
| Rectangle or Box position | One Uniform4 | 1 |
| Fixed or Focused angle | None | 0 |
| Isotropic angle | One Uniform4 | 1 |
| Mono energy | None | 0 |
| DiscreteLines or RegularSpectrum energy | One raw uint32 | 1 |

GGEMS_SourceInitializePrimary in kernels/sources/GGEMSSource.clh calls time,
position, direction, then energy. Isotropic receives its own Uniform4 after
position sampling. Unused lanes are discarded. GGEMS_EnergyDistributionSample
in kernels/sources/GGEMSEnergyDistribution.clh draws one raw word for either
tabulated mapping. The current diagnostic projection consumes no additional
random values. The scalar path in kernels/random/GGEMSPhilox.clh calls
GGEMS_PhiloxNextUInt4, returns X, and discards Y/Z/W without caching them.
Uniform4 consumes all four lanes of one fresh block.

Thus geometry and angle same-budget pairs each consume three blocks per primary.
The owner-swap pair consumes two: one position-or-angle block, then the energy
block. The energy-configuration pair consumes three in both members. Equal total
budgets preserve stream alignment for the next primary as well as this one.
Changing energy mapping cannot alter earlier position/direction draws.

src/random/GGEMSRandom.cc derives Philox keys from SplitMix64(seed), starts
the low counter at zero, and places stream ID in the high words.
src/GGEMSRun.cc assigns context_index * worker_count stream offsets;
GGEMSTransportWorkload owns persistent worker states initialized once.
particle_stream_transport.cl assigns primaries through an atomic cursor and
indexes RNG state by worker ID. Primary IDs do not select RNG streams.

Independent multi-worker runs therefore do not guarantee per-primary replay.
One worker on one device uses stream 0, processes consecutive local IDs, and
fresh exporter processes reproduce that configuration. Padded work-items return
before fetching primaries. Pairing indexes the verified source_local_primary_id;
it ignores CSV row order and also verifies global-primary compatibility.
Seed, actual devices, workers, counts, provenance, pose, and unchanged packed
sampler configuration must agree. No cross-device or general scheduling
reproducibility is claimed.

These observable equalities check the documented draw budget and ordering; they
are not another RNG qualification, an independent proof of every internal RNG
operation, or a statistical independence proof.

## Structural and analytical definitions

The unchanged CSV retains exact decimal int64 pm positions, uint64 meV energies
and uint64 ps times. Directions are reconstructed as binary32 from the
max_digits10 CSV decimals before binary64 analysis. The exporter validates Gamma
on every raw Source record; Python verifies Gamma metadata because the current
CSV has no particle column. Source-only rows, slot 0, complete unique local/global
IDs [0,N), zero overflow, expected counts/capacities, exact 0 ps, unit weight,
and finite nonzero directions are mandatory. Point and Fixed members also retain
their exact center/axis_z contracts.

The analyzer rejects malformed frame shape, nonfinite/non-binary32 values,
singular or left-handed frames, and a wrong matrix-convention declaration.
The actual axes are columns of M. Norms, dots, determinant, handedness and
condition number are reported; there is no invented orthogonality tolerance.
Requested orientation remains separate from execution axes.

Position recovery subtracts the exact center using Python integers before
conversion to binary64, then applies inverse(M), never transpose(M).
For the Rectangle, u_x = local_x / width + 0.5 and
u_y = local_y / height + 0.5. Lateral support excesses are reported separately
from the reconstructed plane residual. Local-Z reports signed statistics and
mean, median, p95 and maximum absolute pm residuals. Binary32 displacement/frame
arithmetic, half-away-from-zero pm commitment before exact center addition, and
inverse reconstruction explain why stored integer pm does not mean exact local
planarity after an oblique transform.

Angular recovery applies inverse(M) to the observed global direction and
normalizes the result in binary64, retaining global and raw-local norm diagnostics.
Using the actual packed bounds:

    u_cos = (local_unit_z - cos_lower) / (cos_upper - cos_lower)
    u_phi = (atan2(local_unit_y, local_unit_x) - phi_min) / (phi_max - phi_min)

Both selected sectors lie within atan2's nonwrapping interval. Undefined phi
is excluded only from phi-related measurements, with explicit counts.
For position and angle, the ideal reference is continuous Uniform[0,1]:
mean 1/2, population variance 1/12. ECDF D examines both sides of empirical
jumps and reference tails; observations are never clipped or repaired.
Support excursions and numerical residuals are measurements, not automatic
statistical failures.

Energy uses exactly the E1 finite definition, copied locally without changing E1.
Actual packed centers, even full width W, and cumulative uint32-ticket bounds
are mandatory. Regular bins are half-open, lower = center - W//2. Every energy
must belong to exactly one bin and be reachable. For bin ticket count M:

    offset = floor(W*t/M),  0 <= t < M
    C(k) = 0                                  for k < 0
    C(k) = M                                  for k >= W - 1
    C(k) = (M*(k+1) + W - 1) // W             otherwise
    F(k) = C(k) / M

C is an analytical count, not generated Monte Carlo or an inferred raw ticket.
An observed k with C(k) == C(k-1) is unreachable and fails. Conditional ECDF D
groups ties and compares empirical before/after jumps to F(k-1)/F(k) using
exact integer cross products before final binary64 reporting. Empty bins have
null conditional D. Probability residuals, maximum absolute bin error,
total variation and cumulative-bin maximum deviation use Fraction arithmetic.
The four actual ticket masses must be 1/8,1/8,1/4,1/2; the DiscreteLines member
also validates E1's zero-ticket line exclusion.

The integrated summary contains the u_x/u_y/u_cos/u_phi Pearson matrix on common
defined-phi rows and conditional means per energy bin. Per-bin counts preserve
phi exclusions; position/cosine means still use all applicable rows. These are
descriptive coupling diagnostics. Small correlations do not prove independence.
The four-panel figure shows local XY, equal-solid-angle coordinates, energy,
and correlations. Its constant energy density is exact bin mass divided by width;
the finite integer law is evaluated by the separate CDF, not a continuous fit.

Every case has acceptance_thresholds = null. Exact-pair violations, impossible
provenance, malformed captures, energy support/reachability violations and
fixed-field violations fail immediately. No p-values or arbitrary epsilons
weaken those exact contracts.

## Checkpoint and review

The implementation smoke uses 8192/64 for the integrated case and 2048/1 for each
pair member, seed 20260908. It is a compact integration checkpoint; retained
150000-primary G1/A1/E1/G2-A2 results remain the higher-statistics reference.
Statistical/numerical acceptance budgets, uncertainty presentation, broader
device/compiler coverage, and later performant Output reruns remain separate.
The approved future meV -> ueV migration is not implemented here and will
require focused E1/I1 reruns after its dedicated code change.

Scratch-only analysis probes exercise malformed CSV/frame/provenance, support
excursions, unreachable finite energies, wrong time/weight, corrupted exact
pair fields, provenance-domain mismatch, and reordered CSVs. Tiny M/W counting
examples check the finite law, including unreachable offsets when W > M.
They do not simulate Source or Philox. Execute one unchanged G1, A1, E1, T1,
and G2/A2 case against the existing exporter.

    python -m py_compile validation/source/integration/cases.py validation/source/integration/run_campaign.py validation/source/integration/analyze.py validation/source/integration/plot.py
    ruff check --no-cache validation/source/integration/
    ruff format --check --no-cache validation/source/integration/
    basedpyright --pythonpath <project-python> validation/source/integration/

No C++ file changes, so I1 needs no new clangd/clang-format pass.
Review all Python diagnostics; NumPy shape/scalar overloads and Matplotlib
keyword stubs can produce warnings without an actionable implementation defect.
Do not suppress them merely to reach zero warnings.

I1 validates analytical CountDriven primary generation within the tested scope.
ActivityDriven/radionuclides, realistic 120 kVp, multi-source campaigns,
Voxelized/PhaseSpace Source, physical transport, Navigation, and clinical
accuracy are outside this checkpoint.
