# Cs-131 selected reference

Selected authority: CEA/LNE-LNHB, A. L. Nichols, September 2021.

Electron capture feeds stable Xe-131. Select only the absolute individual
K X-ray lines in LNHB/LARA. The K total is not an additional emission.
Range-only L/M/N totals and Auger tables do not supply a selected conditional
energy law and are excluded; no representative monoenergy is invented.

The retained BetaShape 2.4 EC output supplies supporting capture information,
not a continuous spectrum or an EC primary. ENSDF gives 9.689 d versus
selected LNHB's 9.681 d. MIRD includes a different, more extensive atomic
line model and is not used to fill the excluded energy laws.

Retained BetaShape 2.4 command:

```text
.\betashape.exe .\Cs131\Cs-131.txt -csv
```

Complete evidence remains in [raw/](../raw/). Unsupported Auger energy
laws, neutrinos and recoil particles are outside this source model.
See the [generic validation instructions](../../../README.md) for running
this `reference.json`. Numerical reproduction of a selected reference is
not a new evaluation of its nuclear-data uncertainty.
