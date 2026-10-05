# Rb-81 selected reference

Use ENSDF (Basunia, evaluated September 2024, NDS 199,271 (2025)) for
ground-state Rb-81 identity and timing; convert 4.571(4) h with 3600 s/h.
Use retained MIRD explicit emission rows and the aggregate positron spectrum
for the selected energy laws. The separate 30.5 min Rb-81m parent is excluded.

The nine MIRD Positron summary rows give physical yield 0.2723788548; their
energies are means, not monoenergetic emissions. Select one aggregate
`RegularSpectrum`. Its area is not the physical yield. ENSDF branch
intensities differ and are a structural cross-check, not averaged with MIRD.
The MIRD annihilation yield 0.544758 is approximately twice the positron
yield; source 511 keV photons are excluded.

Exclude the later 13.10 s Kr-81m IT and ground-state Kr-81 decay. The MIRD
summary contains neither the 190.44/190.46 keV IT photon nor its conversion
family. Conversion energies map to the retained prompt gamma inventory;
the atomic K-vacancy balance agrees with prompt EC plus those conversions,
without the large delayed IT contribution. Retain that prompt atomic law,
including explicit Auger lines. Cascades feeding the isomer are allowed;
its later transition is not flattened to parent time.

Convert every aggregate point exactly: MeV times 1000, density per MeV
divided by 1000. Preserve full support and stored absolute density; leave
the uncertainty column empty because MIRD supplies none. The finite grid
uses bins up to 2 keV for positive-ticket reachability. This validates the
selected aggregate energy law, not individual positron-branch shapes.

Neutrinos, recoil and additional radioactive daughter decays are excluded.
Complete retained evidence remains in [raw/](../raw/). Use
[reference.json](reference.json) with the
[generic validation instructions](../../../README.md).
