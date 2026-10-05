# Ho-166 selected reference

Select the retained LNHB/NIST evaluation (2024), converting the
ground-state 26.808 h half-life with exactly 3600 s/h. Keep eight positive
beta-minus branches. Select BetaShape 2.4 `dN/dE exp.` for 1853.8 keV
(`1 - 0.87*W - 1/W + 0.22*W^2 - 0.02*W^3`, measured at 410-1790 keV)
and 1773.2 keV (`(1 - 0.105*W) * (q^2 + l_2*p^2)`, 410-1700 keV).
Other branches use `dN/dE calc.`. The first branch has a non-unique
Xi/allowed approximation warning; the second is first-forbidden unique.
Keep LNHB absolute prompt photon/conversion yields. Auxiliary 185-microsecond
IT and long-lived Ho-166m decay files do not redefine this parent.

Retained BetaShape command (not rerun):

```text
.\betashape.exe .\Ho166\Ho-166.txt -csv
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
