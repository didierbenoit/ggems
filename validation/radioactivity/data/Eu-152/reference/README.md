# Eu-152 selected reference

Selected authority: CEA/LNE-LNHB, V. R. Vanin, R. M. de Castro and E. Browne, USP/LBNL, 2004.

Keep thirteen beta-minus and two beta-plus branches separate, followed
by prompt Sm/Gd nuclear photons, absolute compact X rays and conversion
groups. Physical yields are independent marginals, not a categorical CDF.
`fixint=1` preserves the evaluated EC/beta-plus split. Auxiliary Eu isomer
parents, capture primaries and source annihilation photons are excluded.

Select `dN/dE exp.` for the 1474.5 keV beta-minus branch:
`q^2 + 0.79*p^2 + 5`, measured 1100-1450 keV. Other branches use
`dN/dE calc.`. Retained forbidden non-unique Xi warnings remain a
nuclear-model limitation; reproducing these laws is not experimental
shape validation. The 485.8 keV positron branch needs bins up to 2 keV
for positive-bin ticket reachability; other grids remain near 0.5 keV.
The continuous CSV is unchanged and retains full support.

Use the commentary's directly recommended 4939(6) d, converted with
86400 s/d; 13.522(16) y is its alternate rounded representation.
ENSDF gives 13.517 y and 27.92% beta-minus, versus LNHB's 27.9%.
The retained branch sum is not forced to that rounded mode total.
MIRD's positron total 0.0001381379 differs from the selected 0.000274.
Unplaced LNHB parent gamma lines are retained; distinct conversion
transitions sharing a photon energy keep separate shell laws.

Retained BetaShape 2.4 command (not rerun):

```text
.\betashape.exe .\Eu152\Eu-152.txt fixint=1 -csv
```

CSV energy, selected density and adjacent uncertainty tokens retain full
support unchanged. Only the conditional piecewise-linear law is normalized;
physical yields remain separate.

Complete evidence remains in [raw/](../raw/). Unsupported Auger energy
laws, neutrinos, recoil and additional radioactive daughter decays are
excluded. See the [generic validation instructions](../../../README.md).
