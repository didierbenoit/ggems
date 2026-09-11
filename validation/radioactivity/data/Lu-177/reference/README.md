# Lu-177 selected reference

This package independently reconstructs the eight current GGEMS Lu-177 emission
groups from the recovered evidence in [../raw](../raw/). It uses the existing
reference schema. Scientific values were read from the raw files, not copied
from the built-in or GoogleTests. Group order and the documented historical
energy representation are the implementation mapping being compared.

## Authority and half-life

The primary authority is the **M.A. Kellett / X. Mougeot, CEA-LNHB Lu-177
evaluation dated 26/11/2025** (`lnhb/Lu-177_tables.pdf`, printed footer).
The comments describe an October 2025 evaluation with literature through
mid-October 2025. The associated `Lu-177.PenNuc.txt` prints 23/10/2025; these
source dates are preserved without inventing another evaluation.

The selected half-life is **6.6443(9) d** (tables, section 2, page 1; comments,
section 2.1). Exact conversion with 86400 s/d gives **574067.52 s**, with
reported uncertainty **77.76 s**. LARA also prints rounded `574.07E3 s` and
`0.08E3 s`; those rounded seconds do not replace the selected days conversion.

Source precedence is explicit:

- **LNHB 2025 tables**, sections 2.1 and 5.2: beta endpoints/yields and the six
  emitted nuclear gamma lines. Direct numerical PenNuc beta records and LARA
  gamma records were checked against these printed tables.
- **Associated LNHB/LARA** `lnhb/Lu-177.lara.txt`: five compact Hf X-rays.
  Grouped XL and K-beta emissions keep the published representative energies.
- **Associated LNHB PenNuc** `lnhb/Lu-177.PenNuc.txt`: all 36 K/L1/L2/L3/M/N
  conversion-electron energy/yield records, sorted by increasing energy.
- **MIRD** `mird/Lu-177 Summary Spectrum.csv`: only the 15 rows labeled
  `Auger electron`. LNHB supplies compact Auger ranges rather than these
  detailed lines. No MIRD nuclear, X-ray or conversion data replace LNHB.
- **ENSDF** `ensdf/beta_decay_6.6443_d.pdf`: independent cross-check only.
  Its August 2019 F. G. Kondev evaluation is not merged into this reference.

## BetaShape 2.4 and ordered groups

The exact retained command in `betashape/v2.4/command.txt` is:

```text
.\betashape.exe Lu177\Lu-177.txt -csv
```

The input is `betashape/v2.4/input/Lu-177.txt`; the retained outputs are in
`betashape/v2.4/output/`. No BetaShape rerun, `-qval`, `fixint`, endpoint
rescaling, or historical spectrum is used. Each CSV copies the energy,
`dN/dE calc.` and uncertainty tokens unchanged from its selected `.bs` file.
There is no interpolation or GGEMS binning in the reference CSV. The existing
validator integrates the piecewise-linear density and conditions on its full
area for energy comparisons; it retains the separate physical branch yield.

| Group | Particle and distribution | Selected endpoint / lines | Yield per parent decay | BetaShape output |
|---|---|---|---:|---|
| 0 | Electron, RegularSpectrum | 175.5(8) keV | 0.1155 | `beta-_Lu177_trans3.bs` |
| 1 | Electron, RegularSpectrum | 247.1(8) keV | 0.00003 | `beta-_Lu177_trans2.bs` |
| 2 | Electron, RegularSpectrum | 383.8(8) keV | 0.0899 | `beta-_Lu177_trans1.bs` |
| 3 | Electron, RegularSpectrum | 496.8(8) keV | 0.7945 | `beta-_Lu177_trans0.bs` |
| 4 | Gamma, nuclear DiscreteLines | 6 | 0.1727721 | -- |
| 5 | Gamma, compact X-ray DiscreteLines | 5 | 0.08533 | -- |
| 6 | Electron, Auger DiscreteLines | 15 | 1.116556849 | -- |
| 7 | Electron, conversion DiscreteLines | 36 | 0.14733459 | -- |

The full-density beta reference means are respectively **46.696946149,
77.150926098, 110.234341530 and 147.752828806 keV**. These are independent
piecewise-linear integrals of the retained densities, not rounded header means
or compiled finite-ticket means. Pointwise uncertainties are copied, but no
covariance is supplied; this campaign compares selected central shapes.

**Model limitation:** the 383.8 and 496.8 keV transitions are first forbidden
non-unique transitions. BetaShape 2.4 treats both as allowed under the Xi
approximation and states `unpredictable: check with measurement if possible`.
This is provenance of the selected model, not a GGEMS discrepancy or evidence
that the shapes have been experimentally established. No correction or
reinterpretation is introduced here.

## Line representation and yields

`reference.json` records every selected energy, physical line intensity,
available uncertainty and raw source line. LARA percentages are divided by
100; PenNuc and MIRD yields are already per parent decay. Discrete group yields
are exact decimal sums. The beta sum is **0.99993** and the total flattened
yield is **2.521923539 particles per parent decay**. Neither sum is forced to one.
Only a conditional energy law divides line weights or spectrum density by its
own sum/integral; that operation does not normalize physical emission yields.

Current GGEMS comments document the historical integer-meV embedding, now
scaled exactly to micro-eV. The reference applies that mapping independently to
MIRD MeV energies and retains their original tokens. The sole nonzero residual
is **0.0244606 -> 0.024461 keV**, or **+400 micro-eV**, for the Auger line with
yield 0.157786. All other selected line energies are exact at that embedding.
The mapped energy is used for the finite-ticket comparison; the raw energy
remains visible as scientific evidence.

The recovered MIRD CSV contains no date, release identity or line uncertainties.
Production comments identify MIRDspecs version 20250101 / ICRP 107 (2008), but
that provenance is not independently established by the CSV. Missing
uncertainties and unavailable covariance are not invented.

## Source differences and rare statistics

The selected photon energies come from section 5.2 (emitted photons), not the
slightly different transition energies in section 2.2. The selected PenNuc
conversion records likewise remain distinct from the compact printed electron
table: for example, the first K energy is 6.2917 keV in PenNuc versus 6.2909 keV
in section 4; the PenNuc EN record at 112.68906 keV has yield 0.00439, whereas
the printed N-range entry reports 0.00390. This package preserves the selected
numeric records and does not silently reconcile energy/grouping differences.

ENSDF agrees on 6.6443(9) d but reports beta intensities 11.66%, 0.016%, 8.89%
and 79.44%, with 383.9 keV for the third endpoint. These do not replace the
selected LNHB 2025 values. MIRD non-Auger quantities also remain unselected.

The adopted 247.1 keV branch is **0.00003 +/- 0.00021 per parent**; the large
reported uncertainty does not authorize changing its central yield. Use the
normal 32-window, four-half-life campaign with automatic activity, Philox seed
77777, 256 workers and 128 population replicas. No rare branch or weak line is
amplified. Insufficient statistical evidence must remain distinct from the
deterministic reference comparison and from statistical non-rejection.
