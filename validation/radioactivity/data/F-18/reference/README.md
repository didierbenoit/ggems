# F-18 selected reference

The selected LNHB/KRI evaluation is V. Chiste, M.-M. Be and N. K. Kuzmenko,
updated in 2014. The half-life converts the more precise evaluated hours
using exactly 3600 s/h. The model includes the positron, LNHB L-Auger
electron and two equal-energy oxygen K-alpha lines combined into one photon
group. Their physical yields are independent marginals.

Select BetaShape 2.4 (06/2024) `dN/dE exp.`, with Hofmann's (1964)
`1 + 0.0034*W` factor. The retained `fixint=1` command preserves the evaluated
EC/beta-plus split. Its full-support law extends beyond the measured
70-600 keV factor range.

The known LNHB K-Auger emission is excluded because its 0.456-0.502 keV
range does not define a supported conditional energy law. It is not
physically absent. No alternative MIRD mean or EADL law is substituted.
There is no EC placeholder or source-level 511 keV annihilation photon;
annihilation belongs to subsequent positron transport.

Retained BetaShape command (not rerun):

```text
.\betashape.exe F18\F-18.txt fixint=1 -csv
```

The CSV law is piecewise linear and normalized only for conditional energy.
Physical yields remain separate; the finite compiled grid is compared with
the full selected reference. Pointwise uncertainty columns are retained,
but the current analysis compares central densities without covariance.

Complete evidence remains in [raw/](../raw/). Use [reference.json](reference.json)
with the [generic validation commands](../../../README.md); the JSON contains
only machine-consumed inputs.
