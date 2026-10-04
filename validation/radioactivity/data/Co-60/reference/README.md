# Co-60 selected reference

Select R. G. Helmer's LNHB/DDEP evaluation, updated by M.-M. Be through
January 2006. The half-life uses LARA's published seconds directly.
LNHB supplies the beta branches, LARA the photons and compact X rays, and
PenNuc the conversion electrons. Only the seven detailed Auger entries
use the retained MIRD Summary Spectrum CSV.

Use BetaShape 2.4 (06/2024) `dN/dE exp.` for the 317.32 and 1490.56 keV
branches (Sastry, 1972, and Wolfson, 1956), and `dN/dE calc.` for the
664.46 keV branch, which has no tabulated experimental factor. Keep each
full-support transition spectrum and its separate physical yield.

Internal-pair formation is excluded: the tabulated total pair kinetic
energy does not specify the electron/positron energy-sharing law. It is
not a monoenergetic source particle. No daughter-chain contribution is added.

Retained BetaShape command (not rerun):

```text
.\betashape.exe Co60\Co-60.txt -csv
```

The CSV law is piecewise linear and normalized only for conditional energy.
Physical yields remain separate; the finite compiled grid is compared with
the full selected reference. Pointwise uncertainty columns are retained,
but the current analysis compares central densities without covariance.

Complete evidence remains in [raw/](../raw/). Use [reference.json](reference.json)
with the [generic validation commands](../../../README.md); the JSON contains
only machine-consumed inputs.
