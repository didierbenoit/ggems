# I-131 selected reference

This package reconstructs the ten ordered emission groups from the recovered
files in [../raw](../raw/). Scientific values were recovered before inspecting
the I-131 built-in. The CSVs contain retained reference densities, not GGEMS
production tables. Group order and the historical energy embedding are the
implementation mapping being compared.

## Selected authorities

The primary authority is the **V. Chiste / M.-M. Be CEA/LNE-LNHB I-131 / Xe-131
evaluation**, updated in the recovered 2013/2014 release. The table footer reads
`13/11/2001 - 20/1/2014`; the comments identify the December 2013 update; the
LNHB text records `CUT=30-NOV-2013`; PenNuc prints `17/01/2014`. These dates are
retained as source evidence, without inventing a different evaluation.

- **Half-life and six beta endpoints/yields:** `lnhb/I-131_tables.pdf`, sections
  2 and 2.1, page 1; numeric beta records in `lnhb/I-131.PenNuc.txt`.
- **Prompt nuclear gamma lines:** emitted-photon data in the tables, section
  5.2, pages 4-5, and `lnhb/I-131.lara.txt`. Transition energies and total
  gamma-plus-conversion intensities in section 2.2 are different quantities.
- **Five compact Xe X-rays:** `lnhb/I-131.lara.txt`, using its representative
  XL and K-beta group energies without artificial sub-line splitting.
- **104 prompt conversion-electron lines:** the associated LNHB numeric
  `lnhb/I-131.PenNuc.txt` records, sorted by electron energy. Retain its K, L,
  L1/L2/L3, M and N group labels and published yields.
- **13 detailed Auger lines only:** rows labeled `Auger electron` in
  `mird/I-131 Summary Spectrum.csv`. LNHB provides compact Auger ranges rather
  than this detailed line representation. No other MIRD quantity replaces LNHB.
- **ENSDF:** `ensdf/beta_decay.pdf`, July 2006, Yu. Khazov, I. Mitropolsky and
  A. Rodionov, is an independent cross-check, not the selected authority.

The selected half-life is **8.0233(19) d**. Multiplication by exactly 86400 s/d
provides **693213.12 s**, with reported uncertainty **164.16 s**. LARA's rounded
`693.21E3 s` does not replace the exact conversion of its selected days value.

## BetaShape 2.4 and physical groups

The retained command in `betashape/v2.4/command.txt` is exactly:

```text
.\betashape.exe I131\I-131.txt -csv
```

The input is `betashape/v2.4/input/I-131.txt`; selected retained outputs are in
`betashape/v2.4/output/`. There is no rerun, `-qval`, `fixint`, endpoint rescaling,
or new spectrum generation. Each reference CSV copies the energy, selected
density and corresponding uncertainty tokens unchanged. The validator
integrates their piecewise-linear density and divides by its full area only
for the conditional energy law. Physical branch yields remain separate.

| Group | Particle / distribution | Endpoint or line count | Yield per parent | Retained transition / column |
|---|---|---:|---:|---|
| 0 | Electron / RegularSpectrum | 247.9(6) keV | 0.02130 | `beta-_I131_trans5.bs`, calculated |
| 1 | Electron / RegularSpectrum | 303.9(6) keV | 0.00643 | `beta-_I131_trans4.bs`, calculated |
| 2 | Electron / RegularSpectrum | 333.8(6) keV | 0.0720 | `beta-_I131_trans3.bs`, calculated |
| 3 | Electron / RegularSpectrum | 606.3(6) keV | 0.894 | `beta-_I131_trans2.bs`, experimental |
| 4 | Electron / RegularSpectrum | 629.7(6) keV | 0.0006 | `beta-_I131_trans1.bs`, calculated |
| 5 | Electron / RegularSpectrum | 806.9(6) keV | 0.00386 | `beta-_I131_trans0.bs`, calculated |
| 6 | Gamma / DiscreteLines | 18 prompt nuclear lines | 1.002249 | LNHB/LARA |
| 7 | Gamma / DiscreteLines | 5 compact X-ray lines | 0.0597 | LNHB/LARA |
| 8 | Electron / DiscreteLines | 13 Auger lines | 0.6975269601 | MIRD |
| 9 | Electron / DiscreteLines | 104 conversion lines | 0.06307688239 | LNHB PenNuc |

**606.3 keV experimental spectrum:** `beta-_I131_trans2.bs` and `I-131.rpt`
explicitly retain the measured shape factor `1 + 0.02*W`, from H. Daniel et al.,
Z. Physik 179, 62 (1964), reference `1964DA19`, database transition 80. The
reported measurement range is 380-590 keV. The retained full-range experimental
column is the selected model; this does not claim measurements over its entire
support. BetaShape reports 190.49(22) keV for the calculated mean and 191.90(22)
keV for the experimental mean. This package selects `dN/dE exp.` and its
uncertainty, not `dN/dE calc.`.

Current GGEMS comments identify the embedded experimental table as historically
retained from BetaShape 2.2. Independently integrating the retained **2.4
experimental column** over the current 1213-bin support reproduces every
compiled ticket count. The maximum conditional-bin weight residual is
7.53e-17. No historical 2.2 input is used as active reference evidence.

For the six full-support reference densities, independent integrated means are
**68.841073459, 86.309938769, 95.903734426, 191.899276600, 199.143354422 and
280.909917039 keV**, respectively. These are neither rounded header means nor
compiled finite-ticket means. Pointwise uncertainties are retained; absent
covariance is not invented.

The 303.9 and 629.7 keV transitions are first forbidden non-unique. Their 2.4
outputs use the Xi approximation, calculate them as allowed, and report
`unpredictable: check with measurement if possible`. The 806.9 keV calculation
is first forbidden unique. These are retained model limitations, not corrections
or GGEMS discrepancies.

## Delayed Xe-131m boundary

Include the I-131 beta branch at **806.9 keV**, which populates the 163.930 keV
Xe-131m level. Exclude that level's delayed gamma and its six conversion lines
from the prompt I-131 definition. The selected daughter half-life is
**11.962(20) d** (`lnhb/I-131.txt`, level record). LNHB tables section 1 states
that the quoted 163.9 keV gamma intensity applies only at `t = tm = 14.04(9) d`.
It is not a prompt parent-time intensity. The reference explicitly lists these
seven exclusions; the current compiled definition contains none of them.
No daughter-chain emissions or separate Xe-131m definition are introduced.

## Line representation, source differences and rare statistics

Every selected line retains its physical intensity and available uncertainty.
LARA percentages are divided by 100; PenNuc and MIRD yields already refer to
one parent decay. The beta yield sum is **0.99819**; the flattened total is
**2.82074284249 particles per parent decay**. These are multiplicities, not
categorical branch probabilities; neither group nor line yields are normalized.

The historical integer-meV embedding documented by the current built-in is
preserved independently when mapping MIRD energies. The two nonzero residuals
are **0.0261504 -> 0.026150 keV (-400 micro-eV)** and
**0.0362367 -> 0.036237 keV (+300 micro-eV)**. Original MeV tokens remain in
`reference.json`; mapped energies are used for the finite-ticket comparison.
All other selected energies are exact at the historical embedding. Runtime
canonical Energy remains integer micro-eV. The recovered MIRD CSV supplies no
release identity or line uncertainties; its ICRP-107 attribution is documented
by production comments, not independently established by the CSV itself.

The selected PenNuc records are more resolved than the printed electron table.
For example, the 80.1854 keV level's K electron is 45.621 keV in PenNuc versus
45.6209 keV in printed section 4; its EN yield is 0.0002161 versus the printed
N-range 0.0001921. These distinct numeric records/groupings are not silently
reconciled. The selected shell/group-resolved numeric authority is PenNuc.

ENSDF reports **8.0252(6) d** and beta intensities **2.08%, 0.645%, 7.23%,
89.6%, 0.050% and 0.39%**, in endpoint order. It also reports Xe-131m at
11.86(4) d. These differ from the selected LNHB release and are not averaged,
normalized or substituted. The older LNHB tabulated beta mean energies are
also not used to distort the selected BetaShape 2.4 shapes.

Use the unchanged standard campaign: four half-lives, 32 windows, final-window
target 1000, seed 77777, 256 workers and 128 population replicas. Never amplify
rare branches or weak lines. Empty samples and DKW complete acceptance limits
>= 1 are `insufficient_samples`; deterministic comparisons remain available.
