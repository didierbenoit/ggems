# Rb-82 selected reference

Select the LNHB evaluation by M.-M. Be (January 2015). Convert the
adopted 1.2652 min half-life with exactly 60 s/min; the retained ENSDF value
differs and is not substituted. Model every positive evaluated positron
branch separately, plus supported LARA photons and PenNuc conversion groups.
Zero-yield beta-plus transitions create no source groups.

Use BetaShape 2.4 (06/2024) `dN/dE calc.` for all 13 selected spectra; no
experimental factor is tabulated. The retained `fixint=1` preserves the
evaluated EC/beta-plus splits. Capture probabilities create no primaries;
unsupported atomic energy laws are not inferred from them. Do not add
source-level 511 keV photons or activity from the upstream Sr-82 parent.

Retained BetaShape command (not rerun):

```text
.\betashape.exe .\Rb82\Rb-82.txt fixint=1 -csv
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
