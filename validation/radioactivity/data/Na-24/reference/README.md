# Na-24 selected reference

Select the LNHB evaluation by V. P. Chechev and N. K. Kuzmenko (June
2014), converting hours with exactly 3600 s/h. Keep four independent beta-minus
branches, absolute LARA photon lines and PenNuc conversion groups. Identical
canonical line energies are combined by adding their absolute yields.

Use BetaShape 2.4 (06/2024). The dominant 1392.721 keV branch uses `dN/dE exp.`:
the retained factor is `1 - 0.011*W`, measured over 100-1390 keV (Genz et al.,
1976). Its approximately 552.788 keV mean differs from both the calculated
553.768 keV mean and the older LNHB tabular mean; these are not averaged.
The other branches use `dN/dE calc.`. Weak forbidden branches carry Xi
approximation warnings. Preserve the retained endpoint precision and full
support. Exclude the listed 511 keV pair-annihilation radiation and unsupported
internal-pair energy laws.

Retained BetaShape command (not rerun):

```text
.\betashape.exe .\Na24\Na-24.txt -csv
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
