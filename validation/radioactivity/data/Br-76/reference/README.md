# Br-76 selected reference

Select the LNHB evaluation by A. L. Nichols (2015), its 2016 tables and
April 2017 erratum. Convert the adopted 16.1 h half-life using exactly
3600 s/h. The EC/beta-plus definition retains each positive positron branch,
absolute LARA photon yields, and PenNuc conversion lines grouped by transition.
The retained `fixint=1` preserves the evaluated EC/beta-plus intensities.

Use BetaShape 2.4 (06/2024) `dN/dE calc.` for all selected branches; no
experimental factor is tabulated. Several forbidden non-unique transitions
use the Xi approximation with an explicit unpredictability warning. Reproducing
these laws does not experimentally establish their shape accuracy. The LNHB
comments also discuss a measured positron total that differs from the adopted
branch sum; it is not substituted or averaged. Source-level annihilation
photons and unsupported internal-pair energy laws are excluded.

The 3941 keV branch uses bins just below 1 keV: the usual approximately
0.5 keV grid gives its positive last bin no 32-bit ticket. The coarser grid
preserves the endpoint and makes every positive bin reachable. Other branches
use the usual grid. Both are compared with the full unmodified CSV law.

Retained BetaShape command (not rerun):

```text
.\betashape.exe .\Br76\Br-76.txt fixint=1 -csv
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
