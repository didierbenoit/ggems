# Tc-99m independent reference

The reference was recovered from the local `../raw/` evidence before comparing
the production definition. Physical intensities are particles per parent decay,
not mutually exclusive probabilities. They are never normalized to one.

## Selected authority and prompt boundary

The selected CEA/LNE-LNHB evaluation is by C. Morillon, M. M. Be, V. Chechev and
A. Egorov. `lnhb/Tc-99m_com.pdf`, p. 1, identifies completion in December 2000
and the half-life update in January 2004; the PenNuc record is dated 19/01/2004.
The later PDF printing footer does not establish a new nuclear selection.

`lnhb/Tc-99m.lara.txt` and `Tc-99m_tables.pdf`, p. 1, give
**6.0067(10) h = 21624.12(3.60) s**. Convert the hours exactly; the LARA seconds
entry, 21624.1 s, is a rounded display. The commentary, p. 2, associates this
adopted half-life with pertechnetate solution. The selected Q values are
436.2(2) keV for beta decay from the isomer and 142.683(1) keV for IT.

Tc-99m decays predominantly by isomeric transition to Tc-99. The rare direct
beta-minus transitions to Ru-99, and their prompt Ru-99 gamma and conversion
electrons, belong to the same parent definition. Later radioactive decay of
Tc-99, whose selected half-life is 211.5(1.1) thousand years, is excluded.
No daughter-chain contribution is added.

## Composite beta-minus spectrum

The three `BEM` records in `lnhb/Tc-99m.PenNuc.txt`, also tabulated in
`Tc-99m_tables.pdf`, p. 1, supply the physical branches:

| Branch / retained file | Selected endpoint (keV) | Yield per parent | Yield uncertainty | Raw BetaShape endpoint (keV) | Mapped reference mean (keV) |
|---|---:|---:|---:|---:|---:|
| `beta-_Tc99m_trans0.bs` | 436.3(2) | 0.0000100 | 0.0000030 | 436.20 | 151.241445451153 |
| `beta-_Tc99m_trans1.bs` | 346.7(2) | 0.0000260 | 0.0000050 | 346.52 | 101.201072751108 |
| `beta-_Tc99m_trans2.bs` | 113.9(2) | 0.00000106 | 0.00000006 | 113.82 | 29.705337507320 |

All three shapes are retained **BetaShape 2.4 (06/2024) calculated** results in
`betashape/v2.4/output/`. No experimental shape is substituted. The exact
recorded command in `betashape/v2.4/command.txt` is:

```text
.\betashape.exe Tc99m\Tc-99m -csv
```

There is no `-qval` or `fixint`, and no calculation was rerun. The `.trans` file
explicitly distinguishes energies calculated from Q and level energies from
the given measured `Emax`. The retained input has ground-state Q = 293.52 keV
and parent excitation 142.683 keV; subtracting daughter levels 0, 89.68 and
322.38 keV explains the displayed calculated endpoints. The selected LNHB
branch endpoints remain authoritative. Thus the retained mappings are
436.20 -> 436.3, 346.52 -> 346.7 and 113.82 -> 113.9 keV.

For branch j, let h_j(E) be the piecewise linear interpolation of the retained
calculated density, H_j its exact trapezoidal area, and s_j the endpoint ratio.
The mapped conditional density is f_j(E) = h_j(E/s_j)/(s_j H_j). The Jacobian
preserves unit conditional area. The composite density is

```text
f(E) = sum_j[y_j f_j(E)] / Y,    Y = sum_j y_j = 0.00003706.
```

The CSV uses the union of the three mapped knot sets, including all endpoints:
1,165 knots. It therefore represents the sum of the piecewise linear laws,
without an additional resampling approximation. Decimal integration gives
unit area and mean **112.6586617322743 keV**. Only conditional shapes are
normalized; the original three yields and the global yield remain separate.
The uncertainty CSV column is empty because the recovered evidence supplies
no covariance model for the normalized mixture. Raw pointwise uncertainties
remain available in the retained branch files; no uncertainty was invented.

BetaShape qualifies the 436.2 keV transition as first-forbidden unique (`1U`).
For both other transitions it states:

> This 1st forbidden non-unique transition from the beta - decay of Tc-99 is calculated as allowed.

The 346.52 keV file adds exactly:

> This non-unique transition is calculated according to the Xi-approximation: me*alpha*Z/R = 11412.6 keV  vs  346.52 keV  -->  unpredictable: check with measurement if possible.

The 113.82 keV file uses the same Xi approximation and concludes
`-->  should be correct.` These are retained model qualifications, not GGEMS
discrepancies. The `.trans` file additionally warns that information about
each decay is not totally sure. No shape correction is made here.

## Discrete emissions and group mapping

| Group | Particle / law | Selected content | Physical group yield |
|---:|---|---|---:|
| 0 | Electron / RegularSpectrum | Three-branch beta mixture | 0.00003706 |
| 1 | Gamma / DiscreteLines | Five prompt nuclear gamma lines | 0.885241444 |
| 2 | Gamma / Mono | Ultra-weak 2.1726 keV gamma | 0.000000000074 |
| 3 | Gamma / DiscreteLines | Five compact Tc X-ray entries | 0.08209 |
| 4 | Electron / DiscreteLines | 22 detailed Auger lines | 4.4144146979128 |
| 5 | Electron / DiscreteLines | 18 conversion-electron lines | 1.1131270181 |

Total flattened yield: **6.4949102200868 particles per parent decay**.
All 51 discrete physical lines are retained, including lines too weak to sample
in the standard campaign. JSON line weights retain their absolute physical
yields; the energy distribution uses their conditional ratios.

- **Nuclear gamma:** the six `g` rows in `lnhb/Tc-99m.lara.txt`, independently
  agreeing with all PenNuc `GA` records and the PDF photon table, p. 4. The
  ordinary group contains 89.6, 140.511, 142.683, 232.7 and 322.4 keV. The three
  Ru-99 lines are prompt de-excitations following the direct parent beta decay.
- **Ultra-weak gamma:** LARA gives 2.1726(4) keV and 7.4(2)e-9 photons per 100
  decays, hence 7.4(2)e-11 per parent. With all six gamma lines in one 2^32
  ticket distribution, its quota is 0.3590292592 and the existing largest
  remainder allocation gives it zero tickets. Its separate Mono group is a
  numerical representation split that preserves the evaluated yield.
- **X-rays:** the five non-`g` LARA entries, with the published effective XL,
  K-alpha-2, K-alpha-1, grouped K-beta-1 and grouped K-beta-2 energies. No
  microscopic sub-lines are inferred from the grouped data.
- **Auger:** all 22 `Auger electron` rows in
  `mird/Tc-99m Summary Spectrum.csv`. This is the established MIRDspecs/ICRP-107
  detailed atomic representation. LNHB supplies grouped Auger ranges, not
  these detailed lines. The retained CSV itself does not embed an ICRP edition
  citation; that attribution is documented by the current implementation.
  The older `mird-99Tc-table-1.csv` is not the selected detailed dataset.
- **Conversion:** all 18 `EK`, `EL`, `EL1`, `EL2`, `EL3`, `EM` and `EN` records
  from `lnhb/Tc-99m.PenNuc.txt`, sorted by energy with source row and emitting
  nucleus preserved. Four are prompt Ru-99 contributions; 14 are Tc IT
  contributions. LNHB commentary p. 7 describes calculation from transition
  energies, binding energies, conversion coefficients and gamma probabilities.

## Representation and source disagreements

The compiled beta grid is [0.00079, 436.3) keV, 873 bins of width 0.49977 keV.
Integrating this reference on that grid and conditioning on its support
reproduces every compiled bin weight within 2.65e-19 absolute and every ticket
count exactly. The reference retains its full support from zero; the omitted
low-energy conditional mass is 5.4159256430e-6. The finite-ticket mean is
112.659414413468 keV, 0.000752681193 keV above the unbinned reference. The full
grid/reference CDF supremum is 1.1733357621e-5. These differences describe the
existing finite-grid representation.

The historical meV quantization, retained in current integer micro-eV storage,
affects two MIRD energies: 0.0296081 -> 0.029608 keV (-100 micro-eV) and
0.0314686 -> 0.031469 keV (+400 micro-eV). JSON preserves `source_energy_MeV`,
`source_energy_keV` and each residual explicitly; `energy_keV` gives the
represented value used by the existing discrete validator. All other selected
line energies are exact in the compiled representation. Physical line yields
are unchanged, apart from ordinary binary floating representation.

ENSDF's retained July 2017 evaluation is an independent cross-check. It gives
6.0072(9) h (1.8 s above the selected half-life), and calculated beta endpoints
440.2(10), 350.6(10), 117.8(10) keV instead of the selected LNHB values.
These evaluations are not averaged or substituted. MIRD also has different
nuclear values, for example 0.890567 at 140.511 keV versus selected LNHB 0.885;
MIRD is used only for detailed Auger lines. Within LNHB, the transition table
contains the historical 142.675(25) keV entry, whereas the photon table, LARA
and PenNuc select 142.683(1) keV; commentary p. 7 explains the latter as the
sum of the two adopted transition energies. The explicit selected photon and
PenNuc electron records are retained without reconciliation.

The standard campaign keeps physical yields and sample target unchanged.
Empty samples and non-discriminating DKW limits >= 1 are
`insufficient_samples`; no rare component is amplified to force a PASS.
