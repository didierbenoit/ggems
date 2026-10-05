# Y-90 selected reference

Select the retained LNHB evaluation (Chiste and Be, 2015) for ground-state
Y-90. Use the commentary's explicitly adopted 64.041(31) h result with
exactly 3600 s/h; 2.6684(13) d and the LARA seconds field are rounded
representations. The separate 3.19 h isomer is not included.

Keep three beta-minus marginals. Select BetaShape 2.4 `dN/dE exp.` for
2278.7 keV, with `(1 - 0.0114*W) * (q^2 + l_2*p^2)` measured over
60-2230 keV; use `dN/dE calc.` for 518.0 and 92.4 keV. The last branch
uses the retained Xi approximation, not an experimental shape. The retained
`fixint=1` flag has no EC/beta-plus split to preserve here.

Retain the prompt 2186.254 keV photon. LNHB gives the E0 conversion yield
as an undivided 1742.70-1760.67 keV range and internal-pair summary energies;
neither supplies a complete selected conditional law, so these products are
excluded. The LARA 511 keV annihilation line is also excluded. Rounded beta
branch yields are preserved individually, not forced to sum to one.

Retained BetaShape command (not rerun):

```text
.\betashape.exe .\Y90\Y-90.txt fixint=1 -csv
```

CSV energy, selected density and adjacent uncertainty tokens preserve the
full tabulated support. Only the conditional law is normalized for sampling.

ENSDF and MIRD remain independent cross-checks; their values do not replace
the selected LNHB/BetaShape laws.

Neutrinos, recoil and additional radioactive daughter decays are excluded.
Complete retained evidence remains in [raw/](../raw/). Use
[reference.json](reference.json) with the
[generic validation instructions](../../../README.md).
