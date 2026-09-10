# Radionuclide validation

This is a generic validation workflow for compiled GGEMS radionuclides. The first
selected reference is O-15. A successful campaign execution does **not** imply
that its nuclear data, population law, birth times, spectra and physical
transport have all been validated. Read the separate results in `analysis.json`.
Scientific discrepancies are retained; this workflow never corrects a built-in.

The runner uses the public built-in factory, Source, population planner, Run and
Observer APIs. It forwards device selectors to `GGEMSOpenCL::SelectDevices()`.
There is one Point/Fixed ActivityDriven source, with the original ordered emission
groups and their original yields. No production RNG, unit, ABI, process or
transport code is changed. Canonical sample columns are `time_ps` and
`energy_micro_eV`; conversion factors in exported definitions come from central
GGEMS Units. Dose remains independent.

## Build and run

Use an isolated source copy if the live checkout must remain read-only. The
ordinary Python build can generate files in its source tree. From that isolated
repository root, using its normal configured build and Python environment:

```text
cmake --build build --target validation_radioactivity
python -B -m unittest discover -s validation/radioactivity/tests -v
python -B -m validation.radioactivity.run_campaign --exporter build/validation/radioactivity/ggems_radionuclide_exporter.exe --reference validation/radioactivity/data/O-15/reference/reference.json --output <new-campaign-directory> --device cpu --revision <reviewed-source-revision>
python -B -m validation.radioactivity.plot <campaign-directory>
python -B -m validation.radioactivity.analyze <campaign-directory>
```

On other platforms omit `.exe`. Analysis requires Python 3.12 or later and
SciPy; plotting requires Matplotlib and NumPy. The pilot uses the existing T800
environment. Versions, platform, commands and the supplied source revision are
saved in `settings.json`. Install validation dependencies only in an authorized
environment; they are not production GGEMS dependencies.

Use `--device gpu` to select GGEMS GPU devices. Device names, order, vendor,
driver, OpenCL version, build mode and host compiler are retained in `run.json`.
The default is Philox, seed 77777 and 256 workers per selected device. The
exporter rejects an existing output directory. Failed or timed-out runs retain
their evidence; they never produce an accepted complete analysis. The default
exporter timeout is 1800 seconds and can be specified before the campaign.

The executable also supports `--describe --nuclide <canonical-name> --output
<new-directory>` without OpenCL. `definition.json` and `group_N.csv` expose the
compiled half-life, yields, kinds, mono energies, complete tables, weights and
ticket CDFs. The runtime definition has no generator/provenance property, so that
field is explicitly unavailable. A reference may identify a source comment for
a separate, clearly labeled provenance audit.

## Reference data

See [the O-15 provenance package](data/O-15/reference/README.md). The schema uses
decimal strings for source scientific values and uncertainties. `reference.json`
contains identity, half-life, ordered groups, their particle types and yields,
distribution descriptions, exclusions and raw-file provenance. Yields mean
expected primaries per parent decay; they are never normalized.

The three generic distribution descriptions are:

- `Mono`: `energy` contains `value`, `standard_uncertainty`, `unit: "keV"` and
  source identity.
- `DiscreteLines`: `lines` contains ordered `energy_keV` and conditional `weight`
  entries. Weights describe selection within that emission group, independently
  of its yield. Energies must be exactly representable in canonical Energy.
- `RegularSpectrum`: `reference_representation: "piecewise_linear_density"`,
  `energy_unit: "keV"`, and `spectrum_file` identify a CSV with columns
  `energy_keV,density_per_keV,standard_uncertainty_per_keV`. All supplied points
  are preserved. Analysis integrates the linear density, rather than linearly
  interpolating a cumulative histogram. Conditional normalization and any
  transformations must be explicit in the data provenance.

The O-15 reference maps the selected ground-state beta+ transition to one
Positron group. Its 511 keV annihilation information is explicitly excluded from
Source. EC is a nuclear channel, not a transported source particle. ENSDF and
MIRD cross-checks are not merged into the selected LNHB evaluation.

Additional references for H-3, C-14, F-18, C-11, Ga-68, Co-60, Lu-177, the iodine
nuclides, Am-241 and Tc-99m can use the same exporter and analysis code. Selecting
and mapping each evaluation remains scientific work. This pilot does not certify
those nuclides or invent their reference packages.

## Campaign design

The default horizon is four **compiled half-lives**, split into 32 consecutive
equal windows after rounding to picoseconds. No O-15 seconds are hard-coded.
For very long half-lives, uint64 picoseconds cannot hold four half-lives. The
generic design caps the horizon at 90% of the available Time range and explicitly
records the requested and actual horizons and the reason. Sub-quantum windows
are rejected. No unit contract is changed.

The activity is chosen to give an expected **1000 total useful source primaries
in the final window**. If `I_last` is the independent decay integral at 1 Bq and
`Y` is the sum of the original emission yields, `A_ref = 1000 / (I_last * Y)`.
Activity is referenced at time zero. Every group then has its own mean
`Lambda_window * yield_group`; groups are not mutually exclusive branches. The
Observer allocation includes a 12-standard-deviation margin above the first
window's total mean plus 64 primaries. This is an allocation policy, not a test
threshold. Incomplete capture is an infrastructure error, never a truncated
accepted sample. Rare groups do not increase the campaign size.

Use `--horizon-half-lives 0.000001` for the generic small-decay regression. This
exercises the existing uniform-limit numerical branch while testing against the
conditioned exponential reference. Its activity is again derived from the useful
sample target. No synthetic production half-life or extra RNG draw is introduced.

## Separate scientific checks

1. **Nuclear/reference data.** Compare compiled name, half-life, ordered groups,
   particles, yields and distribution kinds. Mono energies and discrete line
   laws are checked generically. A continuous spectrum compares every compiled
   bin mass, grid bounds and width, reference mass outside support, CDF supremum,
   and exact finite-ticket mean with the selected reference. A 1e-14 relative
   tolerance for scalar decimal/binary representation is distinct from nuclear
   uncertainty. Shape uncertainties are retained, but no covariance is available;
   the pilot tests the tabulated central shape, not uncertainty-aware evaluation
   equivalence.
2. **Integrated decay counts.** Independently integrate exponential activity in
   Python Decimal at 80-digit precision for every window. A power series for
   `(1-exp(-x))/x` avoids cancellation for small `x`. Compare with the public
   GGEMS planner using the compiled half-life and the actual rounded activity.
   The relative numerical threshold is 5e-14. A separate planner initialized with
   the same seed replays planning without accessing or advancing Run's streams;
   every production count and group range must match it.
3. **Poisson populations.** Use equal-tail Poisson prediction intervals per group
   and window, per-window totals, and the total horizon. A further 128 host-only
   planning replicas use seeds `seed+1` through `seed+128`, with independent
   persistent group streams. Discrete ECDF tests check repeated population shapes
   at each fixed mean. For means at least 100, a two-sided Pearson dispersion
   check uses one degree of freedom per independent count and no fitted
   parameters. Low-mean groups retain the exact tests; no normal approximation is
   forced. CPU and GPU with the same seed share identical host populations and
   must not be pooled as independent population experiments.
4. **Birth times.** For each window, test the ECDF against
   `(1-exp(-lambda*(t-t0)))/(1-exp(-lambda*(t1-t0)))`. All observed times must obey
   `t0 <= t < t1`. The pre-existing Source CDF budget of 1e-5, the high-24-bit
   uniform quantum and one picosecond's conditional probability mass are reported
   separately from sampling uncertainty. Existing deterministic host/OpenCL
   `GGEMSRadioactiveTimeSampling` tests establish raw-zero lower-bound inclusion,
   maximum-word stop exclusion and the small-decay threshold behavior. Random
   samples alone cannot prove that an endpoint is reachable.
5. **Energy shape.** Test the full observed ECDF against both the selected
   scientific reference and the exact finite-ticket law of the compiled table.
   The integer CDF inverts `floor(width * local_ticket / bin_ticket_count)` using
   Python exact integers. Every sample must be reachable by at least one raw
   word. Continuous-reference tests do not add the observed grid discrepancy to
   their acceptance threshold. A non-rejection at this sample size is not proof
   that historical and selected tables are identical.
6. **RNG and transport scope.** Production Source draw order is unchanged. The
   Point/Fixed case draws birth time first, then conditional energy; Mono needs
   no energy word. Replay and population replicas use separate host engines.
   Existing full-suite progression tests cover production order and supported
   engines. Multiple workers/devices preserve the stream contract, but atomic
   worker assignment does not promise per-history bitwise identity across
   different device schedules. No physical radioactive decay chain, positron
   annihilation, stopping power, navigation or dose behavior is certified here.

Thresholds are fixed in `settings.json` **before production sampling**. The
family significance level defaults to 0.01 with conservative Bonferroni divisor
`windows*(2*groups+3)+5*groups+1`; unused hypotheses do not relax it. ECDF tests use
the [DKW-Massart bound](https://doi.org/10.1214/aop/1176990746), including discrete
CDF left and right limits. Poisson tail/quantile calculations use
[SciPy's Poisson distribution](https://docs.scipy.org/doc/scipy/reference/generated/scipy.stats.poisson.html).
The large-mean dispersion approximation follows the independent-Poisson case
documented for [Pearson's test](https://docs.scipy.org/doc/scipy/reference/generated/scipy.stats.chisquare.html).
No campaign seed or threshold is adjusted after an unfavorable result. Empty
groups are `insufficient_samples`, not `pass`.

## Evidence and quality

Each campaign retains:

- `settings.json`: design, activity, horizons, seeds, commands, versions and all
  statistical policies fixed before sampling;
- `run/definition.json`, `run/group_N.csv`: compiled nuclear and exact table data;
- `run/run.json`: devices, build, chronology, window populations and capture
  counters;
- `run/populations.csv`: observed counts and GGEMS planner expectations, with
  replica, seed, window and group identities;
- `run/samples.csv`: all generated Source births and exact integer energies;
- `analysis.json`: separate checks, thresholds, distances and discrepancies;
- `plots/`: standalone PNGs of decay populations, birth times and group spectra;
- complete invocation logs, including failed attempts.

The campaign command returns zero when evidence generation and analysis complete;
scientific failures remain explicit in `analysis.json`. Exceptions or incomplete
export fail the command. Figures are descriptive and do not introduce additional
acceptance tests. Reanalysis uses retained settings and never redraws samples.

Run the repository's `tests`, `clangd-check.py`, Ruff, BasedPyright and Doxygen
workflows on the candidate. New Python tests are also registered as one CTest
entry. The normal Doxygen input excludes `validation/`; this README and reference
provenance are ordinary Markdown. Matplotlib's public API is narrowed at a typed
boundary, and a single local missing-SciPy-stubs diagnostic is documented at its
import; no project-wide type-checking policy is weakened.
