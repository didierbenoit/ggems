# Ru-106 selected reference

Selected authority: CEA/LNE-LNHB, A. Arinc, NPL, April 2013.

The sole beta-minus branch uses `dN/dE calc.` and physical yield one.
Its full retained support ends at 39.40 keV. The selected BetaShape mean
is about 9.85 keV; the older LNHB/ENSDF/MIRD tabular mean near 10.03 keV
does not replace this conditional law.

Rh-106 ground-state decay after about 30 s is a separate radioactive
decay and is excluded with all subsequent chain emissions. ENSDF's
371.8 d differs from selected LNHB's 371.5 d; no values are averaged.

Retained BetaShape 2.4 command:

```text
.\betashape.exe .\Ru106\Ru-106.txt -csv
```

CSV energy, selected density and adjacent uncertainty tokens are copied
unchanged over full support. Validation conditionally normalizes the
piecewise-linear law; physical yields remain separate.

Complete evidence remains in [raw/](../raw/). Unsupported Auger energy
laws, neutrinos and recoil particles are outside this source model.
See the [generic validation instructions](../../../README.md) for running
this `reference.json`. Numerical reproduction of a selected reference is
not a new evaluation of its nuclear-data uncertainty.
