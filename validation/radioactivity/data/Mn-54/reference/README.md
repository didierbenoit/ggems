# Mn-54 selected reference

Selected authority: CEA/LNE-LNHB, KRI, 2014.

Select the retained `fixint=1` positron law with yield 5.7E-9 per
parent decay and BetaShape 2.4 `dN/dE calc.` at 355.2 keV. The published
LNHB table marks 5.7E-7 percent as an upper limit; the retained PenNuc/input
and requested fixed source model use that boundary value. It must not be
interpreted as a measured central intensity or replaced by a recalculated
BetaShape split. Zero stochastic observations are expected at the standard
campaign exposure; deterministic spectrum checks still apply.

Retain prompt Cr X rays, the 834.848 keV photon and its conversion lines.
The level energy is 834.855 keV and its lifetime is 7.9 ps. Source
annihilation photons and an unselected beta-minus mode are excluded.
Independent ENSDF/MIRD alternatives do not redefine the fixed weak yield.

Retained BetaShape 2.4 command (not rerun):

```text
.\betashape.exe .\Mn54\Mn-54.txt fixint=1 -csv
```

CSV energy, selected density and adjacent uncertainty tokens retain full
support unchanged. Only the conditional piecewise-linear law is normalized;
physical yields remain separate.

Complete evidence remains in [raw/](../raw/). Unsupported Auger energy
laws, neutrinos, recoil and additional radioactive daughter decays are
excluded. See the [generic validation instructions](../../../README.md).
