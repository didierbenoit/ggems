# I-124 selected reference

Select B. E. Zimmerman's LNE-LNHB/NIST DDEP evaluation, updated in 2021.
Convert the evaluated days using exactly 86400 s/d. LNHB supplies beta-plus
yields, LARA nuclear photons and compact X rays, and PenNuc conversion
electrons. Only the 13 detailed Auger entries use the retained MIRD Summary
Spectrum CSV, with the historical nearest-meV energy mapping.

BetaShape 2.4 (06/2024) `fixint=1` preserves the evaluated EC/beta-plus
splits. Select `dN/dE exp.` for the 2137.6 and 1534.9 keV branches, using
Booij et al. (1971) factors; select `dN/dE calc.` for the other six branches.
Every CSV retains the full transition support. The 812.1 keV non-unique
forbidden law uses the Xi approximation and carries an unpredictable-shape
warning; it is not an experimentally established spectrum.

The existing main/weak conversion groups partition all positive lines at
absolute yield 1e-9 without changing physical intensities. This is a numerical
partition, not a decay mode. No source annihilation photon, EC placeholder,
internal-pair group or recoil is included. Subsequent daughter decays are
outside the prompt source model.

Retained BetaShape command (not rerun):

```text
.\betashape.exe I124\I-124.txt fixint=1 -csv
```

The CSV law is piecewise linear and normalized only for conditional energy.
Physical yields remain separate; the finite compiled grid is compared with
the full selected reference. Pointwise uncertainty columns are retained,
but the current analysis compares central densities without covariance.

Complete evidence remains in [raw/](../raw/). Use [reference.json](reference.json)
with the [generic validation commands](../../../README.md); the JSON contains
only machine-consumed inputs.
