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
`511000000000 micro-eV`). Philox is configured with seed 77777 by default.
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

## Sample extraction

Birth times are read from raw Source records captured immediately after
production OpenCL primary initialization. The exporter compares them with the
committed Source snapshot for each Run; Python then checks exact integer times.

## Exporter options and sequence files

The shared `ggems_source_sample_exporter` executable produces single-Run CSV
files with these fields:

```text
source_index,source_local_primary_id,global_primary_id,x_pm,y_pm,z_pm,direction_x,direction_y,direction_z,energy_micro_eV,time_ps,record_kind
```

The final `record_kind` column is the existing explicit `Source` tag. Integer
quantities remain exact decimal integers. Direction serialization
retains binary32 `max_digits10` precision. The human-readable Observer dump is
never scientific input.

Chronology options are:

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

GGEMS converts the requested nanoseconds to integer picoseconds, calculates
the effective windows, and advances the Run clock. After each successful Run,
the exporter checks the snapshot bounds, Observer counts, and complete primary
identifiers, then writes `run_000.csv`, `run_001.csv`, or `run_002.csv` and the
corresponding metadata.

A failed extraction stops the campaign without producing a valid complete
sequence manifest. Without chronology options, the exporter produces a single
static Run.

## Metadata and exact analysis

The sequence manifest `metadata.json` contains three configuration descriptions:

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
  and Mono energy exactly `511000000000 micro-eV`.
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

See the [shared build and prerequisites](../README.md#build-and-prerequisites)
for Windows paths, build configurations, and Python dependencies.

Build the existing exporter target using the project's configured toolchain:

```text
cmake --build build --target ggems_source_sample_exporter
python validation/source/time/run_campaign.py --exporter build/validation/source/ggems_source_sample_exporter --device 0 --primaries 256 --workers 64 --seed 77777 --output-dir validation/source/results/time/example
```

The executable suffix/path depends on the platform and build configuration.
`--device` is forwarded unchanged to the established GGEMS device selector.
Use `--cases T1_static`, `--cases T1_configured_windows`, or `--cases T1_reset`
to select cases. All three are the default. Each selected case directory must
be new, so an unsuccessful extraction cannot inherit old results.

For direct configured-sequence extraction:

```text
build/validation/source/ggems_source_sample_exporter --device 0 --geometry point --angular fixed --energy-mode mono --mono-energy-kev 511 --primaries 256 --workers 64 --seed 77777 --case-name T1_configured_windows --chronology configured --time-start-ns 10 --time-stop-ns 65 --time-step-ns 20 --sequence-runs 3 --sequence-dir output/runs --metadata output/metadata.json
```

Create the parent `output/` directory first. To analyze that capture separately,
write to a new analysis directory:

```text
python validation/source/time/analyze.py --metadata output/metadata.json --output-dir output/reanalysis
```

The exact analysis uses the Python standard library. Matplotlib from the
existing project environment generates the optional configured/reset figures;
`--no-plots` avoids importing Matplotlib and its NumPy dependency. SciPy is not
required.

Default `256` primaries per Run and `64` workers are sufficient. Large
sample counts add no information to exact time equality and increase capture
storage and runtime.

## Figures, checks, and scope

Configured and reset cases produce `chronology.png` and `chronology.pdf`.
Each horizontal segment is an actual effective window; a filled start marker
represents all births in that Run and an open stop marker shows its excluded
chronological endpoint. The final shortened window and the reset operation are
annotated. Static time is summary-only. No thousands of identical birth points
or time-distribution histogram are plotted.

Generated CSV, JSON, PNG/PDF figures, and logs go under the selected results
directory. Keep sequence metadata together with its referenced CSV files.

T1 does not validate ActivityDriven, radionuclides, decay-time laws, time of
flight, transport time, RNG replay, realistic 120 kVp spectra, or G2/A2 pose/frame
behavior. Results describe the selected devices and the three configured
sequences above.
