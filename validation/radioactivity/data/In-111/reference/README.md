# In-111 selected reference

Select the LNHB evaluation by V. P. Chechev (March 2006; April 2006
tables), converting days with exactly 86400 s/d. This EC-only source uses
absolute LARA photon yields and PenNuc conversion groups.

The selected per-parent inventory explicitly includes the weak 150.81 keV
isomer-fed photon and its supported conversion lines. These are retained as
independent marginal yields; the 48.50 min Cd-111m level delay is not modeled.
This is not a daughter-population or correlated cascade simulation. No
additional daughter-decay database is used.

The retained BetaShape 2.4 EC calculation is supporting evidence only;
it provides no continuous source law and creates no capture primaries.
Auger totals with grouped energy ranges do not define a complete selected
conditional distribution, so no Auger source group is invented.

Unsupported Auger energy laws, neutrinos, recoil particles and additional
daughter decays are excluded. EC probabilities are not emitted particles.
Selected groups are independent marginals, without cascade correlations.
ENSDF (when retained) and MIRD are cross-checks only and are not averaged
with the selected evaluation.

Complete evidence remains in [raw/](../raw/). Use [reference.json](reference.json)
with the [generic validation commands](../../../README.md). The JSON contains
only machine-consumed inputs.
