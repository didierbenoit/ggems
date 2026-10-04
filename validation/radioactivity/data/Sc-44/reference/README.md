# Sc-44 selected reference

Select E. Browne's LNHB/LBNL evaluation, November 2001. Convert
evaluated hours using exactly 3600 s/h. The EC/beta-plus model contains one
positron group, LARA nuclear photons and compact X rays, and PenNuc conversion
groups by nuclear transition. The newer ENSDF half-life is a cross-check,
not a replacement for the selected LNHB evaluation.

Use BetaShape 2.4 (06/2024) `dN/dE calc.`; no experimental shape factor is
tabulated. The retained `fixint=1` preserves the evaluated EC/beta-plus
split. Keep the full 1474.3 keV transition support instead of stretching it
to the extra digits in PenNuc. Source annihilation photons are excluded
because annihilation belongs to subsequent positron transport.

LNHB's nonzero Auger totals have only grouped energy ranges, without a
complete selected conditional law, so Auger groups are excluded. MIRD is
an independent cross-check and is not substituted. No source-level 511 keV
line, capture/pair placeholder, unsupported internal-pair law, neutrino,
recoil, later daughter decay or unobserved upper-limit branch is added.
These are independent prompt marginals, without cascade correlations or
daughter-level delays.

Retained BetaShape command (not rerun):

```text
.\betashape.exe .\Sc44\Sc-44.txt fixint=1 -csv
```

The CSV law is piecewise linear and normalized only for conditional energy.
Physical yields remain separate; the finite compiled grid is compared with
the full selected reference. Pointwise uncertainty columns are retained,
but the current analysis compares central densities without covariance.

Complete evidence remains in [raw/](../raw/). Use [reference.json](reference.json)
with the [generic validation commands](../../../README.md); the JSON contains
only machine-consumed inputs.
