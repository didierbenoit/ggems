# Ga-67 selected source reference

## Authority and model

LNHB/DDEP, X. Mougeot and V. P. Chechev (LNHB/KRI), revised March 2011; tables 20/09/2000 - 9/9/2011; PenNuc 14/04/2011. The retained LNHB evaluation is the selected nuclear authority. ENSDF and MIRD/MIRDsoft are independent cross-checks; no conflicting evaluations are averaged.

100% electron capture to Zn-67. The definition contains independent marginal emissions per parent decay. Their physical yields are not a categorical distribution and need not sum to one. Selected beta branches remain separate, in increasing endpoint order, followed by nuclear gamma rays, compact X rays, and conversion-electron groups in increasing parent-transition energy order.

The 12 groups have total expected yield **1.7807055311453 primaries per parent decay** within the selected model. This is not a complete decay-energy balance: the exclusions below are physically significant.

## Half-life and decay energy

Select `3.2613 +/- 0.0005 d` from the evaluation. Multiply the evaluated value and standard uncertainty by exactly 86400 s/d; retain the evaluated central value rather than rounded LARA/BetaShape seconds. This gives **281776.3200 s**, with propagated standard uncertainty **43.2000 s**. The exact unit conversion introduces no additional uncertainty.

The parent Q value is **1000.8 +/- 1.2 keV**. Individual beta endpoints differ where a daughter excited state is populated. EC Q values are not positron kinetic endpoints.

## Ordered source inventory

| Index | Group | Particle | Law | Yield per parent decay |
| --- | --- | --- | --- | --- |
| 0 | `nuclear_gamma` | Gamma | DiscreteLines | 0.859899 |
| 1 | `atomic_xray` | Gamma | DiscreteLines | 0.5883 |
| 2 | `conversion_91_263_keV` | Electron | DiscreteLines | 0.00280875 |
| 3 | `conversion_93_307_keV` | Electron | DiscreteLines | 0.3252078 |
| 4 | `conversion_184_577_keV` | Electron | DiscreteLines | 0.0035407 |
| 5 | `conversion_208_939_keV` | Electron | DiscreteLines | 0.0002135254 |
| 6 | `conversion_300_232_keV` | Electron | DiscreteLines | 0.000645600 |
| 7 | `conversion_393_528_keV` | Electron | DiscreteLines | 0.0000885200 |
| 8 | `conversion_494_143_keV` | Electron | DiscreteLines | 7.65457E-7 |
| 9 | `conversion_703_11_keV` | Electron | DiscreteLines | 5.92053E-8 |
| 10 | `conversion_794_4_keV` | Electron | DiscreteLines | 2.82516E-7 |
| 11 | `conversion_887_676_keV` | Electron | DiscreteLines | 5.28567E-7 |

Every selected line, energy uncertainty, absolute yield, and available yield uncertainty is listed in [reference.json](reference.json). LARA photon intensities in percent are divided by exactly 100. PenNuc EK/EL/EM/EN records are already absolute per-parent yields; they are not multiplied by gamma or EC probabilities. Group yield is the sum of the selected absolute line yields. Group uncertainty is not inferred by treating correlated lines as independent.

The compact LARA X-ray entries retain their supplied representative energies and absolute intensities, consistent with existing GGEMS discrete-line references. More detailed relative atomic-line tables are not added again. PenNuc grouped M/N energies are retained as supplied; small differences between conversion-line plus binding energies and photon energies are not silently corrected.

Conversion electrons are grouped by nuclear transition. This preserves every positive retained shell line, including lines too small to receive a ticket if all conversion transitions were collapsed into one conditional distribution. It does not amplify any physical yield or introduce emission correlations. The existing finite-ticket allocator and Source semantics are unchanged.

## BetaShape selection and continuous references

The retained command is reproduced verbatim:

```text
.\betashape.exe .\Ga67\Ga-67.txt -csv
```

The retained executable output identifies **BetaShape 2.4 (06/2024)**. No new run was performed. This is an EC nuclide; there is no selected continuous beta spectrum. Capture outputs document shell probabilities and transition energies, not additional source-particle groups.

## Cross-checks and disagreements

Huo Junde, Huang Xiaolong and J. K. Tuli, NDS 106, 159 (2005), cutoff April 1, 2005: 3.2617(5) d, Q=1000.8(12) keV. Excited-state EC intensities 0.281(4)%, 23.6(3)%, 22.71(9)% and 52.5(11)% differ from LNHB 0.280%, 23.60%, 22.3% and 50.5%.

The PDF conversion summary and detailed PenNuc export differ: the main K line is 83.651 keV at 0.284 per decay in the PDF, versus 83.648 keV at 0.285 in PenNuc; the corresponding L total is 0.0355 versus 0.03511. The selected conversion law is consistently the complete retained PenNuc inventory, not an average or mixture.

For the 300 keV transition, the PDF K line is 290.558 keV at 0.00060 per decay; PenNuc gives 290.574 keV at 0.000578. This retained-export disagreement is preserved.

MIRD detailed atomic and conversion inventories have different line granularity and yields from the selected LNHB inventory. They are not added, averaged, or used to close an energy or yield balance.

Retained MIRD/MIRDsoft summary CSV totals are shown below. Their evaluation identity and parent half-life are not established by the CSV itself. Different atomic granularity, low-energy rows, and a different selected evaluation can change these totals. They are not used to fill missing selected LNHB energy laws. DPK files are dose-deposition data, not emission probabilities.

| MIRD type | Rows | Yield per nuclear transformation | Conditional mean (keV) |
| --- | --- | --- | --- |
| X-ray | 25 | 6.874223572854018 | 0.714855505959 |
| Gamma | 10 | 0.877227 | 176.269820980 |
| Auger electron | 9 | 4.9606602561 | 1.33858242522 |
| Conversion electron | 60 | 0.3440241572820146 | 86.3221632485 |

## Exclusions and scope

LNHB gives nonzero Auger family totals: L=1.675 +/- 0.021 and K=0.604 +/- 0.021 electrons per parent decay. It supplies energy ranges rather than a complete conditional energy law in the retained selected inventory. These Auger emissions are therefore excluded from this source definition; they are not claimed to be absent physically. Substituting arbitrary monoenergies, uniform ranges, or MIRD lines would add an unsupported selection.

No source-level 511 keV annihilation pair is included. EC probabilities do not generate particle placeholders. Internal-pair coefficients without a complete selected energy law do not define additional groups. Neutrinos, recoil nuclei, later daughter decays, zero-energy placeholders, and unobserved upper-limit branches are outside this model. Nuclear cascade correlations and daughter-level delays are not represented by these independent prompt marginals.

## Compiled representation and validation limits

Integer Energy storage is in micro-eV. The beta grid uses `ceil(endpoint / 0.5 keV)` bins and the largest even integer-meV width leaving a strictly positive lower edge while preserving the exact selected upper endpoint. Conditional bin masses are integrated offline from the independent piecewise-linear reference, normalized over retained grid support, then passed to the existing ticket constructor. The full continuous reference retains the small lower-edge mass excluded by this finite representation.

The default generic campaign uses 32 consecutive half-open windows, requests four compiled half-lives, seed 77777, 256 workers per device, a target of 1000 expected total primaries in the final window, and 128 host population replicates. Rare conversion groups can legitimately have much less than one expected observation. Their stochastic checks may remain `insufficient_samples`; all selected lines still require deterministic energy/yield/ticket checks. No rare group is amplified.

Nuclear-data uncertainty, tabulated-continuous interpolation, finite-grid/ticket representation, and Monte Carlo acceptance are separate issues. The generic deterministic spectrum diagnostic remains `comparison_only`; stochastic tests use the unchanged fixed budgets. CPU/GPU runs with the same seed share the host population experiment and are not independent population evidence. Source validation does not establish transport-dose accuracy or the physical completeness of excluded channels.

The `implementation_audit` uses the current source-comment convention to check the compiled file's BetaShape version. Campaign results and exported-definition checks belong in the accompanying integration report; they do not redefine the reference.

## Immutable provenance

All retained raw files are listed with byte counts and SHA-256 hashes in `reference.json`. The inventory includes LNHB tables/comments/LARA/PenNuc/input, retained BetaShape command/input/output, ENSDF, and MIRD/MIRDsoft files. Raw bytes are not regenerated, edited, renamed, or included as modifications in the integration patch.
