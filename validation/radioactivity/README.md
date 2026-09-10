# Radionuclide validation

Run an ActivityDriven source, then compare its populations, birth times and
energies with independent calculations and a selected nuclear reference. The
first reference is O-15. Scientific discrepancies remain in `analysis.json`;
this workflow never changes a built-in.

## Build and use

From the repository root, with the normal GGEMS build and a Python environment
containing NumPy, SciPy and Matplotlib:

```powershell
cmake --build build --target validation_radioactivity

python ./validation/radioactivity/run_campaign.py --help
python ./validation/radioactivity/run_campaign.py --exporter ./build/validation/radioactivity/ggems_radionuclide_exporter.exe --reference ./validation/radioactivity/data/O-15/reference/reference.json --output ./codex_scratch/o15_cpu --device cpu

python ./validation/radioactivity/plot.py ./codex_scratch/o15_cpu
python ./validation/radioactivity/analyze.py ./codex_scratch/o15_cpu
```

Use `--device gpu` and a new output directory for a GPU campaign. Selectors go
directly to `GGEMSOpenCL::SelectDevices()`. The scripts launch directly; no module
invocation is needed. The exporter also accepts `--help` and
`--describe --nuclide O-15 --output <directory>` to export a compiled definition
without OpenCL.

For a protected live checkout, use an isolated source copy: the normal Python
build generates binding/stub files in its source tree. The campaign keeps the
existing Point/Fixed source, Philox seed 77777 and 256 workers per device. Every
CLI option explains its meaning and default. Exporter invocations have a
configurable 1800-second timeout; failed operations retain their logs.

## Reference and compiled data

The [O-15 reference package](data/O-15/reference/README.md) records the selected
LNHB evaluation and BetaShape 2.4 calculation, decimal values and uncertainties,
source filenames, transformations, exclusions and ordered emission-group mapping.
ENSDF and MIRD remain independent cross-checks. Raw evidence is documentary;
campaigns read the selected reference JSON and spectrum CSV.

The three existing conditional energy descriptions are:

- `Mono`: an `energy` object with `value`, `standard_uncertainty`, `unit: "keV"`
  and source identity.
- `DiscreteLines`: ordered `lines` with exact `energy_keV` values and conditional
  `weight` entries.
- `RegularSpectrum`: `reference_representation: "piecewise_linear_density"`,
  `energy_unit: "keV"` and `spectrum_file`. The CSV columns are
  `energy_keV,density_per_keV,standard_uncertainty_per_keV`.

Global yields are expected primaries per parent decay; they are never normalized.
Conditional energy weights and normalization are separate. O-15 maps to one
Positron group. The 511 keV annihilation radiation is reference information only;
annihilation belongs to positron transport. EC creates no placeholder primary.
Other nuclides use the same code once their evaluations and mappings are selected.

The compiled definition export contains half-life, ordered particles/yields,
distribution kinds, mono energies, full tables and ticket CDFs. Energy is integer
micro-eV and Time is integer ps; conversion factors come from central GGEMS Units.
Dose remains independent. The optional source-comment audit identifies generator
provenance separately because the compiled definition has no provenance API.

## Campaign and statistical meaning

The default duration is four compiled half-lives, divided into 32 consecutive
half-open windows after rounding to ps. Long-lived nuclides use 90% of the
available uint64 Time range when four half-lives cannot fit; requested and actual
horizons are recorded. A window shorter than one ps cannot be used.

Activity at time zero is chosen for 1000 expected total primaries in the final
window: `A = target / (I_last * sum(yields))`, where `I_last` is the independent
decay integral at 1 Bq. Rare groups do not enlarge the campaign. Capture capacity
uses the first-window mean plus 12 standard deviations and 64 primaries. A
population exceeding that capacity or an Observer overflow stops the export.

Use `--horizon-half-lives 0.000001` to exercise the existing small-decay numerical
regime. Activity is still derived from the sample target; no production half-life
or RNG draw changes.

The analysis keeps these conclusions separate:

1. **Nuclear data:** name, half-life, ordered groups, particles, yields, kinds,
   mono energies and discrete-line laws. Continuous tables report bin-mass
   differences, bounds, excluded reference mass, CDF supremum and exact
   finite-ticket mean. Scalar decimal/binary representation tolerance is 1e-14.
   Pointwise reference uncertainties are retained, but absent covariance prevents
   an uncertainty-aware evaluation-equivalence test.
2. **Integrated decay counts:** an independent 80-digit Decimal exponential
   integral for every window, with a power series for `(1-exp(-x))/x` at small
   `x`. GGEMS planner expectations must agree within 5e-14 relative error.
   A separate seed-matched planner exposes those expectations without advancing
   Run's random streams.
3. **Poisson populations:** each group has mean `Lambda * yield`, independently;
   groups are not mutually exclusive branches. Exact equal-tail prediction
   intervals cover groups, window totals and the whole horizon. Another 128
   host-only population experiments use seeds `seed+1` through `seed+128`.
   Discrete ECDF tests compare their populations at each fixed mean. Pearson
   dispersion uses one degree of freedom per independent count, no fitted
   parameters, and only means at least 100.
4. **Birth times:** per-window ECDF comparison with the exponential distribution
   conditioned to `[start, stop)`, with explicit observed-bound checks. Numerical
   allowance remains the Source 1e-5 CDF contract, the high-24-bit uniform quantum,
   and one ps of conditional probability mass. Existing GGEMS GoogleTests cover
   deterministic raw-zero inclusion and maximum-word stop exclusion.
5. **Energy:** sampled support and full ECDF versus both the selected reference
   and the compiled finite-ticket law. The exact integer CDF inverts
   `floor(width * local_ticket / bin_ticket_count)`. The observed reference/grid
   discrepancy is never added to the acceptance threshold. Statistical
   non-rejection does not establish identical nuclear tables.

The family significance level is 0.01, divided by the unchanged conservative
Bonferroni count `windows*(2*groups+3)+5*groups+1`. ECDF comparisons use the
[DKW-Massart bound](https://doi.org/10.1214/aop/1176990746). Poisson probabilities,
quantiles and chi-square tails use SciPy. Thresholds are recorded before sampling;
seeds, thresholds and retained samples are not changed after observing a result.
Empty groups are `insufficient_samples`, not `pass`. Plots omit empty-window
statistical points.

Source RNG order remains birth time, then conditional energy for Point/Fixed;
Mono consumes no energy word. Additional population experiments use separate
host streams. CPU and GPU runs with the same seed share the same host populations
and must not be pooled as independent population experiments. Atomic worker
assignment prevents per-history bitwise reproducibility across schedules.
Physical daughter transport, annihilation, stopping, navigation and dose are
outside this validation.

## Results

- `settings.json`: reference, design, fixed significance level, commands and
  scientific Python environment.
- `run/definition.json` and `group_N.csv`: compiled definition and energy tables.
- `run/run.json`: devices, compiler/build mode, seed/workers, chronology and capture
  counts.
- `run/populations.csv`: expected and sampled populations by replica/window/group.
- `run/samples.csv`: `window,group,time_ps,energy_micro_eV`.
- `analysis.json`: separate scientific results, distances and thresholds.
- `plots/`: standalone population, birth-time and energy figures.
- `describe.log` and `exporter.log`: executed commands and diagnostics.

Reanalysis and plotting use retained samples without new draws. Command success
means execution completed; consult each scientific result in `analysis.json`.
Files use LF line endings. GGEMS software regression tests remain in `tests/`
and use GoogleTest; this directory contains scientific validation only.
