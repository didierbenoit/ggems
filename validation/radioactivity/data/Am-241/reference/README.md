# Am-241 selected reference

Select the V. P. Chechev / N. K. Kuzmenko LNHB/KRI evaluation updated
September 2009. Use LARA's published seconds directly. LARA supplies alpha,
nuclear gamma and compact Np X-ray lines; PenNuc supplies positive conversion
lines. Only the 15 detailed Auger entries use the retained MIRD Summary
Spectrum CSV, with historical nearest-meV energy mapping. BetaShape does
not apply to this alpha-decay reference.

The existing conversion partition at absolute yield 1e-9 preserves all
positive lines in main/weak groups. Four zero central records are excluded.
No physical yield is renormalized.

The 5469.47 keV alpha line retains LARA's numerical 0.04%, although the
companion evaluation gives an upper limit of <0.04%. This unresolved
qualification remains a limitation: numerical agreement does not establish
agreement with the evaluated limit. Immediate Np de-excitation is included;
recoil, later Np-237 decay, spontaneous fission and MIRD non-Auger rows are
excluded.

Complete evidence remains in [raw/](../raw/). Use [reference.json](reference.json)
with the [generic validation commands](../../../README.md); the JSON contains
only machine-consumed inputs.
