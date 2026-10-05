# Zn-65 selected reference

Selected authority: CEA/LNE-LNHB, LNHB/INEEL, 2005.

Use the retained 330.1 keV BetaShape 2.4 `dN/dE calc.` positron law
with absolute yield 0.01421. `fixint=1` preserves the evaluated EC/beta-plus
split. Retain prompt Cu nuclear photons, compact X rays and positive
PenNuc conversion lines; zero-intensity shell entries create no lines.
Source annihilation photons and EC placeholder primaries are excluded.
ENSDF/MIRD are independent cross-checks, not replacement shapes or atomic
models. The selected 244.01 d half-life is converted with exact 86400 s/d.

Retained BetaShape 2.4 command (not rerun):

```text
.\betashape.exe .\Zn65\Zn-65.txt fixint=1 -csv
```

CSV energy, selected density and adjacent uncertainty tokens retain full
support unchanged. Only the conditional piecewise-linear law is normalized;
physical yields remain separate.

Complete evidence remains in [raw/](../raw/). Unsupported Auger energy
laws, neutrinos, recoil and additional radioactive daughter decays are
excluded. See the [generic validation instructions](../../../README.md).
