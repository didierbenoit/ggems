# Source / Time — T1 CountDriven chronology validation

T1 observes exact primary birth times from the production GGEMS OpenCL Source
path. It validates the propagation of the Run chronology into the committed
Source snapshot and then into each raw Observer Source record.

For CountDriven, the executable law is exactly:

```text
birth_time_ps = committed_source_record.time_start_ps
```

`time_stop_ps` is a chronology boundary. It is not the upper bound of a random
CountDriven birth-time distribution. Every birth is concentrated at the included
start of the configured half-open window. No birth near its stop is required.

T1 has no statistical goodness-of-fit test, p-value, KS statistic, variance
criterion, or arbitrary acceptance tolerance. An incorrect snapshot bound or
one incorrect birth time is an immediate exact contract failure.

## Canonical cases

All cases use one CountDriven Gamma Source with Point geometry at the origin,
the default identity frame, Fixed +Z direction, Mono 511 keV (exactly
`511000000 meV`), and weight 1. Philox is configured with seed 77777 by default.
The configured Source consumes zero time, position, angular, and energy random
draws; source lookup also consumes none. T1 makes no RNG-state replay claim.

| Case | Requested configuration | Effective windows, exact ps | Exact birth times, ps |
| --- | --- | --- | --- |
| `T1_static` | Ordinary static Run; no `SetTimePicoSecond` call | `[0,0]` | `0` |
| `T1_configured_windows` | Start 10 ns, stop 65 ns, step 20 ns | `[10000,30000)`, `[30000,50000)`, `[50000,65000)` | `10000`, `30000`, `50000` |
| `T1_reset` | Start 10 ns, stop 50 ns, step 20 ns; reset before sequence index 2 | `[10000,30000)`, `[30000,50000)`, `[10000,30000)` | `10000`, `30000`, `10000` |

The last configured window is deliberately shortened to 15 ns. The explicit
expected window tables in `cases.py` are analytical fixtures for these three
cases. Python does not implement a competing Run chronology engine.

Each case uses one exporter process and exactly one initialized `GGEMSRun`.
Configured cases call `Run()` three times on that object. `T1_reset` calls
`ResetTime()` between the second and third successful Runs.

`ResetTime()` rewinds only chronology. Successful Run ids advance by one and
global primary ranges continue contiguously across the reset. Source-local ids
cover `0..N-1` separately in each Run. The analyzer checks relative progression;
it does not prescribe an absolute first Run-id number or derive global ids from
`run_id * N`.

## Production authorities

The executed path is:

```text
GGEMSRun::SetTimePicoSecond / ordinary static mode
    -> one Initialize(), sequential Run(), optional ResetTime()
    -> GGEMSSourceRunSnapshot and copied GGEMSSourceRecord time bounds
    -> production particle_stream_transport.cl
    -> GGEMS_SourceTryInitializePrimary / GGEMS_SourceSampleTimePs
    -> raw GGEMSObserverRecord of kind Source
    -> ggems_source_sample_exporter
    -> exact Python integer analysis
```

Relevant current implementation and focused evidence:

- `src/GGEMSRun.cc`: effective-window ownership, success-only clock advancement,
  snapshot publication, Observer result replacement, and clock-only reset.
- `include/GGEMS/GGEMSTimeWindow.hh`: canonical uint64 ps bounds.
- `src/sources/GGEMSSourceRunSnapshot.cc`: window injection into copied records;
  the attached Source objects are not mutated.
- `include/GGEMS/sources/GGEMSSourceRunSnapshot.hh`: owned snapshot copy,
  `GetTimeWindow()`, and `GetRecords()`.
- `kernels/sources/GGEMSSource.clh`: CountDriven returns `time_start_ps` without
  drawing randomness or using the stop to distribute births.
- `kernels/transport/particle_stream_transport.cl` and
  `kernels/observer/GGEMSObserverRecord.clh`: Source capture immediately after
  primary initialization, including exact birth time and actual provenance.
- `src/particles/GGEMSPrimaryStream.cc`: reservation of monotone global primary
  ranges independently of the Run labels.
- `tests/GGEMSRunTest.cc`: repeated static Runs, shortened configured windows,
  reset, zero-count windows, exhaustion, and last-successful snapshot behavior.
- `tests/sources/GGEMSSourceRunSnapshotTest.cc` and
  `tests/transport/GGEMSDummyTransportWorkloadTest.cc`: copied-window and
  window-start propagation evidence. T1 itself uses production transport.

Existing focused lifecycle tests explain the contract. They are not the T1
scientific result and are not duplicated as additional campaign cases.

## Exporter interface and harvesting

The existing `ggems_source_sample_exporter` executable serves G1, A1, E1, and
T1. The single-Run CSV schema remains unchanged:

```text
source_index,source_local_primary_id,global_primary_id,x_pm,y_pm,z_pm,direction_x,direction_y,direction_z,energy_meV,time_ps,weight,record_kind
```

The final `record_kind` column is the existing explicit `Source` tag. Integer
quantities remain exact decimal integers. Direction and weight serialization
retains binary32 `max_digits10` precision. The human-readable Observer dump is
never scientific input.

New CLI options are:

```text
--chronology static|configured                 # default static
--time-start-ns <value>                        # all three required if configured
--time-stop-ns <value>
--time-step-ns <value>
--sequence-dir <new CSV directory>             # replaces --output
--sequence-runs <positive uint32>              # default 1
--reset-before-run <zero-based sequence index> # optional, configured sequence only
```

`--metadata` remains required. The manifest must be outside the new sequence
CSV directory, for example `case/metadata.json` beside `case/runs/`.
The sequence directory and manifest must not already exist. `--output` and
`--sequence-dir` are mutually exclusive. A reset index must be in
`[1, sequence-runs)`. Time parameters in static mode, incomplete configured
triples, repeated flags, and invalid indices are rejected explicitly.

Start/stop use central `MakeQuantity<TimePoint>(value, "ns")`; step uses
`MakeQuantity<Duration>(value, "ns")`. `GGEMSRun::SetTimePicoSecond` validates the
canonical configured interval. GGEMS alone calculates effective windows and
advances the clock. There is no Source time setter or local C++ conversion
factor.

The exporter harvests each successful logical Run immediately:

1. Take the owned copy returned by `GetLastSourceRunSnapshot()`.
2. Verify its window equals the copied Source-record bounds.
3. Check Observer counts and select raw Source records; verify actual particle
   and Run ids on every selected raw record.
4. Check complete per-Run provenance and continuation from the preceding Run.
5. Write `run_000.csv`, `run_001.csv`, or `run_002.csv`, then serialize that
   Run's metadata before the next `Run()` call replaces the public result.

Only pointers into the current Observer result are used during that iteration;
none survive into the next Run. The exporter does not substitute subsequently
mutated live Source state for the committed snapshot. A failed extraction does
not produce a valid complete sequence manifest; the campaign runner stops.

Without time-specific options, the existing single-Run static behavior, sample
schema, defaults, device-selector delegation, and G1/A1/E1 configuration remain
compatible. Extra metadata fields are additive.

## Metadata and exact analysis

The sequence manifest `metadata.json` separates three authorities:

- Requested configuration: `requested_time_ns`, ordered as start/stop/step;
  null for ordinary static chronology.
- Central Units conversion: `configured_time_ps`, in the same order;
  null for ordinary static chronology.
- Execution: each ordered `runs` entry contains the actual committed Run window
  (`effective_start_ps`, `effective_stop_ps`), the actual copied Source bounds
  (`snapshot_time_start_ps`, `snapshot_time_stop_ps`), raw Observer Run id,
  global range, and the relative CSV path.

Common fields include `chronology_mode`, `primary_count_per_run`,
`sequence_length`, `reset_before_sequence_index`, and exact representation and
display-unit information. Each Run retains the existing Source geometry, frame,
energy, population, Philox, seed, workers, selector, actual device names, and
Observer counts/capacity metadata. The runner adds the current Git commit when
available through the existing lightweight `git rev-parse HEAD` path.

Before numerical output, the analyzer requires for every Run:

- Zero Observer overflow and exactly `N` Source records from the expected
  complete Source/Terminal capture (`2*N` raw Observer records).
- Exact CSV column names/count, `Source` tags, uint64 fields, unique provenance,
  source slot zero, complete source-local ids, and a consistent global range.
- Point positions exactly zero, finite direction components exactly `(0,0,1)`,
  Mono energy exactly `511000000 meV`, and weight exactly 1.
- The expected CountDriven/Gamma/Philox/identity/Mono configuration in metadata.
  Particle type and Run id are checked on raw records by C++; those fields are
  not added to the stable CSV schema.
- Ordered, complete logical Run metadata; distinct existing CSV paths inside
  the case directory; unchanged execution configuration across the sequence.
- The exact expected snapshot bounds, exact agreement between Run-window and
  Source-record metadata, and continuing Run/global-primary provenance.
- Every observed time, parsed directly as a Python integer, equal to the
  expected committed start. No conversion to float precedes equality checks.

Physical Observer append order is irrelevant. The exporter sorts by Source
provenance for reproducible output; the analyzer also accepts a different CSV
row order when provenance remains complete. It never fills missing ids,
silently repairs records, or sorts unordered sequence metadata into validity.

Each `summary.json` includes an ordered `runs` array with snapshot differences,
birth-time min/max, distinct count, mismatch count, maximum absolute difference,
and provenance ranges. Exact differences must be zero. Reset results describe
the chronological return and continuing ids. Common fields retain the complete
metadata, representation notes, structural status, figure paths/status, and
`acceptance_thresholds: null`. No statistical population is formed by flattening
different windows.

## Build and run

Build the existing exporter target using the project's configured toolchain.
For a patch-only workflow, these commands must be run from a candidate mirror
inside `codex_scratch/`, with an out-of-source build there:

```text
cmake --build build --target ggems_source_sample_exporter
python validation/source/time/run_campaign.py --exporter build/validation/source/ggems_source_sample_exporter.exe --device 0 --primaries 256 --workers 64 --seed 77777 --output-dir validation/source/results/time/smoke
```

The executable suffix/path depends on the platform and build configuration.
`--device` is forwarded unchanged to the established GGEMS device selector.
Use `--cases T1_static`, `--cases T1_configured_windows`, or `--cases T1_reset`
to select cases. All three are the default. Each selected case directory must
be new, so an unsuccessful extraction cannot inherit old results.

For direct configured-sequence extraction:

```text
ggems_source_sample_exporter --device 0 --geometry point --angular fixed --energy-mode mono --mono-energy-kev 511 --primaries 256 --workers 64 --seed 77777 --case-name T1_configured_windows --chronology configured --time-start-ns 10 --time-stop-ns 65 --time-step-ns 20 --sequence-runs 3 --sequence-dir output/runs --metadata output/metadata.json
```

Create the parent `output/` directory first. To analyze that capture separately,
write to a new analysis directory:

```text
python validation/source/time/analyze.py --metadata output/metadata.json --output-dir output/reanalysis
```

The exact analysis uses the Python standard library. Matplotlib from the
existing project environment generates the optional configured/reset figures;
`--no-plots` avoids importing it. No NumPy or SciPy is needed. Do not install
packages automatically.

Default `256` primaries per Run and `64` workers are sufficient. Large
publication-scale counts add no meaningful information to exact time equality
and waste the temporary Observer path's storage and dump-construction work.

## Figures, checks, and scope

Configured and reset cases produce `chronology.png` and `chronology.pdf`.
Each horizontal segment is an actual effective window; a filled start marker
represents all births in that Run and an open stop marker shows its excluded
chronological endpoint. The final shortened window and the reset operation are
annotated. Static time is summary-only. No thousands of identical birth points
or time-distribution histogram are plotted.

Project checks for the candidate:

```text
python -m py_compile validation/source/time/cases.py validation/source/time/run_campaign.py validation/source/time/analyze.py validation/source/time/plot.py
ruff check --no-cache validation/source/time/
ruff format --check --no-cache validation/source/time/
basedpyright --pythonpath <existing-project-python> validation/source/time/
python clangd-check.py validation/source/tools/GGEMSSourceSampleExporter.cc
clang-format --dry-run --Werror validation/source/tools/GGEMSSourceSampleExporter.cc
```

Inspect diagnostics, including those emitted by `clangd-check.py` despite a
successful exit. The explicit CLI parser and Run harvesting may trigger
complexity heuristics. Partially typed Matplotlib APIs may produce unknown-type
warnings; do not suppress them merely to obtain a zero-warning result.

Implementation validation uses all three T1 smokes plus the unchanged G1 Point,
A1 Fixed, and E1 Mono scripts against the modified exporter. Scratch-only
analysis probes cover malformed CSV, missing Run data, duplicate provenance,
wrong snapshot start/stop, one wrong birth, multiple distinct births, the shortened
final window, reset with continuing ids, overlapping reset ranges, and unordered
sequence metadata. They also check exact integer ids beyond binary64's consecutive
integer range and reject float-form time fields.

Generated CSV, JSON, PNG, PDF, logs, probes, and caches stay in the ignored
results area or a caller-selected scratch directory. Only code and documentation
belong in the patch.

T1 does not validate ActivityDriven, radionuclides, decay-time laws, time of
flight, transport time, RNG replay, realistic 120 kVp spectra, or G2/A2 pose/frame
behavior. Multi-device and failure/exhaustion lifecycle coverage remain the
domain of the existing focused tests; the T1 smoke result is limited to the
selected device and these three successful canonical sequences. No scientific
acceptance threshold remains to choose for this exact CountDriven time law.
