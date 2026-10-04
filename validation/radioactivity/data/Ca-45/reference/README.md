# Ca-45 selected reference

Select the LNHB evaluation by M.-M. Be (February 2012; April 2012 tables).
Convert days with exactly 86400 s/d. Preserve both positive beta-minus
branches without forcing their rounded intensity sum to one.

Use BetaShape 2.4 (06/2024) `dN/dE calc.` for both branches; no experimental
factor is tabulated. Keep the weak 245.6 keV branch at its evaluated yield.
Its lack of stochastic observations is not a reason to remove it.

The LNHB tables explicitly give K-conversion electrons at 7.90 keV with
0.0017% absolute intensity. Include this Mono group even though LARA lists
no lines and PenNuc exports zero conversion yields. Do not derive additional
gamma or atomic groups from those incomplete exports. The retained ENSDF
endpoint differs from the selected LNHB evaluation and is not substituted.

Retained BetaShape command (not rerun):

```text
.\betashape.exe .\Ca45\Ca-45.txt -csv
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
