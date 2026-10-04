# I-131 selected reference

Select the V. Chiste / M.-M. Be CEA/LNE-LNHB evaluation updated in
December 2013, with January 2014 tables. Convert the evaluated days using
exactly 86400 s/d. LARA supplies prompt photons and compact X rays; PenNuc
supplies conversion electrons. Only detailed Auger entries use the retained
MIRD Summary Spectrum CSV and its historical nearest-meV energy mapping.

Use BetaShape 2.4 (06/2024) `dN/dE exp.` for the 606.3 keV branch, with
Daniel et al.'s (1964) `1 + 0.02*W` factor, and `dN/dE calc.` for the other
five branches. The 303.9 and 629.7 keV non-unique forbidden laws use the Xi
approximation and carry unpredictable-shape warnings. No mean is forced to
match the older LNHB tabulation.

The beta branch populating Xe-131m is included, but that level's delayed
163.930 keV gamma and six conversion lines are excluded from the prompt
I-131 source. The quoted delayed gamma intensity applies at a later time,
not parent decay. No daughter chain or additional MIRD nuclear emissions
are added.

Retained BetaShape command (not rerun):

```text
.\betashape.exe I131\I-131.txt -csv
```

The CSV law is piecewise linear and normalized only for conditional energy.
Physical yields remain separate; the finite compiled grid is compared with
the full selected reference. Pointwise uncertainty columns are retained,
but the current analysis compares central densities without covariance.

Complete evidence remains in [raw/](../raw/). Use [reference.json](reference.json)
with the [generic validation commands](../../../README.md); the JSON contains
only machine-consumed inputs.
