# Tb-161 selected reference

Use retained ENSDF (C. W. Reich, June 2011, NDS 112,2497 (2011)) for
ground-state Tb-161 identity, 6.89(2) d half-life and 100% beta-minus decay.
Use exactly 86400 s/d. MIRD supplies the explicit line-emission laws and
aggregate beta energy law; its CSV does not establish an evaluation identity.

Select one Electron `RegularSpectrum` with physical beta yield exactly 1.
The nine MIRD Beta summary rows are branch means, not monoenergies; their
rounded yields sum to 1.0000008979 and do not replace the exact ENSDF branch.
Keep explicit gamma, X-ray, Auger and conversion rows. ENSDF identifies the
29.5 ns, 3.14 ns and 0.145 ns Dy levels as directly populated relaxation,
not subsequent radioactive daughters. Weak conversion lines are split only
for finite-ticket reachability.

Convert every aggregate point exactly from MeV to keV (times 1000) and
density per MeV to per keV (divided by 1000). Preserve full support and
stored density, independently of yield. Leave pointwise uncertainty empty:
none is supplied. Validation concerns the selected aggregate law, not
individual beta-branch shapes. No source annihilation photon is included.

Neutrinos, recoil and additional radioactive daughter decays are excluded.
Complete retained evidence remains in [raw/](../raw/). Use
[reference.json](reference.json) with the
[generic validation instructions](../../../README.md).
