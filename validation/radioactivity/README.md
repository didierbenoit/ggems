# Radionuclide validation

Run an ActivityDriven source, then compare its populations, birth times and
energies with independent calculations and a selected nuclear reference. The
reference packages cover fourteen radionuclides. Scientific comparisons are
reported in `analysis.json`; the campaign does not modify the built-in data.

## Build and run

Run commands from the repository root. Use an already configured GGEMS build
and Python 3.12 or newer with NumPy, SciPy, and Matplotlib installed.
The `validation_radioactivity` target builds the exporter without running a
campaign:

```console
cmake --build build --target validation_radioactivity
python validation/radioactivity/run_campaign.py --help
```

For a Windows multi-configuration build, select the configuration and use its
executable directory:

```powershell
cmake --build build --config Release --target validation_radioactivity
python validation/radioactivity/run_campaign.py --exporter build/validation/radioactivity/Release/ggems_radionuclide_exporter.exe --reference validation/radioactivity/data/O-15/reference/reference.json --output validation/radioactivity/results/o15_cpu --device cpu
```

For a single-configuration build, the executable has no `Release/` directory.
On Windows it retains the `.exe` suffix; on Linux and macOS, use:

```console
python validation/radioactivity/run_campaign.py --exporter build/validation/radioactivity/ggems_radionuclide_exporter --reference validation/radioactivity/data/O-15/reference/reference.json --output validation/radioactivity/results/o15_cpu --device cpu
```

The runner extracts and analyzes samples. Generate figures separately:

```console
python validation/radioactivity/plot.py validation/radioactivity/results/o15_cpu
```

Use `--device gpu` and a new output directory for a GPU campaign. Device
selectors use the standard GGEMS OpenCL selection rules. Change `--reference`
to select another radionuclide from the table below. The output directory must
not already exist.

Defaults are Point geometry, Fixed direction, Philox seed 77777, and 256 workers
per device. The runner chooses activity from the sample target described below;
`--target-last-window` controls that target. Larger populations increase capture
and analysis costs. Each exporter invocation has a configurable 1800-second
timeout (`--timeout-seconds`); failed operations retain their logs.

To analyze the same samples again without a new simulation:

```console
python validation/radioactivity/analyze.py validation/radioactivity/results/o15_cpu
```

Reanalysis replaces `analysis.json` and reads the reference and source-tree paths
recorded in `settings.json`. Keep those locations available when moving results.
Run `plot.py` again to refresh figures after reanalysis.

The exporter can also write a compiled definition without initializing OpenCL:

```console
build/validation/radioactivity/ggems_radionuclide_exporter --describe --nuclide O-15 --output validation/radioactivity/results/o15_definition
```

Adapt the executable path to the platform and configuration as above.

## Reference and compiled data

Each package documents its selected evaluation, source files, uncertainties,
transformations, exclusions, and ordered emission groups. Read its README before
interpreting comparisons, especially where the modeled emissions form only a
subset of the physical inventory.

| Radionuclide | Reference documentation | Campaign input |
| --- | --- | --- |
| H-3 | [H-3 reference](data/H-3/reference/README.md) | `data/H-3/reference/reference.json` |
| C-11 | [C-11 reference](data/C-11/reference/README.md) | `data/C-11/reference/reference.json` |
| C-14 | [C-14 reference](data/C-14/reference/README.md) | `data/C-14/reference/reference.json` |
| O-15 | [O-15 reference](data/O-15/reference/README.md) | `data/O-15/reference/reference.json` |
| F-18 | [F-18 reference](data/F-18/reference/README.md) | `data/F-18/reference/reference.json` |
| Co-60 | [Co-60 reference](data/Co-60/reference/README.md) | `data/Co-60/reference/reference.json` |
| Ga-68 | [Ga-68 reference](data/Ga-68/reference/README.md) | `data/Ga-68/reference/reference.json` |
| Tc-99m | [Tc-99m reference](data/Tc-99m/reference/README.md) | `data/Tc-99m/reference/reference.json` |
| I-123 | [I-123 reference](data/I-123/reference/README.md) | `data/I-123/reference/reference.json` |
| I-124 | [I-124 reference](data/I-124/reference/README.md) | `data/I-124/reference/reference.json` |
| I-125 | [I-125 reference](data/I-125/reference/README.md) | `data/I-125/reference/reference.json` |
| I-131 | [I-131 reference](data/I-131/reference/README.md) | `data/I-131/reference/reference.json` |
| Lu-177 | [Lu-177 reference](data/Lu-177/reference/README.md) | `data/Lu-177/reference/reference.json` |
| Am-241 | [Am-241 reference](data/Am-241/reference/README.md) | `data/Am-241/reference/reference.json` |

Raw evaluation files provide provenance and independent comparisons. Campaigns
read the selected `reference.json` and any referenced spectrum CSV files.

The three conditional energy descriptions are:

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
The other packages supply their own ordered emission groups to the same runner.

The compiled definition export contains half-life, ordered particles/yields,
distribution kinds, mono energies, full tables and ticket CDFs. Energy is integer
micro-eV and Time is integer ps; conversion factors come from central GGEMS Units.
Generator provenance is reported separately from source-file comments when the
reference requests it. These comments do not identify the compiled binary.

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

Use `--horizon-half-lives 0.000001` for a short observation horizon relative to
the half-life. Activity is still derived from the sample target.

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
   and one ps of conditional probability mass. Statistical samples alone do not
   establish whether every exact endpoint is reachable.
5. **Energy:** sampled support and full ECDF versus both the selected reference
   and the compiled finite-ticket law. The exact integer CDF inverts
   `floor(width * local_ticket / bin_ticket_count)`. The observed reference/grid
   discrepancy is never added to the acceptance threshold. Statistical
   non-rejection does not establish identical nuclear tables.

The family significance level is 0.01, divided by the conservative
Bonferroni count `windows*(2*groups+3)+5*groups+1`. ECDF comparisons use the
[DKW-Massart bound](https://doi.org/10.1214/aop/1176990746). Poisson probabilities,
quantiles and chi-square tails use SciPy. Thresholds are recorded before sampling;
seeds, thresholds and retained samples are not changed after observing a result.
Empty samples and DKW comparisons whose complete acceptance limit (including
any numerical budget) is >= 1 are `insufficient_samples`. Computed diagnostics
are retained. Rare physical groups are never amplified to force a statistical
PASS. Plots omit statistically insufficient window comparisons.

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
- `plots/`: population and birth-time figures, detailed `energy_group_N.png`
  emission figures, and `<nuclide>_emission_<particle>.png` aggregate figures.
- `describe.log` and `exporter.log`: executed commands and diagnostics.

Reanalysis and plotting use retained samples without new draws. Command success
means execution completed; consult each scientific result in `analysis.json`.
`pass` and `fail` refer to the recorded comparison and threshold.
`insufficient_samples` means the available sample cannot support that test;
`comparison_only` reports a deterministic difference without a pass/fail decision.
`not_applicable` indicates that the stated conditions for a test are not met.

## Emission figures

These are **source emission spectra**, not detected or measured spectra.
Particle figures aggregate every group of the same runtime particle type.
Filename tokens are `alpha`, `beta_plus` (positrons), `gamma`, and `electron`.
GGEMS represents beta-minus, Auger and conversion electrons as `Electron`;
their contributions remain together rather than inferring a new particle type
from an emission group's origin. Detailed group figures retain that distinction.

Aggregate curves retain the physical yields: each conditional distribution is
multiplied by its group yield per parent decay before summation. Compiled
continuous densities use the union of the original bin edges and finite-ticket
bin masses. Reference curves sum the selected piecewise-linear densities only
within their individual supports. No spectrum is extrapolated or independently
rescaled to make its group contribution as large as another's.

Mono and discrete groups use line spectra; identical canonical energies are
combined. Line intensities are emissions per parent decay, with a logarithmic
intensity axis to retain weak lines. Continuous densities are emissions per
parent decay per keV. A particle with both kinds has two useful panels with
these distinct units. No figure is created for an absent runtime particle type.

Sample intensities are counts divided by the retained independent expected
parent-decay integral (and by histogram width for a continuous density).
The displayed `sqrt(N)` count error bars describe sampling noise; they are not
new acceptance intervals or evaluated nuclear-data uncertainties. No samples
are created for rare groups. Their deterministic contributions remain visible.
Detailed continuous plots retain their conditional densities and available CDF
comparisons. Discrete plots have one panel, and unavailable comparison panels
and empty legends are omitted. Plotting does not modify `analysis.json`.
