# Cs-134 selected reference

Select the retained LNHB evaluation (M. M. Be, February 2012) for
ground-state Cs-134 beta-minus/EC decay. Convert the tabulated 2.0644(14) y
using the evaluation's exact `365.24219878 d/y` and `86400 s/d`; do not mix
this with the rounded 754.0(5) d or LARA seconds fields. The separate
2.912 h Cs-134m parent is excluded.

Select four BetaShape 2.4 laws: `dN/dE exp.` for 658.39 and 415.64 keV
(`Cexp(W)=1`, measured over 415-658 and 90-415 keV respectively), and
`dN/dE calc.` for 1454.26 and 89.06 keV. The 1454.26 keV second-forbidden
non-unique branch is approximated as first-forbidden unique; BetaShape calls
its Xi approximation unpredictable. Numerical reproduction does not validate
that nuclear approximation. Preserve the independently rounded branch yields.

Retain supported prompt Ba photons/conversion and the tiny Xe gamma from EC.
The retained compact atomic law is Ba-only; no Xe atomic yield is invented
from capture probabilities. EC creates no primary.

Retained BetaShape command (not rerun):

```text
.\betashape.exe .\Cs134\Cs-134.txt -csv
```

CSV energy, selected density and adjacent uncertainty tokens preserve the
full tabulated support. Only the conditional law is normalized for sampling.

ENSDF and MIRD are independent cross-checks only. Unsupported grouped Auger
energy laws are excluded; compact LARA X-ray energies retain its convention.

Neutrinos, recoil and additional radioactive daughter decays are excluded.
Complete retained evidence remains in [raw/](../raw/). Use
[reference.json](reference.json) with the
[generic validation instructions](../../../README.md).
