# I-123 selected reference

Select the V. Chiste / M.-M. Be LNHB/DDEP evaluation, with PenNuc dated
July 16, 2003 and tables printed in 2004. Convert the evaluated half-life
in hours using exactly 3600 s/h. This EC source has four discrete groups:
LARA nuclear photons and compact X rays, detailed MIRD Summary Spectrum
Auger entries, and PenNuc K/L conversion electrons.

The MIRD Auger model includes outer-shell cascade channels, explaining its
larger total than LNHB's compact K/L inventory. Those models are not added
together or claimed to be identical. The retained CSV alone does not
establish the ICRP-107 attribution documented by the implementation.
Auger energies retain the historical nearest-meV mapping, with residuals
up to 200 micro-eV.

Supporting BetaShape 2.4 EC output with `fixint=1` preserves the evaluated
zero positron intensity despite an energetically possible weak transition.
No beta spectrum, EC placeholder, annihilation photons, additional atomic
rows, neutrino, recoil or later daughter-decay emission is selected.

Complete evidence remains in [raw/](../raw/). Use [reference.json](reference.json)
with the [generic validation commands](../../../README.md); the JSON contains
only machine-consumed inputs.
