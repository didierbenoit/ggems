# C-14 selected source reference

The scientific reference was recovered from `../raw/` before inspecting the
current GGEMS built-in. It uses the existing generic JSON/CSV reference format.

## Nuclear authority and emission boundary

The primary authority is the CEA/LNE-LNHB C-14 / N-14 evaluation by
M.-M. Be and V. P. Chechev, updated January 2012. See
`lnhb/C-14_com.pdf`, opening paragraph; `lnhb/C-14.txt`, update card
01-JAN-2012; and `lnhb/C-14.PenNuc.txt`, evaluation date 12/01/2012.
The retained tables PDF also carries an October 2012 footer date.

- **Half-life:** `lnhb/C-14.lara.txt`, lines 6-7, gives **5700(30) a** and
  explicitly **179.9E9(0.9E9) s**. The reference selects those seconds,
  **179900000000 +/- 900000000 s**, retaining the evaluated years separately.
  No new year conversion is substituted. The compiled scalar is exactly
  179900000000 s, matching the current 1e-14 relative scalar policy.
- **Q value / endpoint:** **156.476(4) keV**, from PenNuc's `Q` and `BEM`
  records and LNHB tables sections 1-2.
- **Branch:** **100% beta-minus to stable N-14 ground state**, parent 0+,
  daughter 1+, allowed transition. The physical yield is exactly **1 electron
  per parent decay**. The emitted antineutrino and recoil are not additional
  incident-particle groups in this source representation.
- **No selected prompt discrete emissions:** the evaluated definition has one
  beta branch and no prompt gamma or atomic-relaxation group. There is no
  daughter-chain emission, positron, ion or placeholder group. The compiled
  definition contains exactly one ordered Electron / RegularSpectrum group.

ENSDF and MIRD remain cross-checks. Neither supplies the selected spectrum.
Physical branch and total flattened yields both remain exactly **1**.

## Active experimental spectrum

The retained command in `betashape/v2.4/command.txt` is:

```powershell
.\betashape.exe C14\C-14.txt -csv
```

Active evidence is **BetaShape 2.4 (06/2024)**, specifically the `dN/dE exp.`
and adjacent uncertainty columns of `output/beta-_C14_trans0.bs`. No 2.2 file,
`-qval`, `fixint`, endpoint rescaling or recalculation is used.

The output identifies **2023SI12**, A. Singh, X. Mougeot, S. Leblond, M. Loidl,
B. Sabot and A. Nourreddine, *Nuclear Instruments and Methods in Physics
Research A* **1053**, 168354 (2023), database transition 8. Its exact factor is:

```text
1 - 0.00043*me*(W-1)
C1=-4.30E-4 37 (2023SI12)
Cexp(W) = 1+C{-1}m{-e}(W-1)
```

The coefficient's reported standard uncertainty is **0.000037** in the card's
notation. The recorded measurement range is **25-156 keV**. The selected
full-support spectrum is BetaShape's calculation using this measured factor;
the measurement itself does not cover all of **0-156.476 keV**.
The retained report assumes an allowed (`A`) transition; the `.trans` file
also carries its general warning that information about this decay is not
totally sure. No model correction is introduced.

`beta_minus_156_476_keV.csv` copies all **314 raw energy, experimental density
and uncertainty tokens unchanged**, from 0 through 156.476 keV. It contains
the retained 0.5 keV sampling grid and final 0.476 keV interval, not GGEMS bins.
The full piecewise-linear area is **1.000002347658280**; conditional comparisons
divide by that area, while the physical yield remains separate and unchanged.
Pointwise uncertainties are retained; no covariance is available for an
uncertainty-aware spectrum-equivalence test.

| Mean-energy quantity | keV | Selection |
|---|---:|---|
| Historical LNHB evaluated mean | 49.16(1) | Context from the 2012 allowed calculation |
| BetaShape 2.4 calculated header mean | 49.2606(14) | Not selected |
| BetaShape 2.4 experimental header mean | 48.9252(14) | Active experimental factor |
| Full retained experimental CSV, piecewise-linear mean | 48.92532644595687 | Independent reference |

These values are not interchangeable and are not averaged. The historical
LNHB mean does not override the active experimental shape. Integrating the
retained calculated column separately gives 49.25719065714020 keV; this is
a printed-grid integral, distinct from its BetaShape header mean and not an
additional active spectrum.

## Current representation comparison

| Quantity | Result |
|---|---:|
| Full reference support keV | [0, 156.476] |
| Compiled half-open support keV | [0.000414, 156.476) |
| Bins / regular width keV | 313 / 0.499922 |
| Compiled normalized-weight grid mean keV | 48.92574034385963 |
| Exact finite-ticket mean keV | 48.9257402825096247904002666473388671875 |
| Full continuous-grid/reference CDF supremum | 6.52259335523e-6 |
| Reference probability below compiled lower edge | 4.17128212849e-6 |

The documented historical even-integer-meV grid reconstructs independently
from the selected endpoint and 0.5 keV maximum width. Its energy values are
currently stored as integer micro-eV. Integration of the selected raw density
over those bins, conditioned on the compiled support for this representation
audit only, reproduces the normalized weights within **5.14e-19** and all
**313 ticket allocations exactly**. Independent allocation from the compiled
weights gives the same result. Every positive bin is reachable; the smallest
has **425 tickets** out of 2^32. The exact finite-ticket mean is
`2626680680574607991/53687091200000000` keV.

The generic full-reference comparison retains the excluded lower tail and
reports its small representation residual; no tail or acceptance threshold
is adjusted. This agreement establishes that the current table is the
selected Singh-based 2.4 experimental spectrum, not the previous pre-Singh
shape. No discrete-line reference or discrete-line comparison is applicable.

## Source differences

The recovered ENSDF evaluation (Chechev, April 1998) agrees on 5700(30) years,
unit beta yield and the ground-state daughter, but gives **156.475(4) keV**
and mean **49.47 keV**. The MIRD summary contains one unit-yield beta entry
with mean **49.4533 keV**. These are not substituted for selected LNHB nuclear
values or the active BetaShape experimental spectrum.
LNHB's comments discuss discrepant endpoint measurements and explicitly adopt
156.476(4) keV; this package preserves that selection.
