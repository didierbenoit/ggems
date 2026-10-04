# Lu-177 selected reference

Select M. A. Kellett and X. Mougeot's CEA-LNHB evaluation, October 2025,
with tables dated November 26, 2025. Convert the evaluated days using exactly
86400 s/d. Nuclear beta/gamma data use the evaluation, compact X rays use
LARA, and conversion electrons use PenNuc. Only the 15 detailed Auger entries
use the retained MIRD Summary Spectrum CSV. Their energies retain the
historical nearest-meV mapping, including a 400 micro-eV rounding residual.

All four branch spectra select BetaShape 2.4 (06/2024) `dN/dE calc.`.
The 383.8 and 496.8 keV first-forbidden non-unique transitions use the
allowed Xi approximation; BetaShape warns that their shapes are unpredictable
and should be checked against measurement. Agreement with these selected
laws does not establish their experimental accuracy.

MIRD non-Auger emissions do not replace or supplement the selected LNHB
groups. Physical branch yields are not forced to sum to one.

Retained BetaShape command (not rerun):

```text
.\betashape.exe Lu177\Lu-177.txt -csv
```

The CSV law is piecewise linear and normalized only for conditional energy.
Physical yields remain separate; the finite compiled grid is compared with
the full selected reference. Pointwise uncertainty columns are retained,
but the current analysis compares central densities without covariance.

Complete evidence remains in [raw/](../raw/). Use [reference.json](reference.json)
with the [generic validation commands](../../../README.md); the JSON contains
only machine-consumed inputs.
