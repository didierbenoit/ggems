# H-3 selected reference

LNHB/DDEP, V. P. Chechev, April 2006, supplies the nuclear selection.
The half-life uses LARA's published seconds directly; no new year conversion
is substituted. The model is one beta-minus Electron group to stable He-3,
with unit yield and no selected prompt photon or atomic-relaxation group.

The selected BetaShape 2.4 (06/2024) column is `dN/dE exp.`, using Piel's
1973 experimental factor `Cexp(W)=1`. It differs from the calculated column.
The full 0-18.591 keV Q-derived spectrum is retained; it is not clipped to the
separately evaluated atomic electron endpoint of 18.564 keV or claimed to
describe every chemical form of tritium. The measured factor covers 11-18 keV.
Neutrinos, recoil and placeholder particles are outside this source model.

Retained BetaShape command (not rerun):

```text
.\betashape.exe H3\H-3.txt -csv
```

The CSV law is piecewise linear and normalized only for conditional energy.
Physical yields remain separate; the finite compiled grid is compared with
the full selected reference. Pointwise uncertainty columns are retained,
but the current analysis compares central densities without covariance.

Complete evidence remains in [raw/](../raw/). Use [reference.json](reference.json)
with the [generic validation commands](../../../README.md); the JSON contains
only machine-consumed inputs.
