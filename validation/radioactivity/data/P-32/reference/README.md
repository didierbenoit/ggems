# P-32 selected reference

Select M.-C. Lepy's CEA/LNE-LNHB evaluation, March 2026. Convert the
evaluated days using exactly 86400 s/d instead of rounded LARA seconds.
The source is one allowed beta-minus Electron group to stable S-32 with
unit physical yield.

Select BetaShape 2.4 (06/2024) `dN/dE exp.`, with Wiesner, Flothmann and
Gils' (1973) factor `1 - 0.019*W`, measured over 200-1635 keV. The full
transition support is retained. LNHB adopts the theoretical 693.306 keV
mean; the selected experimental-shape result is near 689.484 keV. These
are distinct laws and are not averaged or adjusted to force either mean.

LARA reports no separate emissions. Relative sulfur atomic-line tables do
not supply absolute source yields: no gamma, X-ray, Auger, antineutrino,
recoil, daughter-chain or placeholder group is added.

Retained BetaShape command (not rerun):

```text
.\betashape.exe .\P32\P-32.txt -csv
```

The CSV law is piecewise linear and normalized only for conditional energy.
Physical yields remain separate; the finite compiled grid is compared with
the full selected reference. Pointwise uncertainty columns are retained,
but the current analysis compares central densities without covariance.

Complete evidence remains in [raw/](../raw/). Use [reference.json](reference.json)
with the [generic validation commands](../../../README.md); the JSON contains
only machine-consumed inputs.
