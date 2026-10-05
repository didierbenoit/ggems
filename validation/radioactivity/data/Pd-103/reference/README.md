# Pd-103 selected reference

Selected authority: CEA/LNE-LNHB, A. L. Nichols and T. Kibedi, 2024.

Electron capture creates no primary. The selected law contains only
supported prompt photons and their conversion lines from higher Rh-103
levels. Rh-103m at 39.753 keV lives about 56.115 min: its transition,
conversion electrons and associated relaxation are excluded.

The retained compact Rh atomic totals mix prompt capture relaxation with
delayed isomer conversion. They are excluded because the retained absolute
line law does not separate those contributions. This deliberately limited
source is not a complete Pd-103 photon-dose spectrum.

The retained BetaShape 2.4 EC calculation supports the level interpretation;
it supplies no continuous spectrum or arbitrary EC particle. ENSDF uses
16.991 d rather than selected LNHB's 17.00 d. MIRD is an independent
comparison with different atomic/source boundaries, not a replacement law.

Retained BetaShape 2.4 command:

```text
.\betashape.exe .\Pd103\Pd-103.txt -csv
```

Complete evidence remains in [raw/](../raw/). Unsupported Auger energy
laws, neutrinos and recoil particles are outside this source model.
See the [generic validation instructions](../../../README.md) for running
this `reference.json`. Numerical reproduction of a selected reference is
not a new evaluation of its nuclear-data uncertainty.
