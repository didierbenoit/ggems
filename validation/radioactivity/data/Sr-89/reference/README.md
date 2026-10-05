# Sr-89 selected reference

Select the LNHB/PTB evaluation (1999, half-life update 2002), converting
50.57 d with exactly 86400 s/d. Keep both positive beta-minus branches.
For the dominant branch select BetaShape 2.4 `dN/dE exp.` with
`(1 - 0.0112*W) * (q^2 + l_2*p^2)`, measured over 75-1400 keV; use
`dN/dE calc.` for the weak branch. Retain the full tabulated laws.

The 909 keV Y-89m level has a retained 16.05 s half-life. Its gamma and
conversion electrons are excluded from the parent-time source. The
0.00086-per-decay K-X intensity is principally beta-induced internal
ionization (LNHB commentary, section 4.2); it is not inferred from the
much smaller delayed conversion yield. Keep LARA's compact 14.9585 keV
representation, with its limitation: the measurement is a K-X total,
not a separately measured K-alpha-1 line. The commentary quotes a delayed
conversion contribution of 5.1e-7 per decay, far below the 7e-5 measured
uncertainty; the retained compact intensity does not resolve that component.

Retained BetaShape command (not rerun):

```text
.\betashape.exe .\Sr89\Sr-89.txt -csv
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
