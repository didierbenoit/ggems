# Zr-89 selected reference

Select the A. L. Nichols LNHB evaluation (2014), converting 78.42 h
with exactly 3600 s/h. Retain the ground-state EC/beta-plus parent and
BetaShape 2.4 `dN/dE exp.` with `1 - 0.39*W + 0.09*W^2`, measured over
100-840 keV. The retained extrapolation outside that interval remains part
of the selected full-support law. `fixint=1` preserves the EC/beta-plus split.
The raw spectrum ends at 901.8 keV; the input transition quotes 901.83 keV.

The 908.97 keV Y-89m level has a 15.84 s half-life. Its gamma and conversion
electrons are excluded because the source has no daughter-delay model.
Higher-level prompt photons and the 1744.72 keV conversion group remain.
The tabulated Y X-ray totals mix capture and conversion relaxation, including
the delayed transition. No separated absolute prompt spectrum is retained,
so the entire mixed X-ray group is excluded; no prompt fraction is invented.
No Zr-89m parent, capture primary or source annihilation photon is added.

The usual near-0.5-keV grid gives a positive low-energy bin no 32-bit ticket.
Near-1-keV bins preserve the endpoint and make every positive bin reachable.
The unmodified CSV remains the full-support comparison reference.

Retained BetaShape command (not rerun):

```text
.\betashape.exe .\Zr89\Zr-89.txt fixint=1 -csv
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
