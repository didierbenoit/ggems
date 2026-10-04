# Hg-203 selected reference

Select the LNHB evaluation by A. L. Nichols (January 2004), using its
adopted Woods half-life, 46.594 d, and exactly 86400 s/d. Model two beta-minus
branches, LARA photons and PenNuc conversion lines.

Use BetaShape 2.4 (06/2024) `dN/dE calc.` for both branches; neither has an
experimental factor. The dominant 212.6 keV transition is treated as first
forbidden non-unique with the Xi approximation and an unpredictability warning.
This is distinct from the older evaluation's allowed-shape treatment and mean.
Validation tests reproduction of the selected retained law, not the accuracy
of this approximation. ENSDF's different half-life is a cross-check only.

Retained BetaShape command (not rerun):

```text
.\betashape.exe .\Hg203\Hg-203.txt -csv
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
