# Cu-67 selected reference

Select X. Mougeot's CEA/LNE-LNHB evaluation, February 2026. Convert
evaluated hours using exactly 3600 s/h. The beta-minus model retains four
branches in increasing endpoint order, followed by LARA nuclear photons
and compact X rays, then PenNuc conversion groups by nuclear transition.

Use BetaShape 2.4 (06/2024) `dN/dE calc.` for all four transition spectra;
none has a tabulated experimental shape factor. Physical branch yields use
the more precise probabilities in LNHB Table 4 rather than the rounded
percentages. Spectrum areas are normalized only for conditional energy;
they do not replace or renormalize the evaluated physical yields.

LNHB's nonzero Auger totals have only grouped energy ranges, without a
complete selected conditional law, so Auger groups are excluded. MIRD is
an independent cross-check and is not substituted. No source-level 511 keV
line, capture/pair placeholder, unsupported internal-pair law, neutrino,
recoil, later daughter decay or unobserved upper-limit branch is added.
These are independent prompt marginals, without cascade correlations or
daughter-level delays.

Retained BetaShape command (not rerun):

```text
.\betashape.exe .\Cu67\Cu-67.txt -csv
```

The CSV law is piecewise linear and normalized only for conditional energy.
Physical yields remain separate; the finite compiled grid is compared with
the full selected reference. Pointwise uncertainty columns are retained,
but the current analysis compares central densities without covariance.

Complete evidence remains in [raw/](../raw/). Use [reference.json](reference.json)
with the [generic validation commands](../../../README.md); the JSON contains
only machine-consumed inputs.
