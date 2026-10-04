# Tc-99m selected reference

Select the C. Morillon / M.-M. Be / V. Chechev / A. Egorov LNHB evaluation
completed in 2000, with the January 2004 half-life update. Convert evaluated
hours using exactly 3600 s/h. Include isomeric de-excitation and rare direct
beta-minus decay to Ru-99, with its prompt products. Later Tc-99 decay is
excluded. Photons and compact X rays use LARA; conversion electrons use
PenNuc; only detailed Auger entries use the retained MIRD Summary Spectrum
CSV and historical nearest-meV energy mapping.

The composite CSV uses three BetaShape 2.4 (06/2024) `dN/dE calc.` laws.
Retain the established endpoint mappings 436.20 -> 436.3, 346.52 -> 346.7
and 113.82 -> 113.9 keV. Each mapped branch is conditionally normalized,
weighted by its evaluated yield, and summed on the union of knots; division
by the total beta yield defines the conditional composite. The uncertainty
column is empty because no covariance model supports mixture uncertainties.
The 346.52 keV Xi-approximation law carries an unpredictable-shape warning.

The ultra-weak 2.1726 keV gamma remains a separate Mono group so its physical
yield is not lost to finite-ticket rounding in the ordinary gamma group.

Retained BetaShape command (not rerun):

```text
.\betashape.exe Tc99m\Tc-99m -csv
```

The CSV law is piecewise linear and normalized only for conditional energy.
Physical yields remain separate; the finite compiled grid is compared with
the full selected reference. Pointwise uncertainty columns are retained,
but the current analysis compares central densities without covariance.

Complete evidence remains in [raw/](../raw/). Use [reference.json](reference.json)
with the [generic validation commands](../../../README.md); the JSON contains
only machine-consumed inputs.
