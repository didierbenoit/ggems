# O-15 selected reference

Select X. Mougeot's CEA/LNE-LNHB evaluation, with tables dated March 3,
2026. Its published seconds value determines the half-life; the slightly
different LARA value does not replace it. The model is one positron group
with the evaluated physical yield.

Select BetaShape 2.4 (06/2024) `dN/dE calc.` from the retained transition
output. No experimental shape factor is tabulated. `fixint=1` preserves
the adopted EC/beta-plus split. The full spectrum is retained without
endpoint rescaling or adjustment to force a reported mean.

The 511 keV annihilation signature belongs to subsequent positron transport.
No EC placeholder, alternative MIRD atomic-relaxation groups, neutrino or
daughter recoil is included. Source-comment generator provenance is audited
separately from numerical spectrum agreement.

Retained BetaShape command (not rerun):

```text
.\betashape.exe O15\O-15.txt fixint=1 -csv
```

The CSV law is piecewise linear and normalized only for conditional energy.
Physical yields remain separate; the finite compiled grid is compared with
the full selected reference. Pointwise uncertainty columns are retained,
but the current analysis compares central densities without covariance.

Complete evidence remains in [raw/](../raw/). Use [reference.json](reference.json)
with the [generic validation commands](../../../README.md); the JSON contains
only machine-consumed inputs.
