# Co-57 selected reference

Select the V. P. Chechev / N. K. Kuzmenko LNHB/KRI evaluation updated
August 2014. Convert evaluated days using exactly 86400 s/d. This EC source
uses LARA nuclear photons and compact X rays, followed by PenNuc conversion
electrons grouped by nuclear transition. Direct LARA photon intensities are
selected without rescaling them to the different sums in the commentary.

BetaShape 2.4 (06/2024) provides supporting capture information only; no
continuous beta spectrum is selected. Capture probabilities are not emitted
particle yields.

LNHB's nonzero Auger totals have only grouped energy ranges, without a
complete selected conditional law, so Auger groups are excluded. MIRD is
an independent cross-check and is not substituted. No source-level 511 keV
line, capture/pair placeholder, unsupported internal-pair law, neutrino,
recoil, later daughter decay or unobserved upper-limit branch is added.
These are independent prompt marginals, without cascade correlations or
daughter-level delays.

Complete evidence remains in [raw/](../raw/). Use [reference.json](reference.json)
with the [generic validation commands](../../../README.md); the JSON contains
only machine-consumed inputs.
