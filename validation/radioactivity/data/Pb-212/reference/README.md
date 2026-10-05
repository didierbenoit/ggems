# Pb-212 selected reference

Selected authority: CEA/LNE-LNHB, D. E. Zimmerman, NIST, May 2025.

The three beta-minus branches use `dN/dE calc.`. Retained forbidden
non-unique Xi approximations are not experimental validation. LNHB branch
yields are retained without forcing their rounded sum to one; BetaShape
provides only conditional energy laws. LARA supplies prompt photons and
PenNuc supplies conversion lines. The 115.183 keV photon uses LARA's
0.586% rather than PenNuc's 0.5862% rounding.

Only emissions belonging to the Pb-212 parent section are selected. The
additional Bi-212/Po-212/Tl-208 chain inventory in LARA is excluded.
ENSDF retains 10.622 h rather than the selected 10.630 h; MIRD branch
yields also differ. These cross-checks are not averaged into LNHB.

Retained BetaShape 2.4 command:

```text
.\betashape.exe .\Pb212\Pb-212.txt -csv
```

CSV energy, selected density and adjacent uncertainty tokens are copied
unchanged over full support. Validation conditionally normalizes the
piecewise-linear law; physical yields remain separate.

Complete evidence remains in [raw/](../raw/). Unsupported Auger energy
laws, neutrinos and recoil particles are outside this source model.
See the [generic validation instructions](../../../README.md) for running
this `reference.json`. Numerical reproduction of a selected reference is
not a new evaluation of its nuclear-data uncertainty.
