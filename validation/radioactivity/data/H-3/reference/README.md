# H-3 selected scientific reference

## Authority and decay

Primary nuclear authority is the LNHB/DDEP H-3 / He-3 evaluation by
**V. P. Chechev**, revised April 2006: `../raw/lnhb/H-3_com.pdf`.
The associated PenNuc evaluation is dated 31/05/2006; the tables carry
11/07/1998 - 28/6/2006. The historical 1998 header in the retained ENSDF-format
LNHB input does not replace this recovered 2006 evaluation context.

LARA (`H-3.lara.txt`, lines 6-7) publishes **12.312(25) a** and
**388.5E6(0.8E6) s**. The reference uses **388500000 s**, standard uncertainty
**800000 s**, directly from the published seconds. No new year conversion is
introduced. The comments note that the evaluated half-life concerns molecular
tritium; their discussion of other forms is not used to change that selection.

The decay is **100% beta-minus to stable He-3 ground state**. PenNuc records
one branch with yield **1 electron per parent decay**, daughter level zero.
There are no selected prompt gamma or atomic-relaxation lines. Group 0 is the
single Electron / RegularSpectrum law; no discrete, positron, ion, recoil,
antineutrino or placeholder incident-particle group is introduced.

## Q, transition energy, and emitted-electron endpoint

These quantities are explicitly separate in the selected evidence:

| Quantity | Selected value | Evidence |
|---|---:|---|
| Nuclear Q / atomic mass-energy difference | 18.591(1) keV | PenNuc Q record; LNHB comments cite 2003Au03 |
| Evaluated atomic electron endpoint | 18.564(2) keV | PenNuc BEM record; LNHB comments section 2.1 |
| BetaShape transition-energy axis endpoint | 18.591(1) keV | `H-3.trans`, line 9; spectrum header and final knot |

LNHB section 2.1 gives
`Emax = DeltaM*c^2 - E_recoil - [B(He)-B(T)] + [B(RHe+)-B(RT)]`.
Its atomic terms are approximately **-3.4 -64.3 +40.82 eV**, explaining the
approximately 27 eV separation from Q. Chemical form and final electronic
states affect experimental endpoints. The comments derive an atomic endpoint
and adopt 18.564(2) keV; they also list different measured endpoints for real
sources. Thus 18.564 keV is not one universally applicable direct measurement.

There is a documentary uncertainty disagreement: `H-3_tables.pdf`, section 3,
prints **18.564(3) keV**, whereas the comments, PenNuc and retained BetaShape
input use **18.564(2) keV**. Both are retained as evidence; no average is made.

BetaShape explicitly says: **"Transition energy calculated from Q-value and
level energies: 18.591 (1) keV. For information, given measured Emax: 18.564
(2) keV."** The reference CSV retains that Q-derived axis. It is not rescaled
or clipped at the separately evaluated atomic endpoint, and it is not claimed
to model the chemical-form-dependent endpoint of an individual experiment.
The distribution's `endpoint` field describes the BetaShape axis; the
`evaluated_electron_endpoint` field preserves the distinct nuclear evidence.

## Selected BetaShape spectrum

The exact retained command in `../raw/betashape/v2.4/command.txt` is:

```text
.\betashape.exe H3\H-3.txt -csv
```

Active evidence is **BetaShape 2.4 (06/2024)**, allowed (`A`) transition,
`../raw/betashape/v2.4/output/beta-_H3_trans0.bs`. No BetaShape 2.2 file,
`-qval`, `fixint`, endpoint rescaling or new calculation is used.

The selected column is **`dN/dE exp.`**, with its adjacent uncertainty.
The factor is **`Cexp(W)=1 (1973PI01)`**, associated with **W. F. Piel Jr,
Nuclear Physics A 203, 369 (1973)**. The recorded measurement range is
**11-18 keV**, database transition #1. The 0-18.591 keV reference is
BetaShape's full-support spectrum using this experimental factor; the
measurement itself does not cover that full support.

The retained output lists screening, radiative, atomic-exchange and
atomic-overlap corrections and provides distinct calculated and experimental
columns. A constant experimental factor does not establish column equality.
The files do not isolate each correction's contribution to their difference.
`H-3.trans` also retains the warning "information about this decay is not
totally sure"; no unrecorded correction is inferred from that warning.

| Mean energy | keV |
|---|---:|
| Historical LNHB evaluated mean | 5.68(1) |
| BetaShape calculated header, unselected | 5.68012(32) |
| BetaShape experimental header, selected | 5.69565(32) |
| Full retained experimental piecewise-linear integral | 5.6956676692186463354 |
| Full retained calculated piecewise-linear integral, cross-check only | 5.6796845619575559096 |

The header values and integrals of the rounded output grid are different
quantities. Neither the calculated column nor the historical evaluated mean
overrides the selected experimental shape.

`beta_minus_18_591_keV.csv` preserves **311** raw energy, experimental density
and uncertainty tokens unchanged: 0 to 18.591 keV, 0.06 keV nominal spacing,
with the final 0.051 keV interval. The experimental piecewise-linear area is
**1.000000283866845**. Only the conditional spectrum law is normalized by the
existing validator. The physical branch and total particle yields remain **1**.
No compiled bins are inserted into the independent CSV. Pointwise
uncertainties are retained without inventing an uncertainty covariance model.

## Compiled representation comparison

The current built-in explicitly documents its historical BetaShape 2.2
experimental-table provenance and deliberate preservation of the 18.591 keV
axis. Validation uses the active 2.4 evidence above. Its single compiled group
has the selected half-life, particle, kind and unit yield, with **38 bins**,
width **0.489236 keV**, support **[0.000032, 18.591) keV**. Current storage is
integer micro-eV, retaining the historical even-integer-meV grid convention.

All 38 ticket allocations reconstruct exactly from compiled bin weights;
the minimum positive allocation is **145464**, out of 2^32 tickets. Direct
allocation from the central 2.4 reference bin integrals differs in **22 bins**,
by at most **25 tickets**. Maximum conditional-weight difference is
**5.80339e-9**. All compiled weights are compatible with conservative bounds
from the printed densities' final decimal places. This is a precision
diagnostic, not exact central-value equality or a new statistical tolerance.

The compiled normalized-weight grid mean is **5.69730696854864447 keV**;
the same grid independently integrated from 2.4 gives
**5.69730698279939202 keV**. The exact finite-ticket mean is
**6117436772998446149 / 1073741824000000000 keV**, or
**5.697306965476317469961941242218017578125 keV**.

The full-reference/grid CDF supremum is **4.99184042007e-4**. The reference
mass below the compiled lower edge, **2.62923474975e-6**, remains visible in
the deterministic comparison. The full reference is neither truncated nor
renormalized to hide this residual. These representation effects are separate
from the much smaller historical-table/published-density difference.

## Independent cross-checks and limits

The recovered ENSDF June 2015 evaluation agrees on the single unit-yield
ground-state branch, but gives half-life **12.32(2) y**, parent Q
**18.5906(32) keV**, and tabulated average beta energy **5.6817(12) keV**.
It also discusses recoil and residual-state differences between mass
difference and measured endpoint. MIRD's summary gives one beta with mean
**5.67977 keV** and unit yield. Neither source overrides the selected LNHB
nuclear values or the BetaShape experimental density.

No material mismatch with the selected spectral evidence was found. The
small central-table and full-reference representation residuals are reported
explicitly, without changing the scientific reference or statistical budget.
