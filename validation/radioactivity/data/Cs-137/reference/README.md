# Cs-137 selected reference

Selected authority: CEA/LNE-LNHB, S. Leblond, July 2023.

Use `dN/dE exp.` for the 1175.63 keV ground-state branch: the retained
Behrens/Christmas (1983) experimental law has mean about 284.90 keV,
whereas the calculated approximation gives about 422.07 keV. Its measurement
range is 710-1125 keV; no closed-form factor is invented. The two other
branches use `dN/dE calc.`. ENSDF/MIRD alternatives are not averaged in.

The evaluation explicitly uses 365.242198 days per year. Thus 30.018(22) y
is converted with that factor and exact 86400 s/day, rather than using
the rounded LARA seconds field.

Ba-137m de-excitation after about 2.5545 min, including the 661.6553 keV
photon, is excluded. Compact atomic totals and the weak 283.46 keV
photon/conversion law contain inseparable prompt and isomer-fed contributions
and are excluded as mixed laws. The three beta branches remain intact.

The experimental branch uses regular bins up to 2 keV to keep every
positive tail bin reachable with 32-bit tickets; its endpoint is unchanged.
Other branches use the usual approximately 0.5 keV grid. Full reference
support remains in the CSV, independently of finite-grid residuals.

Retained BetaShape 2.4 command:

```text
.\betashape.exe .\Cs137\Cs-137.txt -csv
```

CSV energy, selected density and adjacent uncertainty tokens are copied
unchanged over full support. Validation conditionally normalizes the
piecewise-linear law; physical yields remain separate.

Complete evidence remains in [raw/](../raw/). Unsupported Auger energy
laws, neutrinos and recoil particles are outside this source model.
See the [generic validation instructions](../../../README.md) for running
this `reference.json`. Numerical reproduction of a selected reference is
not a new evaluation of its nuclear-data uncertainty.
