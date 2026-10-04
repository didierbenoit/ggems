# Cu-67 selected source reference

## Authority and model

CEA/LNE-LNHB, X. Mougeot, literature through mid-February 2026; tables 15/02/2026 - 12/03/2026; PenNuc 15/02/2026. The retained LNHB evaluation is the selected nuclear authority. ENSDF and MIRD/MIRDsoft are independent cross-checks; no conflicting evaluations are averaged.

100% beta-minus decay to Zn-67. The definition contains independent marginal emissions per parent decay. Their physical yields are not a categorical distribution and need not sum to one. Selected beta branches remain separate, in increasing endpoint order, followed by nuclear gamma rays, compact X rays, and conversion-electron groups in increasing parent-transition energy order.

The 12 groups have total expected yield **1.86371688758 primaries per parent decay** within the selected model. This is not a complete decay-energy balance: the exclusions below are physically significant.

## Half-life and decay energy

Select `61.82 +/- 0.05 h` from the evaluation. Multiply the evaluated value and standard uncertainty by exactly 3600 s/h; retain the evaluated central value rather than rounded LARA/BetaShape seconds. This gives **222552.00 s**, with propagated standard uncertainty **180.00 s**. The exact unit conversion introduces no additional uncertainty.

The parent Q value is **560.8 +/- 0.8 keV**. Individual beta endpoints differ where a daughter excited state is populated. EC Q values are not positron kinetic endpoints.

## Ordered source inventory

| Index | Group | Particle | Law | Yield per parent decay |
| --- | --- | --- | --- | --- |
| 0 | `beta_minus_167_3_keV` | Electron | RegularSpectrum | 0.01039 |
| 1 | `beta_minus_376_2_keV` | Electron | RegularSpectrum | 0.51735 |
| 2 | `beta_minus_467_5_keV` | Electron | RegularSpectrum | 0.19734 |
| 3 | `beta_minus_560_8_keV` | Electron | RegularSpectrum | 0.27492 |
| 4 | `nuclear_gamma` | Gamma | DiscreteLines | 0.663143 |
| 5 | `atomic_xray` | Gamma | DiscreteLines | 0.06130 |
| 6 | `conversion_91_27_keV` | Electron | DiscreteLines | 0.00575197 |
| 7 | `conversion_93_31_keV` | Electron | DiscreteLines | 0.1259723 |
| 8 | `conversion_184_579_keV` | Electron | DiscreteLines | 0.0075078 |
| 9 | `conversion_208_943_keV` | Electron | DiscreteLines | 0.00000962492 |
| 10 | `conversion_300_212_keV` | Electron | DiscreteLines | 0.00002836413 |
| 11 | `conversion_393_521_keV` | Electron | DiscreteLines | 0.00000382853 |

Every selected line, energy uncertainty, absolute yield, and available yield uncertainty is listed in [reference.json](reference.json). LARA photon intensities in percent are divided by exactly 100. PenNuc EK/EL/EM/EN records are already absolute per-parent yields; they are not multiplied by gamma or EC probabilities. Group yield is the sum of the selected absolute line yields. Group uncertainty is not inferred by treating correlated lines as independent.

The compact LARA X-ray entries retain their supplied representative energies and absolute intensities, consistent with existing GGEMS discrete-line references. More detailed relative atomic-line tables are not added again. PenNuc grouped M/N energies are retained as supplied; small differences between conversion-line plus binding energies and photon energies are not silently corrected.

Conversion electrons are grouped by nuclear transition. This preserves every positive retained shell line, including lines too small to receive a ticket if all conversion transitions were collapsed into one conditional distribution. It does not amplify any physical yield or introduce emission correlations. The existing finite-ticket allocator and Source semantics are unchanged.

## BetaShape selection and continuous references

The retained command is reproduced verbatim:

```text
.\betashape.exe .\Cu67\Cu-67.txt -csv
```

The retained executable output identifies **BetaShape 2.4 (06/2024)**. No new run was performed. Every selected transition explicitly reports no tabulated experimental shape factor. Each reference CSV copies the raw energy token, `dN/dE calc.` token, and adjacent pointwise uncertainty token unchanged. The full raw support is retained, including zero-energy and endpoint entries. No resampling or density renormalization is stored in these CSVs.

For adjacent points `(a,c)` and `(b,d)`, the independent area is `(b-a)(c+d)/2`; the first moment is `(b-a)[a(2c+d)+b(c+2d)]/6`. Exact rational arithmetic on the retained decimal tokens is used before decimal rendering. Conditional analysis divides by the independently integrated area. These source densities may already carry a rounded branch intensity; their area is not substituted for the selected physical yield. Pointwise uncertainty tokens are retained, but their correlations are unavailable, so no uncertainty on the integrated mean is invented.

| Spectrum | Raw points | Piecewise-linear area | Conditional mean (keV) |
| --- | --- | --- | --- |
| [beta_minus_167_3_keV.csv](beta_minus_167_3_keV.csv) | 336 | 0.0103903850091912 | 45.66070723687633 |
| [beta_minus_376_2_keV.csv](beta_minus_376_2_keV.csv) | 378 | 0.5170156739322966 | 114.5457817374058 |
| [beta_minus_467_5_keV.csv](beta_minus_467_5_keV.csv) | 469 | 0.197304310021 | 147.4946561478060 |
| [beta_minus_560_8_keV.csv](beta_minus_560_8_keV.csv) | 562 | 0.275004534643062 | 182.5387152647171 |

Exact raw output paths, endpoint uncertainties, branch-yield uncertainties, and BetaShape header statements are recorded in `reference.json`. Branch spectra are not combined or weighted a second time at runtime.

## Cross-checks and disagreements

Huo Junde, Huang Xiaolong and J. K. Tuli, NDS 106, 159 (2005), cutoff April 1, 2005: 61.83(12) h and Q=561.7(15) keV. The four beta branches have approximately 1.1%, 57%, 22% and 20% in increasing endpoint order, unlike the 2026 LNHB evaluation.

The selected beta yields are the unrounded evaluated Probability (%) column of comments Table 4: 1.039(14), 51.735(628), 19.734(470), 27.492(784). They sum to exactly 100%. The Rounded probability (%) column, PenNuc and BetaShape input give 1.039, 51.7, 19.73, 27.5, summing to 99.969%. This is explicitly explained by LNHB. Selecting the published unrounded values is not renormalizing the rounded values.

The half-life is taken from the evaluated 61.82(5) h in comments section 1.1.1; 2.5758(21) d and 222550(180) s are rounded representations. Gamma energies use the adopted least-squares values (comments Table 7 and LARA), not the intermediate weighted averages of Table 6.

MIRD detailed atomic and conversion inventories have different line granularity and yields from the selected LNHB inventory. They are not added, averaged, or used to close an energy or yield balance.

Retained MIRD/MIRDsoft summary CSV totals are shown below. Their evaluation identity and parent half-life are not established by the CSV itself. Different atomic granularity, low-energy rows, and a different selected evaluation can change these totals. They are not used to fill missing selected LNHB energy laws. DPK files are dose-deposition data, not emission probabilities.

| MIRD type | Rows | Yield per nuclear transformation | Conditional mean (keV) |
| --- | --- | --- | --- |
| X-ray | 25 | 0.775938840051596 | 0.715672209033 |
| Gamma | 6 | 0.72932 | 157.406204245 |
| Beta | 4 | 1.000000 | 135.892689891 |
| Auger electron | 9 | 0.55884612815 | 1.34385568437 |
| Conversion electron | 36 | 0.15280048132576 | 89.9484018124 |

The independent piecewise-linear integral of the retained MIRD beta CSV is 1.0016259899, with conditional mean 136.3752274789856 keV. This is separate from its summary-spectrum mean and from the selected BetaShape branch laws.

## Exclusions and scope

LNHB gives nonzero Auger family totals: L=0.1751 +/- 0.0015 and K=0.0628 +/- 0.0014 electrons per parent decay. It supplies energy ranges rather than a complete conditional energy law in the retained selected inventory. These Auger emissions are therefore excluded from this source definition; they are not claimed to be absent physically. Substituting arbitrary monoenergies, uniform ranges, or MIRD lines would add an unsupported selection.

No source-level 511 keV annihilation pair is included. EC probabilities do not generate particle placeholders. Internal-pair coefficients without a complete selected energy law do not define additional groups. Neutrinos, recoil nuclei, later daughter decays, zero-energy placeholders, and unobserved upper-limit branches are outside this model. Nuclear cascade correlations and daughter-level delays are not represented by these independent prompt marginals.

## Compiled representation and validation limits

Integer Energy storage is in micro-eV. The beta grid uses `ceil(endpoint / 0.5 keV)` bins and the largest even integer-meV width leaving a strictly positive lower edge while preserving the exact selected upper endpoint. Conditional bin masses are integrated offline from the independent piecewise-linear reference, normalized over retained grid support, then passed to the existing ticket constructor. The full continuous reference retains the small lower-edge mass excluded by this finite representation.

| Branch | Bins | Width (micro-eV) | Lower edge (micro-eV) | Excluded conditional mass | Grid mean (keV) | Minimum tickets |
| --- | --- | --- | --- | --- | --- | --- |
| `beta_minus_167_3_keV` | 335 | 499402000 | 330000 | 0.00000610598603332 | 45.66137042447910 | 135 |
| `beta_minus_376_2_keV` | 753 | 499600000 | 1200000 | 0.00000777703187336 | 114.5468073657064 | 31 |
| `beta_minus_467_5_keV` | 935 | 499998000 | 1870000 | 0.00000889736121886 | 147.4960675925888 | 13 |
| `beta_minus_560_8_keV` | 1122 | 499820000 | 1960000 | 0.00000710151248111 | 182.5400869979586 | 16 |

These pre-build exact-integration results satisfy the unchanged 0.005 keV full-reference mean budget inherited from H-3/C-14/P-32/P-33. Exported binary64 weights and their actual finite tickets are checked independently before campaigns.

The default generic campaign uses 32 consecutive half-open windows, requests four compiled half-lives, seed 77777, 256 workers per device, a target of 1000 expected total primaries in the final window, and 128 host population replicates. Rare conversion groups can legitimately have much less than one expected observation. Their stochastic checks may remain `insufficient_samples`; all selected lines still require deterministic energy/yield/ticket checks. No rare group is amplified.

Nuclear-data uncertainty, tabulated-continuous interpolation, finite-grid/ticket representation, and Monte Carlo acceptance are separate issues. The generic deterministic spectrum diagnostic remains `comparison_only`; stochastic tests use the unchanged fixed budgets. CPU/GPU runs with the same seed share the host population experiment and are not independent population evidence. Source validation does not establish transport-dose accuracy or the physical completeness of excluded channels.

The `implementation_audit` uses the current source-comment convention to check the compiled file's BetaShape version. Campaign results and exported-definition checks belong in the accompanying integration report; they do not redefine the reference.

## Immutable provenance

All retained raw files are listed with byte counts and SHA-256 hashes in `reference.json`. The inventory includes LNHB tables/comments/LARA/PenNuc/input, retained BetaShape command/input/output, ENSDF, and MIRD/MIRDsoft files. Raw bytes are not regenerated, edited, renamed, or included as modifications in the integration patch.
