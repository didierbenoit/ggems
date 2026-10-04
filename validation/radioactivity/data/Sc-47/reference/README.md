# Sc-47 selected reference

Select X. Mougeot's CEA/LNE-LNHB evaluation, November 2013. Convert
evaluated days using exactly 86400 s/d. The beta-minus model contains two
branches in increasing endpoint order, the LARA Mono nuclear gamma and
compact X rays, and PenNuc conversion electrons.

Use BetaShape 2.4 (06/2024) `dN/dE calc.` for both full-support transition
spectra; neither has a tabulated experimental shape factor. The older LOGFT
mean energies in the LNHB tables do not replace or modify the selected
BetaShape laws.

LNHB's nonzero Auger totals have only grouped energy ranges, without a
complete selected conditional law, so Auger groups are excluded. MIRD is
an independent cross-check and is not substituted. No source-level 511 keV
line, capture/pair placeholder, unsupported internal-pair law, neutrino,
recoil, later daughter decay or unobserved upper-limit branch is added.
These are independent prompt marginals, without cascade correlations or
daughter-level delays.

Retained BetaShape command (not rerun):

```text
.\betashape.exe .\Sc47\Sc-47.txt -csv
```

The CSV law is piecewise linear and normalized only for conditional energy.
Physical yields remain separate; the finite compiled grid is compared with
the full selected reference. Pointwise uncertainty columns are retained,
but the current analysis compares central densities without covariance.

Complete evidence remains in [raw/](../raw/). Use [reference.json](reference.json)
with the [generic validation commands](../../../README.md); the JSON contains
only machine-consumed inputs.
