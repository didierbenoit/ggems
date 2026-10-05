# Sr-82 selected reference

Selected authority: CEA/LNE-LNHB, M.-M. Be, 2014; January 2015 tables.

Electron capture feeds radioactive Rb-82 ground state. Five absolute
compact Rb X-ray lines from LNHB/LARA form the selected photon law.
BetaShape 2.4 supplies supporting EC information, not a spectrum or an
EC placeholder primary.

Later Rb-82 positrons, photons and transport annihilation products are
not Sr-82 source emissions. Unsupported Auger energy laws are excluded.
ENSDF uses 25.35 d instead of LNHB's 25.347 d; MIRD's more extensive atomic
model is only an independent cross-check.

Retained BetaShape 2.4 command:

```text
.\betashape.exe .\Sr82\Sr-82.txt -csv
```

Complete evidence remains in [raw/](../raw/). Unsupported Auger energy
laws, neutrinos and recoil particles are outside this source model.
See the [generic validation instructions](../../../README.md) for running
this `reference.json`. Numerical reproduction of a selected reference is
not a new evaluation of its nuclear-data uncertainty.
