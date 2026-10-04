# C-14 selected reference

LNHB/DDEP, M.-M. Be and V. P. Chechev, January 2012, supplies the nuclear
selection. The half-life uses LARA's published seconds directly. The model
is one allowed beta-minus Electron group to stable N-14, with unit yield.

Select BetaShape 2.4 (06/2024) `dN/dE exp.` with the Singh et al. (2023)
factor `1 - 0.00043*me*(W-1)`, measured over 25-156 keV. The CSV retains the
full 0-156.476 keV law. Neither the older LNHB mean nor the calculated
BetaShape column replaces this experimental-shape selection. No prompt
gamma, atomic-relaxation, recoil, antineutrino or placeholder group is added.

Retained BetaShape command (not rerun):

```text
.\betashape.exe C14\C-14.txt -csv
```

The CSV law is piecewise linear and normalized only for conditional energy.
Physical yields remain separate; the finite compiled grid is compared with
the full selected reference. Pointwise uncertainty columns are retained,
but the current analysis compares central densities without covariance.

Complete evidence remains in [raw/](../raw/). Use [reference.json](reference.json)
with the [generic validation commands](../../../README.md); the JSON contains
only machine-consumed inputs.
