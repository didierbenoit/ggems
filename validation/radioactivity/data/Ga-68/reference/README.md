# Ga-68 selected reference

Select the LNHB/PTB evaluation by M.-M. Be and E. Schonfeld, reviewed
November 2011. Convert its evaluated minutes using exactly 60 s/min.
LNHB/LARA supplies beta yields, nuclear photons and compact X rays;
PenNuc supplies conversion electrons. Only the nine detailed Auger entries
use the retained MIRD Summary Spectrum CSV, because LNHB gives grouped
Auger ranges. These are selected marginals from different atomic models.

Use BetaShape 2.4 (06/2024): `dN/dE exp.` for the dominant 1899.1 keV
branch (Slot et al., 1972, `1 - 0.01*W`), and `dN/dE calc.` for the other
two branches. The retained `-fixint=1` preserves the evaluated EC/beta-plus
split. The two calculated energy axes retain their established mappings
821.8 -> 821.75 and 243.2 -> 243.23 keV; density and uncertainty tokens
remain as supplied. The Auger line energies retain the historical nearest-meV
mapping, with residuals no larger than 400 micro-eV.

No source annihilation photons, EC placeholder, neutrino or recoil is added.
The possible 1655.87 keV E0 transition has no selected direct photon yield.

Retained BetaShape command (not rerun):

```text
.\betashape.exe Ga68\Ga-68.txt -fixint=1 -csv
```

The CSV law is piecewise linear and normalized only for conditional energy.
Physical yields remain separate; the finite compiled grid is compared with
the full selected reference. Pointwise uncertainty columns are retained,
but the current analysis compares central densities without covariance.

Complete evidence remains in [raw/](../raw/). Use [reference.json](reference.json)
with the [generic validation commands](../../../README.md); the JSON contains
only machine-consumed inputs.
