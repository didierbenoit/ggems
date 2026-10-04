# Mn-52 selected reference

Select A. Luca's LNHB/IFIN-HH evaluation, November 2020. Convert
evaluated days using exactly 86400 s/d. Keep five positron branches in
increasing endpoint order, LARA nuclear photons and compact X rays, and
PenNuc conversion electrons grouped by nuclear transition.

Select BetaShape 2.4 (06/2024) `dN/dE calc.` for all five full-support
transition spectra. No experimental factor is tabulated. `fixint=1`
preserves the evaluated EC/beta-plus splits; their rounded combined sum
is not forced to one. The 1320.3, 922.2 and 274.7 keV second-forbidden
non-unique transitions use first-forbidden unique laws under the Xi
approximation and carry unpredictable-shape warnings. Agreement does not
establish their experimental accuracy. Source annihilation photons are
excluded because annihilation belongs to subsequent positron transport.

LNHB's nonzero Auger totals have only grouped energy ranges, without a
complete selected conditional law, so Auger groups are excluded. MIRD is
an independent cross-check and is not substituted. No source-level 511 keV
line, capture/pair placeholder, unsupported internal-pair law, neutrino,
recoil, later daughter decay or unobserved upper-limit branch is added.
These are independent prompt marginals, without cascade correlations or
daughter-level delays.

Retained BetaShape command (not rerun):

```text
.\betashape.exe .\Mn52\Mn-52.txt fixint=1 -csv
```

The CSV law is piecewise linear and normalized only for conditional energy.
Physical yields remain separate; the finite compiled grid is compared with
the full selected reference. Pointwise uncertainty columns are retained,
but the current analysis compares central densities without covariance.

Complete evidence remains in [raw/](../raw/). Use [reference.json](reference.json)
with the [generic validation commands](../../../README.md); the JSON contains
only machine-consumed inputs.
