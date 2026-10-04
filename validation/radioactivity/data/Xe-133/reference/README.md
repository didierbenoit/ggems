# Xe-133 selected reference

Select the LNHB evaluation by M. Galan (August 2007; April 2008 tables).
Convert the tabulated 5.2474 d central half-life with exactly 86400 s/d.
The comments and tables use inconsistent uncertainty descriptions; this does
not change the selected central value.

Retain three beta-minus groups, absolute LARA photon yields and PenNuc
conversion groups. Use BetaShape 2.4 (06/2024) `dN/dE calc.` for all branches;
no experimental shape factor is tabulated. Preserve the rounded branch sum
without renormalizing physical yields. ENSDF and MIRD remain independent
cross-checks, not alternate inputs to the selected law.

Retained BetaShape command (not rerun):

```text
.\betashape.exe .\Xe133\Xe-133.txt -csv
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
