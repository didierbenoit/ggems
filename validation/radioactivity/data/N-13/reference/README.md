# N-13 selected reference

Select X. Mougeot's LNHB evaluation (February 2026; March 2026 tables).
Convert minutes with exactly 60 s/min. The model contains one Positron group
with yield 0.99803; the evaluated EC fraction 0.00197 creates no primary.
The retained `fixint=1` preserves this split.

Use BetaShape 2.4 (06/2024) `dN/dE exp.`. Its retained factor is
`1 + 0.0014*W`, measured over 100-1000 keV (Daniel and Schmidt-Rohr, 1958).
The experimental-shape mean near 492.21 keV is distinct from the calculated
490.78 keV mean; the current LNHB evaluation adopts the experimental result.
Do not add source-level 511 keV photons: annihilation belongs to transport.

Retained BetaShape command (not rerun):

```text
.\betashape.exe .\N13\N-13.txt fixint=1 -csv
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
