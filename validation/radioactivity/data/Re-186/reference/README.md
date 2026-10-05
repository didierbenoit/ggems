# Re-186 selected reference

Select the LNHB/PTB evaluation (2004), converting 3.7186 d with exactly
86400 s/d. Keep the four positive beta-minus branches of this beta-minus/EC
parent. Select BetaShape 2.4 `dN/dE exp.` for the 1069.5 and 932.3 keV
branches, with `0.034*q^2 + 0.034*p^2 + 1` and
`0.038*q^2 + 0.038*p^2 + 1` (measurement ranges 200-970 and 200-860 keV).
The other branches use `dN/dE calc.`. The dominant non-unique forbidden
calculated laws use the Xi/allowed approximation and carry unpredictability
warnings; reproducing the selected measured-factor law does not validate
that approximation. EC creates no particle. Supported prompt W/Os photons
and conversion electrons use LNHB absolute yields. No beta-plus split
requires `fixint=1`.

Retained BetaShape command (not rerun):

```text
.\betashape.exe .\Re186\Re-186.txt -csv
```

CSV energy, density and adjacent uncertainty tokens retain full raw support.
The piecewise-linear law is normalized only for conditional energy; physical
yields remain separate. Pointwise uncertainties do not provide covariance
for an evaluation-equivalence test.

Unsupported Auger energy laws, neutrinos, recoil and additional radioactive
daughter decays are excluded. Selected groups are independent marginals;
ENSDF and MIRD are cross-checks only and are not averaged with LNHB.

Complete evidence remains in [raw/](../raw/). Use
[reference.json](reference.json) with the
[generic validation commands](../../../README.md).
