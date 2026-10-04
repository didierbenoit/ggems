# Tl-201 selected reference

Select the LNHB evaluation by E. Schonfeld and R. Dersch (1997), with
the May 2004 half-life update. Convert days with exactly 86400 s/d. This is
an EC-only source: selected groups contain absolute LARA photon yields and
PenNuc conversion lines grouped by transition, not capture placeholders.

The LNHB table gives the 141.18 keV photon intensity as an upper limit,
`<0.008%`; LARA and PenNuc lose that qualifier. Exclude this line and its
derived conversion lines rather than interpreting the bound as a measured
central yield. Keep other weak positive photon lines, including 1.565 keV.
The strong low-energy conversion yield associated with that transition lacks
a complete selected conditional law in these exports and is not invented.

The retained BetaShape 2.4 EC calculation is supporting evidence only;
it supplies no continuous source spectrum. No experimental EC particle or
arbitrary atomic distribution is introduced.

Unsupported Auger energy laws, neutrinos, recoil particles and additional
daughter decays are excluded. EC probabilities are not emitted particles.
Selected groups are independent marginals, without cascade correlations.
ENSDF (when retained) and MIRD are cross-checks only and are not averaged
with the selected evaluation.

Complete evidence remains in [raw/](../raw/). Use [reference.json](reference.json)
with the [generic validation commands](../../../README.md). The JSON contains
only machine-consumed inputs.
