# At-211 selected reference

Select the A. L. Nichols LNHB evaluation (2010; 2011 tables), converting
7.216 h with exactly 3600 s/h. Keep four direct alpha lines. The evaluated
4895.4 keV line is an upper limit (<0.00004%); LARA/PenNuc flatten that
qualifier, so it is excluded rather than used as a positive central yield.
The approximate 4993.4 keV line remains, together with the three other
evaluated lines. Rounded line intensities are not forced to the nominal
41.78% alpha branch. Prompt Po/Bi nuclear photons, compact X rays and
conversion groups use absolute retained yields.

EC probabilities create no primaries and there is no beta-plus law.
The subsequent 0.516 s Po-211 alpha decay and the later Bi-207 radioactive
decay are excluded. The auxiliary At-211 IT file does not redefine this
ground-state parent. BetaShape supplies capture information only, with no
continuous alpha spectrum; `fixint=1` is not needed.

Unsupported Auger energy laws, neutrinos, recoil and additional radioactive
daughter decays are excluded. Selected groups are independent marginals;
ENSDF and MIRD are cross-checks only and are not averaged with LNHB.

Complete evidence remains in [raw/](../raw/). Use
[reference.json](reference.json) with the
[generic validation commands](../../../README.md).
