# Cu-64 selected reference

Select the retained LNHB/INEEL evaluation (2011). Convert 12.7004 h
with exactly 3600 s/h. The selected mixed beta-minus/EC/beta-plus source uses
both retained BetaShape 2.4 `dN/dE calc.` laws. `fixint=1` preserves the
retained input split: the positron yield is 0.1751, whereas the companion
PenNuc/LARA export quotes 0.1752. These are not averaged. Prompt nuclear
photons, compact X rays and conversion lines use absolute LNHB yields.
EC creates no primary; 511 keV annihilation photons belong to transport.

Retained BetaShape command (not rerun):

```text
.\betashape.exe .\Cu64\Cu-64.txt fixint=1 -csv
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
