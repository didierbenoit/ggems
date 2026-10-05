# Bi-213 selected reference

Selected authority: CEA/LNE-LNHB, H. Xiaolong and W. Baosong, CNDC, 2011; 2013 tables and 2021 atomic update.

Ten beta-minus branches use `dN/dE calc.`. Two direct alpha lines and
prompt Po-213/Tl-209 photons are selected. The Po-213 ground-state alpha
decay after about 3.70 microseconds and later Tl-209 decay are excluded.

For the 147.7 and 292.8 keV transitions, the published LNHB table M and
N+O coefficients take precedence over discrepant exponents in the retained
ENSDF-format input and derived PenNuc intensities. Absolute yields are
`I_gamma * alpha_M` and `I_gamma * (alpha_N + alpha_O)`, using the published
per-parent gamma intensities. PenNuc compact shell energies are retained.
The selected M yields are 0.000028928 and 0.00006315, and N+O yields are
0.000008832 and 0.0000192397. No raw file is corrected.

LARA absolute atomic emissions reflect the retained 2021 correction.
The photon energy 886.66 keV follows the emission table/LARA, not the
inconsistent 887.76 keV transition-table entry. ENSDF's newer alpha
evaluation gives 2.140%, while selected LNHB gives 2.09%; no averaging
or forced renormalization is performed.

Retained BetaShape 2.4 command:

```text
.\betashape.exe .\Bi213\Bi-213.txt -csv
```

CSV energy, selected density and adjacent uncertainty tokens are copied
unchanged over full support. Validation conditionally normalizes the
piecewise-linear law; physical yields remain separate.

Complete evidence remains in [raw/](../raw/). Unsupported Auger energy
laws, neutrinos and recoil particles are outside this source model.
See the [generic validation instructions](../../../README.md) for running
this `reference.json`. Numerical reproduction of a selected reference is
not a new evaluation of its nuclear-data uncertainty.
