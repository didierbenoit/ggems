# C-11 selected reference

The selected LNHB/DDEP evaluation is V. Chiste and M.-M. Be (2002), with
the 2011 half-life update. Convert the evaluated minutes using exactly
60 s/min instead of substituting the rounded LARA seconds. The selected
source is one positron group to stable B-11.

Select BetaShape 2.4 (06/2024) `dN/dE exp.`, with the Behrens et al. (1975)
factor `1 - 0.0074*W`, measured over 266-892 keV. The full transition law is
retained. `fixint=1` preserves the evaluated EC/beta-plus split; the historical
LNHB mean and calculated BetaShape mean do not override this selection.

LNHB capture probabilities do not specify emitted Auger yields. The selected
emission tables supply no boron X-ray/Auger marginals, and alternative MIRD
atomic data are not imported. This does not assert absence of atomic
relaxation. No EC placeholder or source annihilation photons are included.

Retained BetaShape command (not rerun):

```text
.\betashape.exe C11\C-11.txt fixint=1 -csv
```

The CSV law is piecewise linear and normalized only for conditional energy.
Physical yields remain separate; the finite compiled grid is compared with
the full selected reference. Pointwise uncertainty columns are retained,
but the current analysis compares central densities without covariance.

Complete evidence remains in [raw/](../raw/). Use [reference.json](reference.json)
with the [generic validation commands](../../../README.md); the JSON contains
only machine-consumed inputs.
