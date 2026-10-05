# Sm-153 selected reference

Select the LNHB/INEEL evaluation (2006), converting 1.92855 d with
exactly 86400 s/d. Keep all 18 positive beta-minus branches and select
BetaShape 2.4 `dN/dE calc.` throughout; no experimental factor is retained.
Use each retained spectrum endpoint, including 43.8 keV for the weakest
branch, without substituting rounded PenNuc endpoints. Prompt photons and
conversion groups retain LNHB absolute intensities. Auxiliary Sm-153 IT
evidence does not redefine this ground-state parent. Rare branches remain
physical, even when a stochastic campaign cannot populate them adequately.

Retained BetaShape command (not rerun):

```text
.\betashape.exe .\Sm153\Sm-153.txt -csv
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
