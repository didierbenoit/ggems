# Mo-99 selected reference

Selected authority: CEA/LNE-LNHB, C. Morillon, M.-M. Be, V. P. Chechev and A. G. Egorov, LNHB/KRI, 2004; 2012 tables.

Eleven beta-minus branches remain separate. Select `dN/dE exp.` for
1214.5 keV (`1 - 0.01*W`, measured 450-1150 keV) and 848.1 keV
(`1 + 0.049*W - 0.0047/W + 0.034*W^2`, measured 470-780 keV).
Other branches use `dN/dE calc.`; approximation limitations remain.
The branch yields are absolute LNHB values, not normalized categorical
probabilities. ENSDF gives 65.924 h versus LNHB's 2.7479 d; MIRD's branch
inventory and means differ and do not replace the retained shapes.

Select prompt higher-level Tc-99 gamma/conversion transitions. Exclude
the 142.6832 keV Tc-99m isomer's delayed transitions (about 6.0067 h).
The 140.511 keV photon/conversion totals and compact atomic totals combine
prompt feeding with isomer-fed relaxation; their undivided laws are
excluded. This does not claim that all 140.511 keV photons are delayed.
Later Tc-99 radioactive decay is also excluded.

Retained BetaShape 2.4 command:

```text
.\betashape.exe .\Mo99\Mo-99.txt -csv
```

CSV energy, selected density and adjacent uncertainty tokens are copied
unchanged over full support. Validation conditionally normalizes the
piecewise-linear law; physical yields remain separate.

Complete evidence remains in [raw/](../raw/). Unsupported Auger energy
laws, neutrinos and recoil particles are outside this source model.
See the [generic validation instructions](../../../README.md) for running
this `reference.json`. Numerical reproduction of a selected reference is
not a new evaluation of its nuclear-data uncertainty.
