# Ir-192 selected reference

Selected authority: CEA/LNE-LNHB, E. Browne, LBNL, 2003.

Six beta-minus branches use `dN/dE calc.`; retained EC transitions
create no primary. Prompt Pt-192/Os-192 gamma, compact X-ray and conversion
laws use absolute LNHB/LARA/PenNuc emissions. The directly populated levels
are prompt or have picosecond/nanosecond lifetimes in the selected model.

Preserve the tabulated beta yields (sum 0.949998), even though the nominal
LNHB beta branch is 0.9513. Neither the slightly different BetaShape input
normalization nor MIRD's 0.9512999822 total replaces these absolute values.
ENSDF gives 73.829 d and 95.24% beta decay; selected LNHB uses 73.827 d.
The retained 1.45 min Ir isomer datasets are not the selected parent.
Forbidden-transition approximations remain nuclear-model limitations.

Retained BetaShape 2.4 command:

```text
.\betashape.exe .\Ir192\Ir-192.txt -csv
```

CSV energy, selected density and adjacent uncertainty tokens are copied
unchanged over full support. Validation conditionally normalizes the
piecewise-linear law; physical yields remain separate.

Complete evidence remains in [raw/](../raw/). Unsupported Auger energy
laws, neutrinos and recoil particles are outside this source model.
See the [generic validation instructions](../../../README.md) for running
this `reference.json`. Numerical reproduction of a selected reference is
not a new evaluation of its nuclear-data uncertainty.
