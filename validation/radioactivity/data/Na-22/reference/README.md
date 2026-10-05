# Na-22 selected reference

Selected authority: CEA/LNE-LNHB, M. Galan, CIEMAT, 2009.

Both positron branches use BetaShape 2.4 `dN/dE exp.`. The 546.45 keV
branch uses `1 - 0.005*W` (measured 70-520 keV); the 1821.02 keV branch
uses `q^4 + (10/3)*l_2*q^2*p^2 + l_3*p^4` (660-1660 keV).
Full tabulated support, including extrapolation, is retained. `fixint=1`
preserves evaluated absolute branch yields; neither yield is normalized
across groups. Source annihilation photons are excluded.

The evaluation defines 1 y as 365.24219878 d: use this exact factor and
86400 s/d for 2.6029 y, not rounded LARA seconds. The selected photon
energy is LARA/table 1274.537 keV; the retained daughter-level value is
1274.577 keV. These are not forced to agree. Identical 0.8486 keV X-ray
energies are merged by summing absolute yields. Pair-conversion data give
an average energy, not a complete conditional law, and are excluded.
ENSDF's 2.6018 y and MIRD's positron yield 0.898986022 differ from the
selected evaluation and are not averaged into it.

Retained BetaShape 2.4 command (not rerun):

```text
.\betashape.exe .\Na22\Na-22.txt fixint=1 -csv
```

CSV energy, selected density and adjacent uncertainty tokens retain full
support unchanged. Only the conditional piecewise-linear law is normalized;
physical yields remain separate.

Complete evidence remains in [raw/](../raw/). Unsupported Auger energy
laws, neutrinos, recoil and additional radioactive daughter decays are
excluded. See the [generic validation instructions](../../../README.md).
