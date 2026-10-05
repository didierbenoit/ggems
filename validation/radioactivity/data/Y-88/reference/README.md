# Y-88 selected reference

Select the retained KRI/LNHB evaluation (Chechev and Kuzmenko, 2015).
The parent is ground-state Y-88; the separate millisecond IT datasets are
not this parent. Directly populated Sr-88 levels relax on ps/sub-ns scales.
Retain the evaluated compact photon and shell conversion laws. The weak
484.352 keV photon uses the fixed LARA/table value; the commentary describes
an upper-limit measurement, so its nuclear uncertainty remains substantial.

Select the full BetaShape 2.4 experimental positron law, with factor
`q^2 + l_2*p^2` measured over 200-650 keV. `fixint=1` preserves the
evaluated EC/beta-plus split. The 764.5 keV tabulated support is retained;
PenNuc's 764.51 keV arithmetic endpoint is not used to rescale it.
EC creates no primary. Exclude source 511 keV annihilation photons and
internal-pair products without a selected conditional energy law.
The finite spectrum uses bins up to 4 keV for positive-ticket reachability.

Retained BetaShape command (not rerun):

```text
.\betashape.exe .\Y88\Y-88.txt fixint=1 -csv
```

CSV energy, selected density and adjacent uncertainty tokens preserve the
full tabulated support. Only the conditional law is normalized for sampling.

ENSDF and MIRD are independent cross-checks only. Unsupported grouped Auger
energy laws are excluded; compact LARA X-ray energies retain its convention.

Neutrinos, recoil and additional radioactive daughter decays are excluded.
Complete retained evidence remains in [raw/](../raw/). Use
[reference.json](reference.json) with the
[generic validation instructions](../../../README.md).
