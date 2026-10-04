# I-125 selected reference

Select the V. Chiste / E. Schonfeld / M.-M. Be LNHB/DDEP evaluation
completed in July 2010. Convert the evaluated days using exactly 86400 s/d.
The EC model includes the LNHB nuclear gamma, LARA compact X rays, 13
MIRD Summary Spectrum Auger entries and six PenNuc conversion entries.
The 1.48 ns excited-level de-excitation is flattened at parent decay time.

MIRD's outer-shell Auger cascades explain the large electron multiplicity;
LNHB compact Auger totals are not added again. Auger energies retain the
historical nearest-meV mapping, with residuals up to 400 micro-eV.
The explicit PenNuc EN yield is selected despite its difference from the
compact N-shell table. Outer-shell aggregation is a supported inference,
not an explicitly documented algorithm; no separate EO line is invented.

BetaShape 2.4 supplies supporting EC information only. No continuous beta
spectrum, positron, EC placeholder, source annihilation photon, neutrino
or recoil is selected. Te-125 ground state is stable. MIRD photon and
conversion rows are not imported.

Complete evidence remains in [raw/](../raw/). Use [reference.json](reference.json)
with the [generic validation commands](../../../README.md); the JSON contains
only machine-consumed inputs.
