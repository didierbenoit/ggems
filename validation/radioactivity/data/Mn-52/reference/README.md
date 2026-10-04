# Mn-52 selected source reference

## Authority and model

LNHB/DDEP, Aurelian Luca (IFIN-HH), literature through early November 2020; tables 18/02/2016 - 22/11/2020; PenNuc 22/11/2020. The retained LNHB evaluation is the selected nuclear authority. ENSDF and MIRD/MIRDsoft are independent cross-checks; no conflicting evaluations are averaged.

Electron capture and beta-plus decay to Cr-52; selected summed positron yield 0.3116242 per parent decay. The definition contains independent marginal emissions per parent decay. Their physical yields are not a categorical distribution and need not sum to one. Selected beta branches remain separate, in increasing endpoint order, followed by nuclear gamma rays, compact X rays, and conversion-electron groups in increasing parent-transition energy order.

The 26 groups have total expected yield **3.49357441814143 primaries per parent decay** within the selected model. This is not a complete decay-energy balance: the exclusions below are physically significant.

## Half-life and decay energy

Select `5.592 +/- 0.003 d` from the evaluation. Multiply the evaluated value and standard uncertainty by exactly 86400 s/d; retain the evaluated central value rather than rounded LARA/BetaShape seconds. This gives **483148.800 s**, with propagated standard uncertainty **259.200 s**. The exact unit conversion introduces no additional uncertainty.

The parent Q value is **4712.0 +/- 1.9 keV**. Individual beta endpoints differ where a daughter excited state is populated. EC Q values are not positron kinetic endpoints.

## Ordered source inventory

| Index | Group | Particle | Law | Yield per parent decay |
| --- | --- | --- | --- | --- |
| 0 | `beta_plus_74_1_keV` | Positron | RegularSpectrum | 0.0000133 |
| 1 | `beta_plus_274_7_keV` | Positron | RegularSpectrum | 0.0000009 |
| 2 | `beta_plus_576_1_keV` | Positron | RegularSpectrum | 0.3085 |
| 3 | `beta_plus_922_2_keV` | Positron | RegularSpectrum | 0.00011 |
| 4 | `beta_plus_1320_3_keV` | Positron | RegularSpectrum | 0.003 |
| 5 | `nuclear_gamma` | Gamma | DiscreteLines | 3.004633 |
| 6 | `atomic_xray` | Gamma | DiscreteLines | 0.1767 |
| 7 | `conversion_200_58_keV` | Electron | DiscreteLines | 0.0000122133 |
| 8 | `conversion_346_03_keV` | Electron | DiscreteLines | 0.0000374143 |
| 9 | `conversion_398_09_keV` | Electron | DiscreteLines | 0.00000134052 |
| 10 | `conversion_399_542_keV` | Electron | DiscreteLines | 0.0000027821 |
| 11 | `conversion_501_97_keV` | Electron | DiscreteLines | 0.00000144081 |
| 12 | `conversion_600_42_keV` | Electron | DiscreteLines | 0.00000199633 |
| 13 | `conversion_647_537_keV` | Electron | DiscreteLines | 0.00000121469 |
| 14 | `conversion_744_213_keV` | Electron | DiscreteLines | 0.0003007403 |
| 15 | `conversion_848_134_keV` | Electron | DiscreteLines | 0.00000665519 |
| 16 | `conversion_901_87_keV` | Electron | DiscreteLines | 7.7264E-8 |
| 17 | `conversion_935_519_keV` | Electron | DiscreteLines | 0.0001738738 |
| 18 | `conversion_1045_72_keV` | Electron | DiscreteLines | 8.7398E-8 |
| 19 | `conversion_1246_36_keV` | Electron | DiscreteLines | 0.00000364490 |
| 20 | `conversion_1247_81_keV` | Electron | DiscreteLines | 3.27040E-7 |
| 21 | `conversion_1333_614_keV` | Electron | DiscreteLines | 0.000004098849 |
| 22 | `conversion_1434_06_keV` | Electron | DiscreteLines | 0.0000692727 |
| 23 | `conversion_1645_781_keV` | Electron | DiscreteLines | 2.49629E-8 |
| 24 | `conversion_1981_07_keV` | Electron | DiscreteLines | 1.292537E-8 |
| 25 | `conversion_2257_36_keV` | Electron | DiscreteLines | 7.6216E-10 |

Every selected line, energy uncertainty, absolute yield, and available yield uncertainty is listed in [reference.json](reference.json). LARA photon intensities in percent are divided by exactly 100. PenNuc EK/EL/EM/EN records are already absolute per-parent yields; they are not multiplied by gamma or EC probabilities. Group yield is the sum of the selected absolute line yields. Group uncertainty is not inferred by treating correlated lines as independent.

The compact LARA X-ray entries retain their supplied representative energies and absolute intensities, consistent with existing GGEMS discrete-line references. More detailed relative atomic-line tables are not added again. PenNuc grouped M/N energies are retained as supplied; small differences between conversion-line plus binding energies and photon energies are not silently corrected.

Conversion electrons are grouped by nuclear transition. This preserves every positive retained shell line, including lines too small to receive a ticket if all conversion transitions were collapsed into one conditional distribution. It does not amplify any physical yield or introduce emission correlations. The existing finite-ticket allocator and Source semantics are unchanged.

## BetaShape selection and continuous references

The retained command is reproduced verbatim:

```text
.\betashape.exe .\Mn52\Mn-52.txt fixint=1 -csv
```

The retained executable output identifies **BetaShape 2.4 (06/2024)**. No new run was performed. `fixint=1` intentionally preserves the evaluated EC/beta-plus split. Every selected transition explicitly reports no tabulated experimental shape factor. Each reference CSV copies the raw energy token, `dN/dE calc.` token, and adjacent pointwise uncertainty token unchanged. The full raw support is retained, including zero-energy and endpoint entries. No resampling or density renormalization is stored in these CSVs.

For adjacent points `(a,c)` and `(b,d)`, the independent area is `(b-a)(c+d)/2`; the first moment is `(b-a)[a(2c+d)+b(c+2d)]/6`. Exact rational arithmetic on the retained decimal tokens is used before decimal rendering. Conditional analysis divides by the independently integrated area. These source densities may already carry a rounded branch intensity; their area is not substituted for the selected physical yield. Pointwise uncertainty tokens are retained, but their correlations are unavailable, so no uncertainty on the integrated mean is invented.

| Spectrum | Raw points | Piecewise-linear area | Conditional mean (keV) |
| --- | --- | --- | --- |
| [beta_plus_74_1_keV.csv](beta_plus_74_1_keV.csv) | 372 | 0.000013299999912936 | 34.23727657947319 |
| [beta_plus_274_7_keV.csv](beta_plus_274_7_keV.csv) | 307 | 9.000000741074995e-7 | 132.2551328683899 |
| [beta_plus_576_1_keV.csv](beta_plus_576_1_keV.csv) | 578 | 0.3085000093553 | 241.6338418181333 |
| [beta_plus_922_2_keV.csv](beta_plus_922_2_keV.csv) | 309 | 0.000110000044185168 | 418.0421391747120 |
| [beta_plus_1320_3_keV.csv](beta_plus_1320_3_keV.csv) | 332 | 0.003000001139599294 | 594.4931476800580 |

Exact raw output paths, endpoint uncertainties, branch-yield uncertainties, and BetaShape header statements are recorded in `reference.json`. Branch spectra are not combined or weighted a second time at runtime.

## Cross-checks and disagreements

Yang Dong and Huo Junde, NDS 128, 185 (2015), cutoff July 10, 2015: 5.591(3) d, Q=4711.2(19) keV. Dominant beta-plus and EC branches are 29.4(4)% and 61.4(6)%, versus LNHB 30.85(43)% and 59.9(7)%. The weakest 74 keV positron branch is 0.00118(14)% versus selected 0.00133(17)%.

The retained fixint=1 run preserves the input EC/beta-plus split. The five selected positron yields sum to 31.16242%; the rounded EC branches sum to 68.8877%. Their 100.05012% combined sum is a published rounding/evaluation inconsistency, not a reason to renormalize source yields.

The 1320.3, 922.2 and 274.7 keV branches are second-forbidden non-unique transitions calculated by BetaShape as first-forbidden unique under the Xi approximation. The retained outputs explicitly warn that the result is unpredictable and should be checked against measurement. Agreement with these selected calculated laws does not establish their physical accuracy.

LNHB/PenNuc gives uncertainty 0.0043 on the 0.3085 dominant positron yield; the BetaShape header prints 0.0044. LNHB supplies the selected yield uncertainty; the raw adjacent density uncertainty tokens remain untouched.

PenNuc endpoints retain level-subtraction digits (1320.346, 922.214, 576.117, 274.67 and 74.054 keV); the selected BetaShape grids end at 1320.3, 922.2, 576.1, 274.7 and 74.1 keV. The full raw supports are preserved. The selected gamma inventory has 1247.81 keV, versus 1247.85 keV in evaluation comments.

MIRD detailed atomic and conversion inventories have different line granularity and yields from the selected LNHB inventory. They are not added, averaged, or used to close an energy or yield balance.

Retained MIRD/MIRDsoft summary CSV totals are shown below. Their evaluation identity and parent half-life are not established by the CSV itself. Different atomic granularity, low-energy rows, and a different selected evaluation can change these totals. They are not used to fill missing selected LNHB energy laws. DPK files are dose-deposition data, not emission probabilities.

| MIRD type | Rows | Yield per nuclear transformation | Conditional mean (keV) |
| --- | --- | --- | --- |
| X-ray | 25 | 4.696513148971796 | 0.204154728924 |
| Gamma | 21 | 3.000137 | 1051.40553019 |
| Annihilation photon | 1 | 0.59323 | 511.000 |
| Positron | 2 | 0.2966151837 | 241.790469579 |
| Auger electron | 7 | 3.81777730 | 0.738590085465 |
| Conversion electron | 76 | 0.000600423947621769 | 850.944784200 |

The independent piecewise-linear integral of the retained MIRD beta CSV is 0.2969340446253956, with conditional mean 242.3278392148189 keV. This is separate from its summary-spectrum mean and from the selected BetaShape branch laws.

## Exclusions and scope

LNHB gives nonzero Auger family totals: L=0.0674 +/- 0.0010 and K=0.434 +/- 0.007 electrons per parent decay. It supplies energy ranges rather than a complete conditional energy law in the retained selected inventory. These Auger emissions are therefore excluded from this source definition; they are not claimed to be absent physically. Substituting arbitrary monoenergies, uniform ranges, or MIRD lines would add an unsupported selection.

No source-level 511 keV annihilation pair is included. EC probabilities do not generate particle placeholders. Internal-pair coefficients without a complete selected energy law do not define additional groups. Neutrinos, recoil nuclei, later daughter decays, zero-energy placeholders, and unobserved upper-limit branches are outside this model. Nuclear cascade correlations and daughter-level delays are not represented by these independent prompt marginals.

## Compiled representation and validation limits

Integer Energy storage is in micro-eV. The beta grid uses `ceil(endpoint / 0.5 keV)` bins and the largest even integer-meV width leaving a strictly positive lower edge while preserving the exact selected upper endpoint. Conditional bin masses are integrated offline from the independent piecewise-linear reference, normalized over retained grid support, then passed to the existing ticket constructor. The full continuous reference retains the small lower-edge mass excluded by this finite representation.

| Branch | Bins | Width (micro-eV) | Lower edge (micro-eV) | Excluded conditional mass | Grid mean (keV) | Minimum tickets |
| --- | --- | --- | --- | --- | --- | --- |
| `beta_plus_74_1_keV` | 149 | 497314000 | 214000 | 1.45942900354e-14 | 34.23727656912598 | 2464 |
| `beta_plus_274_7_keV` | 550 | 499454000 | 300000 | 2.56123867799e-14 | 132.2551328718790 | 305 |
| `beta_plus_576_1_keV` | 1153 | 499652000 | 1244000 | 2.14289585849e-13 | 241.6338418583395 | 27 |
| `beta_plus_922_2_keV` | 1845 | 499836000 | 2580000 | 2.12877637763e-12 | 418.0421391879929 | 21 |
| `beta_plus_1320_3_keV` | 2641 | 499924000 | 716000 | 1.32150704627e-13 | 594.4931476838414 | 6 |

These pre-build exact-integration results satisfy the unchanged 0.005 keV full-reference mean budget inherited from H-3/C-14/P-32/P-33. Exported binary64 weights and their actual finite tickets are checked independently before campaigns.

The default generic campaign uses 32 consecutive half-open windows, requests four compiled half-lives, seed 77777, 256 workers per device, a target of 1000 expected total primaries in the final window, and 128 host population replicates. Rare conversion groups can legitimately have much less than one expected observation. Their stochastic checks may remain `insufficient_samples`; all selected lines still require deterministic energy/yield/ticket checks. No rare group is amplified.

Nuclear-data uncertainty, tabulated-continuous interpolation, finite-grid/ticket representation, and Monte Carlo acceptance are separate issues. The generic deterministic spectrum diagnostic remains `comparison_only`; stochastic tests use the unchanged fixed budgets. CPU/GPU runs with the same seed share the host population experiment and are not independent population evidence. Source validation does not establish transport-dose accuracy or the physical completeness of excluded channels.

The `implementation_audit` uses the current source-comment convention to check the compiled file's BetaShape version. Campaign results and exported-definition checks belong in the accompanying integration report; they do not redefine the reference.

## Immutable provenance

All retained raw files are listed with byte counts and SHA-256 hashes in `reference.json`. The inventory includes LNHB tables/comments/LARA/PenNuc/input, retained BetaShape command/input/output, ENSDF, and MIRD/MIRDsoft files. Raw bytes are not regenerated, edited, renamed, or included as modifications in the integration patch.
