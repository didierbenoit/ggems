# Re-188 selected reference

Select the LNHB/LBNL evaluation (1998), converting 17.005 h with
exactly 3600 s/h. Keep all 24 positive ground-state beta-minus branches.
Select BetaShape 2.4 `dN/dE exp.` for 2120.4 and 1965.36 keV, with
`0.039*q^2 + 0.039*p^2 + 1` and `0.031*q^2 + 0.031*p^2 + 1`
(measurement ranges 210-1970 and 210-1840 keV); other branches use
`dN/dE calc.`. Dominant non-unique calculated laws carry Xi/allowed
approximation warnings. Agreement with the selected experimental factors
does not establish that theoretical approximation. Retain absolute prompt
LNHB photon/conversion yields; the auxiliary Re-188m IT file does not
redefine the ground-state parent. Rare branches are not amplified.

Retained BetaShape command (not rerun):

```text
.\betashape.exe .\Re188\Re-188.txt -csv
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
