# Selected O-15 reference

`reference.json` selects the recovered LNHB evaluation by X. Mougeot, with
literature through February 2026 and tables dated March 3, 2026. Decimal strings
retain source precision and absolute standard uncertainties. Raw evidence paths
are relative to `../raw/`. Source identities, evaluation dates, calculation
options and transformations are recorded in `reference.json`.

The selected half-life is **122.266(49) s**, Q+ **2754.18(49) keV**, beta+
probability **99.9001(17)%**, EC probability **0.0999(17)%**, beta+ endpoint
**1732.18(49) keV**, and beta+ mean energy **733.47(23) keV**. These come from
`O-15_tables.pdf`, `O-15_com.pdf`, and the recovered LNHB input. The BetaShape input is the recovered LNHB `O-15.txt` evaluation.

The recovered calculation used **BetaShape 2.4 (06/2024)**:

```powershell
.\betashape.exe O15\O-15.txt fixint=1 -csv
```

There is no Q-value override. `fixint=1` retains the adopted EC/beta+ split.
The transition header reports allowed shape, screening, radiative and atomic
overlap corrections, and no experimental shape factor. The executable was not
rerun. The original execution timestamp is unavailable.

`positron_spectrum.csv` copies all 348 energy, calculated-density and uncertainty
triples from `beta+_O15_trans0.bs` without numerical changes. The total-spectrum
file has identical triples. The grid starts at 0 keV, steps by 5 keV through
1730 keV, and ends at 1732.18 keV. Its trapezoidal integral is
0.998991212775914. For **conditional energy shape only**, analysis integrates the
piecewise linear interpolant and divides by that integral. Its conditional mean
is approximately 733.475207446 keV. The finite output grid accounts for the
difference from the separately reported 733.47 keV calculated mean; no adjustment
forces agreement. Pointwise uncertainties are preserved, but no covariance is
provided, so shape tests condition on the central table without treating those
uncertainties as independent measurements.

The mapping is one ordered `Positron` / `RegularSpectrum` group with physical
yield **0.999001 primary per parent decay**. Conditional spectrum normalization
does not normalize or otherwise alter emission yields. The selected evidence has
no prescribed GGEMS regular grid: validation compares the entire compiled table
and its induced distribution with the selected continuous reference.

The **511 keV / 199.8002(34)%** annihilation radiation is retained as reference
information and explicitly excluded from Source groups. Positron transport owns
annihilation. EC itself is not a transported particle; the selected package does
not invent an EC placeholder or import MIRD atomic-relaxation groups. Neutrino,
daughter recoil and subsequent physical transport are outside this pilot.

The other recovered files remain independent evidence:

| File | Difference from the selection |
| --- | --- |
| LNHB LARA text | 122.268(48) s, versus 122.266(49) s selected. This is consistent with conversion of rounded 2.0378(8) min; the explanation is an inference. |
| ENSDF PDF | 1991 evaluation, cutoff July 1990: 122.24(16) s, Q+ 2754.0(5) keV, beta+ 99.9003(10)%, EC 0.0997(10)%, mean 735.28(23) keV. |
| MIRD CSVs | Mean beta+ energy 0.7352 MeV and additional annihilation/X-ray/Auger entries; evaluation identity is not established by the recovered CSV. |

No conflicting evaluation is merged. Current built-in provenance is audited
separately: its source comment identifies BetaShape 2.2 with an energy axis
rescaled from 1735.0 to 1732.18 keV. This package does not correct that built-in.
