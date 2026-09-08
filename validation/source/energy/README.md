# Source / Energy: E1 canonical energy validation

E1 measures the energies emitted by the current production CountDriven Source.
The authority is `GGEMSSource` configuration -> ordinary `GGEMSRun` -> production
OpenCL primary initialization -> raw Observer `Source` records -> the shared
validation exporter -> exact Python analytical measurements -> Matplotlib.
There is no alternative Monte Carlo sampler, ticket allocator, or production API.

All three cases use one Analytic CountDriven Gamma source, Point at the exact
origin, default identity frame, Fixed +Z, static time 0 ps, weight 1, and Philox.
Position, direction, time, and source lookup consume no random draw. Mono uses
no energy draw; each tabulated mode consumes one raw uint32 energy ticket.
Those draw budgets are established by the production helpers and their focused
tests, not inferred from a histogram. The independently qualified Philox engine
is used within its tested Random scope; E1 measures its energy transformations.

| Case | Configuration in keV | Relative weights | Exact expected probabilities |
|---|---|---|---|
| `E1_mono` | 511 | None | Exact equality to 511000000 meV |
| `E1_discrete_lines` | Lines 20, 40, 60, 80 | 1, 0, 1, 2 | 1/4, 0, 1/4, 1/2 |
| `E1_regular_spectrum` | Centers 25, 35, 45, 55; full width 10 | 1, 1, 2, 4 | 1/8, 1/8, 1/4, 1/2 |

The probabilities in this table are checked against the actual packed ticket
intervals before analysis. They are not substituted for missing metadata.
The zero-weight 40 keV line remains a configured entry with zero ticket width.
RegularSpectrum has bins [20,30), [30,40), [40,50), [50,60) keV. Its definition
is center-based and piecewise constant, with integer emission within each bin;
it does not interpolate between spectrum centers.

## Build and run

Use the existing GGEMS toolchain and installed dependencies. The executable is
still `ggems_source_sample_exporter`; E1 needs no additional CMake option or
target. Configure an out-of-source build with the local dependency settings:

```console
cmake -S . -B codex_scratch/build
cmake --build codex_scratch/build --target ggems_source_sample_exporter
python validation/source/energy/run_campaign.py --exporter codex_scratch/build/validation/source/ggems_source_sample_exporter --device 0 --primaries 4096 --workers 4096 --seed 77777 --output-dir codex_scratch/e1_smoke
```

Use `.exe` on Windows and the configuration subdirectory for multi-configuration
generators. Python 3.12+ is required. The exact analysis uses the standard
library, particularly Python integers, `Fraction`, and `Counter`. Matplotlib
and its NumPy dependency generate figures; SciPy is unnecessary. No packages
are installed automatically. `--no-plots` explicitly skips figures; otherwise
a missing plotting dependency is an error. `MPLBACKEND=Agg` supports headless use.

`--device` is required and is passed unchanged to `GGEMSOpenCL::SelectDevices()`.
Actual device names are recorded in selection order. `--cases` accepts any of
the three complete case names. `cases.py` owns the configurations and modest
development defaults: 4,096 primaries, 4,096 workers, seed 77,777. These defaults
are implementation checks, not publication statistics.

Outputs default to the ignored `validation/source/results/energy/`, or to a
caller-selected `--output-dir`. Each case directory must be new, so a failed
rerun cannot leave stale scientific results. Successful cases contain
`samples.csv`, `metadata.json`, `export.log`, and `summary.json`. The two
statistical cases also produce `energy.png` and `energy.pdf` unless plots were
explicitly disabled. Mono needs only an exact deterministic summary. Generated
CSV, JSON, logs, and figures do not belong in the code patch.

Standalone tabulated extraction and analysis:

```console
codex_scratch/build/validation/source/ggems_source_sample_exporter --device 0 --geometry point --angular fixed --case-name E1_discrete_lines --energy-mode discrete-lines --energy-values-kev 20,40,60,80 --energy-weights 1,0,1,2 --primaries 4096 --workers 4096 --seed 77777 --output samples.csv --metadata metadata.json
python validation/source/energy/analyze.py --samples samples.csv --metadata metadata.json --output-dir analysis
```

The energy CLI is intentionally small:

- `--energy-mode mono` accepts optional `--mono-energy-kev`, default 511.
- `--energy-mode discrete-lines` requires `--energy-values-kev` and
  `--energy-weights` lists.
- `--energy-mode regular-spectrum` requires those lists, interpreted as centers
  and bin weights, plus `--energy-bin-width-kev` for the full bin width.

Lists have matching counts of at least two, strict comma separators, no empty
fields, no whitespace, and finite nonnegative numbers. The real Source builder
requires positive canonical energies, strictly increasing order, valid weights,
and a valid regular grid. Invalid input is rejected before OpenCL setup or
output creation; input is never sorted. Parameters belonging to another energy
mode are rejected. Units conversion remains in GGEMS.

The public RegularSpectrum API derives its canonical width from center spacing.
The exporter converts the requested CLI width with central Units and requires
exact agreement with that derived width. It neither constructs a second grid
nor introduces a production API. An explicit `--case-name` is needed for
standalone E1 analysis; the historical exporter default is `G1_<geometry>`.

## Executed metadata and extraction contract

The exporter reads `GGEMSRun::GetLastSourceRunSnapshot()` after the run. Its
immutable configuration supplies the actual distribution descriptor and arrays
uploaded to OpenCL: `distribution_type`, `table_offset`, `table_count`, exact
`energy_values_meV`, `regular_bin_width_meV`, `relative_weights`, and
`cumulative_ticket_upper_bounds`. These are exposed in the metadata `energy`
object with `ticket_space_size = 2^32` and `representation = "uint64 meV"`.
The offset is an element index shared by the flattened energy and ticket arrays,
not a byte offset. This single-source exporter requires offset zero and exports
the complete arrays. Relative weights are descriptive host state; cumulative
ticket bounds are the executed probability authority.

Mono uses `mono_energy_meV` and empty arrays with zero table count, offset, and
width. For tabulated modes `mono_energy_meV` and the retained top-level
`energy_meV` are zero, reflecting the inactive Source-record Mono field; they
are not a sampled energy or an average. `energy_mode` is the current Source
display name (`Mono`, `Discrete lines`, `Regular spectrum`), while
`energy_configuration` retains the CLI spelling. `requested_energy` separately
records the requested keV values, width, and weights. `display_unit_meV` comes
from central GGEMS Units and is used only for plotting/display conversion.

For these cases, the packed cumulative bounds must be:

| Mode | Cumulative upper bounds | Ticket widths |
|---|---|---|
| DiscreteLines | 1073741824, 1073741824, 2147483648, 4294967296 | 1073741824, 0, 1073741824, 2147483648 |
| RegularSpectrum | 536870912, 1073741824, 2147483648, 4294967296 | 536870912, 536870912, 1073741824, 2147483648 |

The analyzer checks table counts, canonical fixtures, ordering, monotone bounds,
final bound 2^32, and each exact rational ticket fraction. It does not duplicate
the host largest-remainder allocator. A raw ticket equal to a cumulative upper
bound belongs to the next nonempty interval; zero-width entries are skipped.

The stable CSV is unchanged:

```text
source_index,source_local_primary_id,global_primary_id,x_pm,y_pm,z_pm,direction_x,direction_y,direction_z,energy_meV,time_ps,weight,record_kind
```

Integer fields remain exact decimal integers. Floating serialization retains
`numeric_limits<float>::max_digits10`. Canonical +Z is exactly representable.
Only `Source` records enter scientific analysis; diagnostic `Terminal` records
and the human-readable Observer dump are not samples.

The exporter verifies raw per-record particle type as Gamma before writing CSV,
whose existing schema has no particle column. Python checks Gamma metadata,
zero overflow, exact Source/Terminal capture counts and capacities, Source-only
rows, complete unique ordered provenance, slot 0 and local/global IDs [0,N),
Point origin, exact Fixed +Z, static 0 ps, and unit weight. Malformed CSV,
metadata mismatch, or any structural contract failure stops analysis.

Existing Geometry and Angle CLI behavior is preserved: absent energy options
still configure Mono 511 keV, and all old metadata fields retain their values.
G1 and A1 scripts do not need changes. The runner records the checkout commit
through the same lightweight `git rev-parse HEAD` path when available. There
are no hashes, driver fingerprints, or qualification certificates.

## Exact energy analysis

Mono requires exact integer equality for every sample and reports count,
expected/minimum/maximum energy, mismatch count, and maximum absolute integer
difference. A mismatch is an immediate contract failure.

DiscreteLines requires exact configured-line membership and prohibits every
zero-ticket line. Each line reports its canonical energy and display keV,
ticket count, exact probability as a rational string and binary64 display,
observed count/probability, residual, and absolute error. All lines belong to
one categorical experiment; their counts are not treated as independent
binomial experiments. Global metrics are maximum absolute probability error,
total variation distance (half the sum of absolute residuals), and the maximum
absolute cumulative categorical residual in energy order.

RegularSpectrum derives integer edges as `lower = center - W//2`,
`upper = lower + W`, after requiring even positive width. Membership uses
integer comparisons `lower <= energy < upper`. A shared boundary belongs to
the next bin, and the last upper edge is excluded. Every sample must belong
to exactly one bin. Per-bin counts and global probability metrics use the
same actual ticket widths as DiscreteLines.

For within-bin analysis, let `M` be that bin's actual ticket width and `W` its
integer meV width. The kernel maps its local ticket `t` in [0,M) to
`offset = floor(W*t/M)`. The reference derives a counting CDF from this mapping;
it does not generate tickets or copy the overflow-safe OpenCL implementation.
For integer offset `k`, the exact number of tickets with emitted offset <= k is:

```text
C(k) = 0                                  if k < 0
C(k) = M                                  if k >= W - 1
C(k) = ceil(M*(k+1)/W)
     = (M*(k+1) + W - 1) // W             otherwise
F(k) = C(k) / M
```

This follows from `floor(W*t/M) <= k` iff `t < M*(k+1)/W`. All products and
division in `C` use Python arbitrary-precision integers. No enumeration of M
tickets is needed. It also handles W > M, where some integer offsets are
unattainable; emitting such an offset is an exact support failure.

The maximum empirical-versus-discrete-CDF deviation groups repeated offsets.
For each observed distinct k, compare empirical probability before its complete
jump with `F(k-1)` and probability after its jump with `F(k)`. Between observed
offsets the empirical CDF is constant and F is monotone, so those endpoints also
cover unobserved reference jumps and the tails. Subtract exact integer cross
products over denominator `N_bin*M`, then convert the final ratio for reporting.
This is not the continuous Uniform ECDF formula. Per-bin results include count,
distinct energies, offset min/max, and exact-finite CDF maximum deviation.
An unobserved bin has null offset metrics and CDF deviation, never a fabricated
zero agreement measurement.

Every case reports distinct emitted energies, repeated samples/energies,
maximum multiplicity, integer extrema, and exact support status. Repetition on
the finite integer lattice is expected. No sample is clipped or repaired.
Statistical residuals are descriptive; no p-values or arbitrary PASS/FAIL
thresholds are produced. `summary.json` contains metadata, representation notes,
structural status, case metrics, figure paths/status, and
`acceptance_thresholds = null`. Successful exact support diagnostics are zero;
violations stop execution with the offending contract identified.

## Figures and review scope

DiscreteLines shows observed versus exact expected probability and residuals,
including the configured 40 keV zero-weight line. RegularSpectrum shows a fine
energy histogram, constant density from exact bin mass divided by bin width,
observed/expected bin probabilities, residuals, and conditional finite-CDF
deviation by bin. The constant density is the bin-scale spectrum representation;
the exact integer sub-bin law is assessed separately by the finite CDF. Both
figures are saved as PNG and PDF without a statistical acceptance line.

For implementation review, run all three E1 cases and one unchanged G1 and A1
campaign with the modified exporter. Also check the analysis in disposable
probes: malformed CSV, off-line and zero-ticket emission, malformed ticket
bounds, exact lower/shared/final upper edges, samples outside the bins, repeated
offsets, and metadata mismatch. Tiny M/W examples may enumerate all tickets in
those probes to independently verify the analytical CDF and both jump limits.

Run the available quality tools on the candidate:

```console
python -m py_compile validation/source/energy/cases.py validation/source/energy/run_campaign.py validation/source/energy/analyze.py validation/source/energy/plot.py
ruff check --no-cache validation/source/energy/
ruff format --check validation/source/energy/
basedpyright --pythonpath <project-python> validation/source/energy/
python clangd-check.py validation/source/tools/GGEMSSourceSampleExporter.cc
clang-format --dry-run --Werror validation/source/tools/GGEMSSourceSampleExporter.cc
```

Direct-script imports retain the existing narrow BasedPyright annotation for
that import mode. Matplotlib's partially typed APIs can emit third-party type
warnings; inspect them without hiding them to force a zero-warning result.

The current Observer capture requires `2*N <= UINT32_MAX`. The ordinary Run
still constructs its dump, including costly per-history rescans, even when
logging is filtered; practical full captures are much smaller. Worker/device
assignment and scheduling may change which persistent stream supplies a
primary. Sorted provenance is not a bitwise per-primary reproducibility promise.

Scientific sample counts, uncertainty reporting, accepted statistical error
budgets, device coverage, and publication criteria remain open. E1 stops at
these canonical energy configurations. Realistic 120 kVp spectra, Time,
G2/A2 frames/rotations, ActivityDriven/radionuclides, physical transport, and
external Monte Carlo comparisons are outside this patch.
