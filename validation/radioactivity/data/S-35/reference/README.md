# S-35 selected reference

Select the LNHB evaluation by V. P. Chechev and M.-M. Be (January 2012).
Convert the evaluated days with exactly 86400 s/d. Model one beta-minus
Electron group with unit physical yield.

Use the full BetaShape 2.4 (06/2024) 167.33 keV `dN/dE calc.` spectrum;
no experimental shape factor is tabulated. No ENSDF file was retained for
this package. MIRD is an independent cross-check, not a replacement law.

Retained BetaShape command (not rerun):

```text
.\betashape.exe .\S35\S-35.txt -csv
```

CSV energy, selected density and adjacent uncertainty tokens retain the full
raw support. The piecewise-linear law is normalized only for conditional
energy; physical yields remain separate. The current analysis uses central
densities without an uncertainty covariance model.

Unsupported Auger energy laws, neutrinos, recoil particles and additional
daughter decays are excluded. EC probabilities are not emitted particles.
Selected groups are independent marginals, without cascade correlations.
ENSDF (when retained) and MIRD are cross-checks only and are not averaged
with the selected evaluation.

Complete evidence remains in [raw/](../raw/). Use [reference.json](reference.json)
with the [generic validation commands](../../../README.md). The JSON contains
only machine-consumed inputs.
