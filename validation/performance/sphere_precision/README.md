# E1: Sphere FP64 versus FP32

The E2 integer extension and current local commands are documented at the end
of this file. Earlier E1 paths/commands below describe the E1 preparation.

This experiment compares the existing Sphere reference with a direct FP32
candidate in GGEMS navigation and endpoint transport. It does not select a
numerical winner, define a spatial acceptance threshold, or qualify a production
replacement. The existing `GGEMSSphereNavigation.clh` and
`GGEMSAnalyticArithmetic.clh` are unchanged.

## Profile and numerical scope

Configure `GGEMS_EXPERIMENTAL_SPHERE_PRECISION=FP64` (default) or `FP32`.
Other values are rejected. For an actual Sphere, the latter selects
`-DGGEMS_SPHERE_FP32=1` and omits the FP64 navigation enable flag from the real
`particle_stream_transport` build. The reference explicitly selects
`-DGGEMS_SPHERE_FP32=0 -DGGEMS_ENABLE_FP64_NAVIGATION=1`. World, Box and RotatedBox
continue to use their existing paths; RotatedBox still requires FP64. The extra
Sphere parameter is private, transient kernel state. Particle ABI, canonical
integer units, source sampling, Philox and Python APIs are unchanged.

The candidate subtracts integer centers before converting the local offset to
binary32, uses the stored binary32 direction, and retains impact-distance and
stable Q-root selection. Its radial residual is the ordinary `q.q - R*R`.
It omits the FP64 compensated residual, resolution band, and canonical-arrival
reconciliation band. Existing exact integer axial surface/tangent predicates
remain. Ambiguous rounded zeros and unusable parameters report `UNRESOLVED`;
finite-range checks expose failed arithmetic rather than repair it. There is no
epsilon adjustment, fallback, compensation or endpoint correction.

Sphere displacement and fractional rounding arithmetic are binary32. The global
integer anchor stays exact, and the existing final-coordinate half-away-from-zero
rule is retained. Thus this is not a conversion of global positions to float.
World exit still uses the common exact planar witness and integer movement.

## Identical-ray observations

The explicit target `ggems_sphere_precision` builds both probe profiles in one
process, independently of the host library's selected throughput profile. It
passes identical integer inputs and direction bits to production
`GGEMS_AnalyticLocate`, `GGEMS_AnalyticFindBoundary` and
`GGEMS_TransportToNextBoundary`. It does not duplicate the Sphere solver.
The first 32 inputs reproduce `tests/navigation/GGEMSSphereStreamTest.cc`;
a 33rd large-radius, nonunit-direction case observes FP32 discriminant overflow.
The original test expectations and tolerances are not E1 acceptance criteria.

This fixed-ray probe has global/local sizes 1/1 and no sampling or worker loop.
Its runtime is not a throughput measurement. A diagnostic boundary query precedes
the actual transport call, which queries again internally: serialized parameters
belong to that diagnostic query; committed events/endpoints belong to transport.

JSONL records preserve inputs, direction bits, requested and driver-reported
build options, device/runtime/driver identity, all attempted transactions, initial
ownership, material, accepted event order, World exit, failure stage and
`UNRESOLVED`. An unresolved path without an observed hit has an unknown hit/miss
decision. Integer coordinates and signed endpoint differences are strings so
binary64-only JSON consumers cannot silently round them. Distances are in pm;
ray parameters multiply the actual stored direction, which need not be unit length.

Paired endpoint/parameter distributions apply only while accepted event and owner
prefixes match. Different initial anchors after a previous rounding difference
are identified as propagated observations. Sphere and World data are kept
separate. Read the categorical disagreements and unavailable comparisons as well
as the reported medians and maxima; a small surviving subset is not evidence of
agreement across all rays.

The independent host oracle uses closest-point/chord arithmetic in `long double`
at each profile's own anchor. Its significand width is recorded. It is approximate,
not an exact sign oracle: very large radii and tiny squared radial residuals can
exceed even extended precision. Null oracle results are unavailable evidence.
FP64 is an implementation reference, not mathematical ground truth. All numerical
differences are observations, without accuracy pass/fail gates.

Programs are built directly from source through `cl::Program`, bypassing the
GGEMS binary cache. Vendor-internal compiler caching is not observable. The
probe records its source; retain the complete candidate source revision/patch
alongside results because included production headers are also inputs.

## Local build and run commands

The commands below are for Linux MU-TH-UR, from
`/home/dbenoit/Workspace/ggems`. The patch-review source copy is
`codex_scratch/e1_sphere/source`; it contains the candidate and read-only links to
the checkout's existing external dependencies. These commands do not apply the
patch to the checkout. Use the same compiler and Release configuration for both
profiles. Existing local Python/GTest paths are explicit; no dependency install
is required.

Configure/build FP64 (one line each):

```sh
cmake -S codex_scratch/e1_sphere/source -B codex_scratch/e1_sphere/build_fp64 -G Ninja -DCMAKE_BUILD_TYPE=Release -DCMAKE_CXX_COMPILER=clang++ -DGGEMS_EXPERIMENTAL_SPHERE_PRECISION=FP64 -DGGEMS_USE_COMPILER_CACHE=OFF -Dpybind11_DIR=/home/dbenoit/Workspace/ggems/.venv/lib/python3.14/site-packages/pybind11/share/cmake/pybind11 -DPython_EXECUTABLE=/home/dbenoit/Workspace/ggems/.venv/bin/python -DPYBIND11_STUBGEN_EXECUTABLE=/home/dbenoit/Workspace/ggems/.venv/bin/pybind11-stubgen -DFETCHCONTENT_SOURCE_DIR_GOOGLETEST=/home/dbenoit/Workspace/ggems/build/_deps/googletest-src
cmake --build codex_scratch/e1_sphere/build_fp64 --target ggems_core ggems_sphere_precision -j4
```

Configure/build FP32:

```sh
cmake -S codex_scratch/e1_sphere/source -B codex_scratch/e1_sphere/build_fp32 -G Ninja -DCMAKE_BUILD_TYPE=Release -DCMAKE_CXX_COMPILER=clang++ -DGGEMS_EXPERIMENTAL_SPHERE_PRECISION=FP32 -DGGEMS_USE_COMPILER_CACHE=OFF -Dpybind11_DIR=/home/dbenoit/Workspace/ggems/.venv/lib/python3.14/site-packages/pybind11/share/cmake/pybind11 -DPython_EXECUTABLE=/home/dbenoit/Workspace/ggems/.venv/bin/python -DPYBIND11_STUBGEN_EXECUTABLE=/home/dbenoit/Workspace/ggems/.venv/bin/pybind11-stubgen -DFETCHCONTENT_SOURCE_DIR_GOOGLETEST=/home/dbenoit/Workspace/ggems/build/_deps/googletest-src
cmake --build codex_scratch/e1_sphere/build_fp32 --target ggems_core ggems_sphere_precision -j4
```

Run the paired numerical observation on one explicitly selected device. Choose
the NVIDIA command on the intended GPU machine; inspect the recorded device name
to confirm the RTX 4060 Laptop. If multiple NVIDIA GPUs match, use its printed
numeric device index. The Intel command is the available local CPU check, not a
substitute for the GPU experiment:

```sh
codex_scratch/e1_sphere/build_fp64/validation/performance/sphere_precision/ggems_sphere_precision --device 'nvidia;gpu' --output codex_scratch/e1_sphere/sphere_nvidia.jsonl
codex_scratch/e1_sphere/build_fp64/validation/performance/sphere_precision/ggems_sphere_precision --device 'intel;cpu' --output codex_scratch/e1_sphere/sphere_intel.jsonl
```

## Real transport campaign

Use `validation/tmp/smoke_navigation_test.py`. It already specifies 65,536 workers,
1,000,000,000 Aioninos, Philox seed 7,777,777, point (-500,0,0) mm, +X direction,
uniform-solid-angle ConeBeam 0–2.5 degrees, a 250 mm Sphere at the origin, World
3 m per axis, and no observer capture. Change only its device selection from
`"0"` to `"nvidia;gpu"` in a scratch copy, and confirm the device printed at startup.
The current transport local size is 64 in both profiles.

The Python build writes its extension and generated stubs into its source package.
Two build directories sharing one Python source tree can silently run the wrong
profile's extension. Build the two campaign profiles in distinct scratch source
copies and select each package explicitly with `PYTHONPATH`. Do not build the
Python target from the protected checkout.

These are campaign preparation/run commands for use after review; the billion-history
campaign was not executed during patch preparation. Fresh destinations are
required (copying fails if they already exist). Each command is one shell line.

```sh
python3 -c 'from pathlib import Path; import shutil; candidate = Path("codex_scratch/e1_sphere/source"); [shutil.copytree(candidate, Path("codex_scratch/e1_campaign") / profile / "source", symlinks=True, ignore=shutil.ignore_patterns("build", "__pycache__", "*.so", "*.pyd", "*.dll", "*.dylib", "*.pyi")) for profile in ("fp64", "fp32")]'
mkdir -p codex_scratch/e1_campaign/fp64/evidence codex_scratch/e1_campaign/fp32/evidence
cmake -S codex_scratch/e1_campaign/fp64/source -B codex_scratch/e1_campaign/fp64/build -G Ninja -DCMAKE_BUILD_TYPE=Release -DCMAKE_CXX_COMPILER=clang++ -DGGEMS_EXPERIMENTAL_SPHERE_PRECISION=FP64 -DGGEMS_USE_COMPILER_CACHE=OFF -Dpybind11_DIR=/home/dbenoit/Workspace/ggems/.venv/lib/python3.14/site-packages/pybind11/share/cmake/pybind11 -DPython_EXECUTABLE=/home/dbenoit/Workspace/ggems/.venv/bin/python -DPYBIND11_STUBGEN_EXECUTABLE=/home/dbenoit/Workspace/ggems/.venv/bin/pybind11-stubgen -DFETCHCONTENT_SOURCE_DIR_GOOGLETEST=/home/dbenoit/Workspace/ggems/build/_deps/googletest-src > codex_scratch/e1_campaign/fp64/evidence/configure.log 2>&1
PYTHONDONTWRITEBYTECODE=1 cmake --build codex_scratch/e1_campaign/fp64/build --target ggems -j4 > codex_scratch/e1_campaign/fp64/evidence/build.log 2>&1
cmake -S codex_scratch/e1_campaign/fp32/source -B codex_scratch/e1_campaign/fp32/build -G Ninja -DCMAKE_BUILD_TYPE=Release -DCMAKE_CXX_COMPILER=clang++ -DGGEMS_EXPERIMENTAL_SPHERE_PRECISION=FP32 -DGGEMS_USE_COMPILER_CACHE=OFF -Dpybind11_DIR=/home/dbenoit/Workspace/ggems/.venv/lib/python3.14/site-packages/pybind11/share/cmake/pybind11 -DPython_EXECUTABLE=/home/dbenoit/Workspace/ggems/.venv/bin/python -DPYBIND11_STUBGEN_EXECUTABLE=/home/dbenoit/Workspace/ggems/.venv/bin/pybind11-stubgen -DFETCHCONTENT_SOURCE_DIR_GOOGLETEST=/home/dbenoit/Workspace/ggems/build/_deps/googletest-src > codex_scratch/e1_campaign/fp32/evidence/configure.log 2>&1
PYTHONDONTWRITEBYTECODE=1 cmake --build codex_scratch/e1_campaign/fp32/build --target ggems -j4 > codex_scratch/e1_campaign/fp32/evidence/build.log 2>&1
sha256sum codex_scratch/e1_campaign/fp64/source/python/ggems/*.so > codex_scratch/e1_campaign/fp64/evidence/python-module.sha256
sha256sum codex_scratch/e1_campaign/fp32/source/python/ggems/*.so > codex_scratch/e1_campaign/fp32/evidence/python-module.sha256
python3 -c 'from pathlib import Path; source = Path("codex_scratch/e1_sphere/source/validation/tmp/smoke_navigation_test.py").read_text(encoding="utf-8"); old = "opencl.select_devices(\"0\")"; assert source.count(old) == 1; source = source.replace(old, "opencl.select_devices(\"nvidia;gpu\")").replace("import ggems\n", "import ggems\nprint(ggems.ggems.__file__, flush=True)\n", 1); Path("codex_scratch/e1_campaign/smoke.py").write_text(source, encoding="utf-8", newline="\n")'
for e1_sample in warmup 1 2 3 4 5; do for e1_profile in fp64 fp32; do PYTHONDONTWRITEBYTECODE=1 PYTHONPATH="$PWD/codex_scratch/e1_campaign/$e1_profile/source/python" "$PWD/.venv/bin/python" -u codex_scratch/e1_campaign/smoke.py > "codex_scratch/e1_campaign/$e1_profile/evidence/$e1_sample.log" 2>&1; printf '%s\n' "$?" > "codex_scratch/e1_campaign/$e1_profile/evidence/$e1_sample.exit-code"; done; done
```

Check both build exit codes before running. The last command retains each process
exit code and continues collecting the other profile even if one fails. It is six
billion-history runs per profile, including warm-up. Reverse the inner profile
order for a second batch if comparing thermal/order effects. The printed module
path, CMake profile and logged OpenCL flag must agree.

Retain configure/build logs, CMake caches, exact source/patch provenance, module
hashes, and the complete stdout/stderr of every attempt, including failed runs.
Use one full warm-up per profile and repeated measured runs; record their order
and power/thermal conditions. The commands use a fresh process per run, so each
starts from the same seed. Worker-owned RNG plus atomic dynamic primary claiming
means that seed-matched runs do not assign identical rays to history IDs.
Throughput is a matched-distribution experiment, not a per-history numerical
comparison.

At detail 4 the existing program logs show exact OpenCL options and whether the
program came from source, memory cache or a cached binary. The profile flag is
part of cache identity. Keep these records; do not infer a cache hit from elapsed
time. The existing Linux GGEMS cache is under `~/.ggems/opencl_cache`, not
`XDG_CACHE_HOME`; these future production runs use that existing runtime behavior.
Neither profile adds fast-relaxed-math.

The added detail-2 chunk record prints the event-profiled `kernel_time_ps`, assigned
primaries, launch configuration and all 14 raw transport counters, even when the
run subsequently rejects unresolved geometry. Preserve zeros. The raw
`next_primary_id` includes terminal out-of-range worker claims and is not the
number of completed histories. Sum additive counters across chunks, take the maximum stack depth, retain
the raw cursor separately, sum kernel time, and report `completed_history_count / (kernel_time_ps * 1e-12)` histories/s,
alongside consumed/assigned counts and all error counters. If a profile fails,
report that outcome and its partial work explicitly; a fast failed run is not a
same-science speedup. Separate kernel time from host/end-to-end time and cache
initialization. Report each sample plus median/range; do not replace the paired
measurement with the historical ~7.574 s / ~132.03 M histories/s context.

Existing FP64 correctness tests remain unchanged. Their reference-specific
geometric expectations are not silently relaxed for FP32; this explicit E1
observation target adds no numerical acceptance threshold to CTest.

## E2: bounded integer Sphere candidates

The same CMake option now accepts four values. `FP64` remains the default; the
existing FP64 and FP32 algorithms are unchanged. These are experimental choices,
not a change to GGEMS canonical units or a production precision policy.

| CMake value | `GGEMS_SPHERE_FP32` | `GGEMS_SPHERE_INTEGER` | Sphere calculation |
|---|---:|---:|---|
| `FP64` | 0 | 0 | Original reference; FP64 navigation enabled |
| `FP32` | 1 | 0 | E1 candidate; FP64 navigation disabled |
| `INT64` | 0 | 64 | Exact relative pm, bounded two-limb discriminant |
| `INT32_LOCAL` | 0 | 32 | Quantized local int32 inputs, native int64 discriminant |

These flags appear in the actual `particle_stream_transport` build options and
therefore in its existing cache identity. Integer flags are zero for other
geometries. World/Box behavior is unchanged; RotatedBox still uses its FP64 path.
Neither integer Sphere profile requires FP64 device capability. No Fast Math flag
is added. Persistent coordinates, direction, energy, time and records are unchanged.

### Scales, bounds and operations

Both candidates subtract the integer Sphere center from the global integer anchor
before any spatial conversion. Directions are never normalized by the candidate.
Their binary32 bits are decoded exactly, then rounded to fixed-point integers,
nearest with ties away from zero. Each stored component must be in `[-4,4]`, and
the quantized vector must be nonzero. No floating arithmetic, float fallback or
precision retry participates in either integer Sphere solver or endpoint movement.

`INT64` retains `q` and `R` in integer pm with `|q_i| <= 2^42` and
`1 <= R <= 2^42` (about 4.398 m). It uses `D = round(direction * 2^17)`;
each `|D_i| <= 2^19`. Position/radius quantization is zero within that domain.
Direction quantum is `2^-17`, with per-component conversion error at most half
that quantum. Exact ownership compares the two-limb sums `q.q` and `R*R`, using
the sign of `q.D` for forward ownership on the surface.

For the intersection, `A=D.D < 2^40`, `|B=q.D| < 2^63`, and
`H=sum((R*D_i)^2)-sum((q cross D)_i^2)`. Each cross component is bounded by
`2^62`; each nonnegative sum fits unsigned 128 bits. If `H` is positive it is
less than `3*2^122 < 2^124`. Four 32-bit partial products implement each wide
product; there is no arbitrary-precision library. A restoring integer square
root takes at most 62 bit-pair steps. The selected numerator is
`n=-B +/- floor(sqrt(H))`, with nonnegative `n < 2^64`; the denominator is `A`.
The floor square root is a declared numerical approximation, not a corrected root.

`INT32_LOCAL` uses a fixed spatial quantum `Q=2^24 pm = 16.777216 micrometers`.
It rounds each relative coordinate and radius to this grid, nearest with ties
away from zero, and requires `|q_i_ticks| <= 32767` and
`1 <= R_ticks <= 32767`. The exact raw magnitude limit is
`32767*Q + Q/2 - 1 pm`; radii below `Q/2` are unsupported. This is approximately
a 549.74 mm local range. Position/radius conversion errors are at most `Q/2`.
The grid was chosen from the native quadratic's bit budget, not an accuracy goal.
Large global translations do not consume this local range.

This profile uses `D=round(direction * 2^12)`, `|D_i| <= 16384`.
Its direction quantum is `2^-12`, again with a half-quantum maximum conversion
error per component. Local coordinates and `D` are int32; `A=D.D` and `B=q.D`
fit signed int32. `C=q.q-R*R` and `H=B*B-A*C` use signed int64:
`|B*B|+|A*C| < 18*2^58 < 2^63`. The native integer square root takes at most
32 steps. The selected local numerator is multiplied by `Q`, remaining below
`2^56`, before entering the common rational endpoint path. This is a native
quadratic solver, distinct from INT64's exact-pm wide cross-product formulation.

For both profiles the packet holds `n_pm/A`. With direction fraction bits `F`,
the comparable ray parameter is `t_pm=(n_pm/A)*2^F` and the unrounded integer-ray
displacement is `n_pm*D/A`. Parameter times the applicable direction norm gives
the diagnostic path length. Native divmod `n_pm/A` factors each endpoint into
an integral displacement and a bounded residual product (`<2^59`), avoiding
software multiword division. The final signed global coordinate is rounded once,
nearest with ties away from zero. Thus a local displacement tie is decided using
the sign of the final global coordinate, not the displacement alone.

World selection and exit movement retain the original unquantized stored direction.
A small private two-limb scaled comparison orders the candidate parameter against
the exact World witness; this common World-order operation also uses 128 bits for
`INT32_LOCAL`. Candidate endpoints must remain in the closed World before atomic
commit. Outside endpoints fail without clipping, saturation or a spatial push.
Exact tangency has no positive chord from outside; inconsistent inside ownership,
unusable forward roots, unsupported inputs and ordering failures stay unresolved.
The original material/owner and bounded three-event transport rules remain in force.

The likely costs to measure are integer square-root loops, native 64-bit divmod,
wide products in INT64 and exact World ordering. No speed advantage is assumed.

### Four-profile observations

The existing executable now runs all four profiles and compares each candidate
with the same FP64 trace. All 33 E1 fixtures remain, with three explicit domain
checks and eight medical ConeBeam rays. The latter use radius 250 mm, source
(-500,0,0) mm and World half extents 1.5 m. Fixed stratified uniform bit patterns
are passed once to the unchanged production angular sampler for the 0–2.5 degree
solid-angle cone in a +X frame. The captured direction bits are reused verbatim
by all four navigators; this is a descriptive sample, not a statistical campaign.

JSONL adds per-profile domains/widths, actual per-query quantized inputs and signed
conversion residuals, rational parameters, query reasons, diagnostic path lengths
and cumulative accepted-path differences. A post-volume-exit diagnostic preflight
is marked unused because the production convex-volume shortcut bypasses it.
Unavailable quantization fields are null. Endpoint failure is distinguished by
the Transport failure stage; query reasons describe the preceding diagnostic query.
Raw accepted endpoints and the initial ray are sufficient to reconstruct propagated
differences independently. The source capture and source direction law are not
changed to accommodate any navigator.

Statistics report scalar sample count, contributing rays, excluded rays, minimum,
median, nearest-rank p95/p99 and maximum. The exclusion count is all input rays
minus rays with at least one finite sample for that particular statistic; a ray
may contribute to one event/anchor group and be excluded from another. Classified
and unclassified hit decisions are separate from complete and unresolved paths:
a known hit can still fail on a later boundary. No difficult ray is discarded
from the raw output, and an unknown decision is never reported as a miss.

The `long double` oracle remains an approximate independent original-direction
calculation at each profile's own anchor. Report its sample population and
limitations together with its errors. Diagnostic parameters/path lengths come
from the pre-query, whereas events and integer endpoints come from the actual
Transport transaction. The full four-way numerical comparator requires a device
capable of running the FP64 reference even though integer-only production does not.

### Current local E2 commands and verification boundary

From `/home/dbenoit/Workspace/ggems`, the protected checkout already contains E1.
The patch-review candidate is `codex_scratch/e2_sphere/source`; its baseline is a
byte-preserved snapshot of the current working tree, including uncommitted E1.
No checkout source is built in place. The local GoogleTest source currently
available is `codex_scratch/n1_2_python_geometry/deps/googletest` (version 1.18.0);
the old E1 `build/_deps` path is no longer available.

Build/run the bounded four-profile observation (each command is one line):

```sh
TMPDIR="$PWD/codex_scratch/e2_sphere/logs" cmake -S codex_scratch/e2_sphere/source -B codex_scratch/e2_sphere/build_fp64 -G Ninja -DCMAKE_BUILD_TYPE=Release -DCMAKE_CXX_COMPILER=clang++ -DGGEMS_EXPERIMENTAL_SPHERE_PRECISION=FP64 -DGGEMS_USE_COMPILER_CACHE=OFF -Dpybind11_DIR=/home/dbenoit/Workspace/ggems/.venv/lib/python3.14/site-packages/pybind11/share/cmake/pybind11 -DPython_EXECUTABLE=/home/dbenoit/Workspace/ggems/.venv/bin/python -DPYBIND11_STUBGEN_EXECUTABLE=/home/dbenoit/Workspace/ggems/.venv/bin/pybind11-stubgen -DFETCHCONTENT_SOURCE_DIR_GOOGLETEST=/home/dbenoit/Workspace/ggems/codex_scratch/n1_2_python_geometry/deps/googletest
TMPDIR="$PWD/codex_scratch/e2_sphere/logs" cmake --build codex_scratch/e2_sphere/build_fp64 --target ggems_sphere_precision -j4
TMPDIR="$PWD/codex_scratch/e2_sphere/logs" codex_scratch/e2_sphere/build_fp64/validation/performance/sphere_precision/ggems_sphere_precision --device 'intel;cpu' --output codex_scratch/e2_sphere/verification/sphere_four_cpu.jsonl
TMPDIR="$PWD/codex_scratch/e2_sphere/logs" codex_scratch/e2_sphere/build_fp64/validation/performance/sphere_precision/ggems_sphere_precision --device 'nvidia;gpu' --output codex_scratch/e2_sphere/verification/sphere_four_nvidia.jsonl
```

For later throughput measurement, prepare **new, separate** Python source packages
from the candidate. These are commands for use after review, not an automatically
launched campaign. Fresh copy destinations are required. The configure loop below
explicitly builds all four choices, each with its own source, module and build:

```sh
python3 -c 'from pathlib import Path; import shutil; candidate = Path("codex_scratch/e2_sphere/source"); [shutil.copytree(candidate, Path("codex_scratch/e2_campaign") / profile / "source", symlinks=True, ignore=shutil.ignore_patterns("build", "__pycache__", "*.so", "*.pyd", "*.dll", "*.dylib", "*.pyi")) for profile in ("FP64", "FP32", "INT64", "INT32_LOCAL")]'
(for e2_profile in FP64 FP32 INT64 INT32_LOCAL; do TMPDIR="$PWD/codex_scratch/e2_sphere/logs" cmake -S "codex_scratch/e2_campaign/$e2_profile/source" -B "codex_scratch/e2_campaign/$e2_profile/build" -G Ninja -DCMAKE_BUILD_TYPE=Release -DCMAKE_CXX_COMPILER=clang++ -DGGEMS_EXPERIMENTAL_SPHERE_PRECISION="$e2_profile" -DGGEMS_USE_COMPILER_CACHE=OFF -Dpybind11_DIR="$PWD/.venv/lib/python3.14/site-packages/pybind11/share/cmake/pybind11" -DPython_EXECUTABLE="$PWD/.venv/bin/python" -DPYBIND11_STUBGEN_EXECUTABLE="$PWD/.venv/bin/pybind11-stubgen" -DFETCHCONTENT_SOURCE_DIR_GOOGLETEST="$PWD/codex_scratch/n1_2_python_geometry/deps/googletest" && TMPDIR="$PWD/codex_scratch/e2_sphere/logs" PYTHONDONTWRITEBYTECODE=1 cmake --build "codex_scratch/e2_campaign/$e2_profile/build" --target ggems -j4 || exit 1; done)
python3 -c 'from pathlib import Path; source = Path("codex_scratch/e2_sphere/source/validation/tmp/smoke_navigation_test.py").read_text(encoding="utf-8"); old = "opencl.select_devices(\"0\")"; assert source.count(old) == 1; source = source.replace(old, "opencl.select_devices(\"nvidia;gpu\")").replace("import ggems\n", "import ggems\nprint(ggems.ggems.__file__, flush=True)\n", 1); Path("codex_scratch/e2_campaign/smoke.py").write_text(source, encoding="utf-8", newline="\n")'
```

Each following command requests exactly one billion-history run. Use only when
the appropriate GPU is available and measurement is authorized. Check all four
builds first and confirm the RTX 4060 Laptop in the startup log. Preserve exit
codes and use distinct output filenames for each warm-up/repetition:

```sh
TMPDIR="$PWD/codex_scratch/e2_sphere/logs" PYTHONDONTWRITEBYTECODE=1 PYTHONPATH="$PWD/codex_scratch/e2_campaign/FP64/source/python" .venv/bin/python -u codex_scratch/e2_campaign/smoke.py > codex_scratch/e2_campaign/FP64/run.log 2>&1
TMPDIR="$PWD/codex_scratch/e2_sphere/logs" PYTHONDONTWRITEBYTECODE=1 PYTHONPATH="$PWD/codex_scratch/e2_campaign/FP32/source/python" .venv/bin/python -u codex_scratch/e2_campaign/smoke.py > codex_scratch/e2_campaign/FP32/run.log 2>&1
TMPDIR="$PWD/codex_scratch/e2_sphere/logs" PYTHONDONTWRITEBYTECODE=1 PYTHONPATH="$PWD/codex_scratch/e2_campaign/INT64/source/python" .venv/bin/python -u codex_scratch/e2_campaign/smoke.py > codex_scratch/e2_campaign/INT64/run.log 2>&1
TMPDIR="$PWD/codex_scratch/e2_sphere/logs" PYTHONDONTWRITEBYTECODE=1 PYTHONPATH="$PWD/codex_scratch/e2_campaign/INT32_LOCAL/source/python" .venv/bin/python -u codex_scratch/e2_campaign/smoke.py > codex_scratch/e2_campaign/INT32_LOCAL/run.log 2>&1
```

These use the unchanged 65,536-worker, Philox seed 7,777,777, CountDriven Aionino,
125-keV, Air/Water, no-Observer scenario described above. Record the printed
module path, CMake profile, actual program options/cache source, 64-work-item local
size, every counter and kernel-event time. Keep host/wall time separate. Report
individual samples and incomplete runs; use completed histories divided by summed
kernel-event seconds for completed-histories/s. Seed equality still does not align
history rays across dynamically scheduled workers. Historical FP64/FP32 timings
are context only, including the separate FP32 Fast Math observation.

Patch verification built and imported four isolated modules under
`codex_scratch/e2_sphere/packages/`. Only those verification copies redirect the
existing GGEMS binary cache directory into scratch via a recorded, excluded adapter;
the delivered production code retains its original cache behavior. Initialize-only
checks verify the real transport profile/options without launching histories.
Exact adapter diffs, loaded module hashes, compiler logs and tested kernel hashes
are retained with each package. No billion-history run or NVIDIA performance
measurement is implied by those checks.

The local 44-ray CPU observation on Intel i5-12500H, driver
`2026.21.7.0.24_160000`, produced the following counts. Classified means that
hit/miss is known; it does not imply a completed path.

| Profile | Classified | Unclassified | UNRESOLVED | Category differences from FP64 |
|---|---:|---:|---:|---:|
| FP64 | 42 | 2 | 3 | reference |
| FP32 | 38 | 6 | 9 | 10 |
| INT64 | 38 | 6 | 8 | 6 |
| INT32_LOCAL | 30 | 14 | 14 | 17 |

The raw observation is retained locally in
`codex_scratch/e2_sphere/verification/sphere_four_profiles_cpu.jsonl`.
All 208 original FP64/FP32 ray/attempt rows for the 33 E1 fixtures are unchanged.
These results establish neither an accuracy acceptance level nor GPU performance.
