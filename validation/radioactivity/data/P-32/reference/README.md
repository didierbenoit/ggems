# P-32 selected source reference

For build instructions, campaign commands, and result interpretation, see the
[radionuclide validation guide](../../../README.md). Select this package with
`--reference validation/radioactivity/data/P-32/reference/reference.json`.

The scientific reference was recovered from the immutable local `../raw/`
package before generating the new production arrays. LNHB supplies the selected
nuclear evaluation; BetaShape supplies the selected continuous beta law. ENSDF
and MIRD/MIRDsoft remain independent cross-checks. Existing GGEMS output is not
the scientific oracle. No raw file was modified or regenerated.

## Nuclear authority and emission boundary

The selected authority is **CEA/LNE-LNHB, M.-C. Lépy, March 2026**, with
literature through **February 2026**. The tables carry dates
**2026-02-19 / 2026-03-27**; the PenNuc evaluation is dated
**2026-03-27**. See `lnhb/P-32_com.pdf`, sections 1-1.2,
`lnhb/P-32_tables.pdf`, sections 1-2 and 4, and the retained LNHB text records.

The decay is **100% allowed beta-minus to stable S-32 ground state**
(1+ to 0+). Q and the selected transition endpoint are
**1710.661 +/- 0.040 keV**,
adopted from AME2020 in the evaluation. The PenNuc `BEM` record gives unit branch
probability and a zero-energy daughter level. The source contains exactly one
ordered **Electron / RegularSpectrum** group, with physical yield and total
yield both exactly **1 electron per parent decay**.

The LARA file states `No emissions`: it lists no separate photon or atomic
source lines. This does not remove the beta branch documented in PenNuc and the
tables. The sulfur X-ray/Auger energies and relative atomic intensities in
section 3 are not absolute per-parent source yields. No gamma, X-ray, Auger,
antineutrino, recoil, daughter-chain or zero-energy placeholder group is added.

## Half-life selection and conversion

The adopted half-life is **14.273 +/- 0.007 d**.
The comments, tables, LNHB parent record and LARA days field agree on this
central value. Use exact **86400 s/d** for both the value and its standard
uncertainty:

```text
T = 14.273 * 86400 = 1233187.200 s
u(T) = 0.007 * 86400 = 604.800 s
```

The conversion factor has no uncertainty. The retained LARA and BetaShape
outputs separately print **1233200 +/- 600 s**.
Those rounded seconds differ from the exact conversion by
**+12.8 s**. They are retained as provenance, not substituted for the
more precise evaluated central value. This follows the explicit conversion
practice used for F-18 and Tc-99m. The compiled scalar is
**1233187.2 s**; scientific uncertainty is not an extra numerical tolerance.

## Selected BetaShape law

The retained command in `betashape/v2.4/command.txt` is:

```powershell
.\betashape.exe .\P32\P-32.txt -csv
```

The active output is **BetaShape 2.4 (06/2024)**,
`betashape/v2.4/output/beta-_P32_trans0.bs`. The command uses the retained
LNHB input without a Q-value override or intensity override. The executable was
not rerun; its original execution timestamp is unavailable. The output lists
screening, radiative, atomic exchange and atomic overlap corrections. Its
`.trans` file retains the general warning that information about this decay is
not totally sure; no additional correction is inferred from that warning.

The selected column is **`dN/dE exp.`**, with its adjacent uncertainty.
The qualified experimental factor is **`1 - 0.019*W`**, with coefficient card
`C1=-0.019 3 (1973WI**)` (coefficient standard uncertainty 0.003). BetaShape
attributes it to **W. Wiesner, D. Flothmann and H. J. Gils, Nuclear Instruments
and Methods 112, 449 (1973)**, database transition 16. The measured range is
**200-1635 keV**. The full-support BetaShape law uses that factor beyond the
measurement interval; it is not a measurement over the full energy support.

This selection follows the H-3/C-14 precedent for qualified tabulated
experimental shape factors. **LNHB explicitly adopts the theoretical mean
693.306(18) keV**, while also discussing the experimental-shape result
689.484(18) keV. The two laws are distinct. No average is taken, no density is
altered to force either mean, and the experimental result is not relabeled as
LNHB's adopted theoretical mean.

| Mean-energy quantity | keV | Role |
|---|---:|---|
| LNHB adopted / BetaShape calculated header | 693.306(18) | Evaluated theoretical context |
| BetaShape experimental header | 689.484(18) | Selected experimental shape |
| Selected transition CSV piecewise-linear mean | 689.48646212634046 | Independent conditional reference |
| Calculated column piecewise-linear mean | 693.30477795711105 | Unselected cross-check |

The selected transition triples and the total-spectrum triples are identical.
The `.new` and `.rpt` files record BetaShape's replacement of the input EAV by
the experimental-shape mean; that generated record does not rewrite the LNHB
evaluation's choice.

## Independent continuous reference

`beta_minus_1710_661_keV.csv` copies all **344 raw energy, selected
density and adjacent uncertainty tokens unchanged**, over **[0,
1710.661] keV**. The nominal raw spacing is
**5 keV**, with a final **0.661 keV** interval.
There is no endpoint rescaling, truncation, density renormalization in the CSV,
or insertion of compiled GGEMS bins.

The piecewise-linear area is **0.999997246410269405**. The
conditional mean is **689.48646212634048 keV**.
These are independently calculated integrals of the rounded tabulated density,
not BetaShape's internal quadrature or its separately reported header mean.
For each segment [a,b] with endpoint densities c,d, the area is
`(b-a)*(c+d)/2` and the first moment is
`(b-a)*(a*(2*c+d)+b*(c+2*d))/6`. Conditional comparisons divide by the full
area. Physical yield remains separately equal to 1.

The CSV preserves pointwise standard uncertainties, but the raw package
provides no covariance model. The current tests condition on the central law;
they do not treat density uncertainties as independent measurements or claim
uncertainty-aware spectral equivalence.

## Compiled representation comparison

The regular-grid rule is fixed before comparison: `N = ceil(endpoint / 0.5 keV)`;
choose the largest even integer-meV width at most `endpoint/N` that leaves a
strictly positive lower edge, and set `lower = endpoint - N*width`. Multiply
meV integers by exact 1000 for canonical integer micro-eV storage. This keeps
the selected upper endpoint exactly and follows the existing beta built-ins.

| Quantity | Result |
|---|---:|
| Table count | 3422 |
| Width, integer micro-eV | 499900000 |
| Lower edge, integer micro-eV | 3200000 |
| Upper edge, integer micro-eV, exclusive | 1710661000000 |
| Normalized-weight grid mean, keV | 689.48708830766680 |
| Exact finite-ticket law mean, keV | 689.48708763645743 |
| Finite-ticket mean minus full reference mean, keV | 0.000625510117061 |
| Continuous-grid/reference CDF supremum | 8.99689669952e-07 |
| Full reference probability below the lower edge | 8.99689669952e-07 |
| Minimum positive bin allocation, out of 2^32 | 1 |

Offline exact rational integration of the selected piecewise-linear law over
each bin, conditioned on the retained support for the compiled representation
only, reproduces every exported binary64 weight after rounding. Independent
Hamilton apportionment of both the exact reference bin masses and the exported
binary64 weights reproduces all **3422** ticket allocations exactly.
Every positive bin remains reachable, and the final cumulative bound is 2^32.
The finite-ticket mean is exactly
`185082780775804414873/268435456000000000` keV.

The full independent reference retains the lower tail. Its deterministic
comparison status remains **`comparison_only`**, not an uncertainty-aware
physics acceptance result. The focused mean test uses the existing H-3/C-14
**0.005 keV** representation budget against the independent full-support
mean, fixed before generating the arrays. No residual is added to the
generic campaign's statistical thresholds.

## Independent cross-checks and disagreements

The retained ENSDF evaluation is **Jun Chen, NDS 201, 1 (2025), cutoff 2024-10-31**.
It confirms the single unit-yield ground-state branch. It gives half-life
**14.266 +/- 0.004 d**,
parent Q **1710.66 +/- 0.04 keV**,
and average beta energy **694.587 +/- 0.019 keV**.
Its beta-radiation table separately prints decay energy
**1710.7 +/- 1.4 keV**;
that displayed uncertainty is not substituted for its parent-Q uncertainty or
the selected LNHB endpoint uncertainty.

The MIRD summary contains one unit-yield beta entry, mean
**694.777 keV**. The separately retained
MIRD beta-density CSV has 117 points;
its piecewise-linear area is
**0.9999819066** and its
conditional mean is
**695.986357210506 keV**.
That coarse-grid integral differs from the summary mean and is not a new
selected evaluation. Evaluation identity and half-life are not established by
these recovered MIRD CSVs. No half-life or ICRP edition is inferred from them.
The retained DPK describes absorbed dose versus radius, not a Source spectrum;
physical transport/dose validation is outside this package.

None of these differences is averaged or used to replace the selected LNHB
values or the selected BetaShape column.

## Raw inventory and validation scope

`reference.json` records each of the **18 raw files**, its byte size and SHA-256:
five LNHB files (input, LARA, PenNuc, tables and comments), one ENSDF PDF, three
MIRD CSVs (beta spectrum, summary and DPK), and nine BetaShape files (command,
input, transition/total spectra, summary CSV, `.new`, `.read`, `.rpt`, `.trans`).
The command and full outputs are evidence from the retained local run; no
external evaluation or new BetaShape run is substituted.

Validation separates nuclear scalars and group mapping, exact compiled-grid
construction, independent integrated decay counts, Poisson populations,
conditional radioactive birth times and conditional energy distributions.
The unmodified generic protocol uses four compiled half-lives, 32 consecutive
half-open windows, Philox seed 77777, 256 workers and 128 additional independent
host population replicates. CPU and GPU campaigns using the same seed share
the host population experiment and must not be pooled as independent samples.
Worker scheduling does not guarantee per-history bitwise CPU/GPU equality.

The production source comment is checked through `implementation_audit`;
this is provenance text, not a compiled provenance API. Source births pass
through the current diagnostic transport path. The package does not validate
navigation, physical electron stopping, dose, daughter chains or a complete
transport physics model. Campaign statuses and counts belong to the retained
campaign `analysis.json`, rather than being inferred from exporter exit codes.
