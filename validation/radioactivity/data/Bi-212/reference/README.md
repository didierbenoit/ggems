# Bi-212 selected reference

Selected authority: CEA/LNE-LNHB, A. L. Nichols, Surrey, 2010; February 2011 tables.

Seven beta-minus branches use `dN/dE calc.` with retained forbidden
non-unique approximation warnings. The alpha law combines eight ordinary
direct Bi-212 lines and three beta-delayed long-range lines from excited
Po-212 states populated by Bi-212 beta decay. LNHB explicitly includes
these long-range emissions in the Bi-212 decay. Actual alpha energies and
absolute LARA yields are selected, not transition Q values.

Prompt Tl-208/Po-212 photons and conversion electrons are included.
Later radioactive Po-212 ground-state alpha decay at 8785.17 keV and
Tl-208 decay remain excluded. The groups are independent marginal
emissions per Bi-212 parent decay, without cascade correlations.

ENSDF ground-state data use 60.55 min and a 35.94% alpha branch, compared
with LNHB's 60.54 min and 35.93%. The retained 25 min ENSDF isomer dataset
is not the selected parent. BetaShape's rounded branch intensities and
572.6 keV support need not reproduce the 572.7 keV PenNuc endpoint;
selected CSV tokens and evaluated physical branch yields remain separate.

Retained BetaShape 2.4 command:

```text
.\betashape.exe .\Bi212\Bi-212.txt -csv
```

CSV energy, selected density and adjacent uncertainty tokens are copied
unchanged over full support. Validation conditionally normalizes the
piecewise-linear law; physical yields remain separate.

Complete evidence remains in [raw/](../raw/). Unsupported Auger energy
laws, neutrinos and recoil particles are outside this source model.
See the [generic validation instructions](../../../README.md) for running
this `reference.json`. Numerical reproduction of a selected reference is
not a new evaluation of its nuclear-data uncertainty.
