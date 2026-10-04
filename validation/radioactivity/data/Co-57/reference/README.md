# Co-57 selected source reference

## Authority and model

LNHB/DDEP, V. P. Chechev and N. K. Kuzmenko (KRI), updated August 2014; tables 07/07/2014 - 1/3/2017; PenNuc 09/09/2014. The retained LNHB evaluation is the selected nuclear authority. ENSDF and MIRD/MIRDsoft are independent cross-checks; no conflicting evaluations are averaged.

100% electron capture to Fe-57. The definition contains independent marginal emissions per parent decay. Their physical yields are not a categorical distribution and need not sum to one. Selected beta branches remain separate, in increasing endpoint order, followed by nuclear gamma rays, compact X rays, and conversion-electron groups in increasing parent-transition energy order.

The 12 groups have total expected yield **2.4649141719071 primaries per parent decay** within the selected model. This is not a complete decay-energy balance: the exclusions below are physically significant.

## Half-life and decay energy

Select `271.81 +/- 0.04 d` from the evaluation. Multiply the evaluated value and standard uncertainty by exactly 86400 s/d; retain the evaluated central value rather than rounded LARA/BetaShape seconds. This gives **23484384.00 s**, with propagated standard uncertainty **3456.00 s**. The exact unit conversion introduces no additional uncertainty.

The parent Q value is **836.2 +/- 0.5 keV**. Individual beta endpoints differ where a daughter excited state is populated. EC Q values are not positron kinetic endpoints.

## Ordered source inventory

| Index | Group | Particle | Law | Yield per parent decay |
| --- | --- | --- | --- | --- |
| 0 | `nuclear_gamma` | Gamma | DiscreteLines | 1.055677 |
| 1 | `atomic_xray` | Gamma | DiscreteLines | 0.5885 |
| 2 | `conversion_14_41295_keV` | Electron | DiscreteLines | 0.784623 |
| 3 | `conversion_122_06065_keV` | Electron | DiscreteLines | 0.02017715 |
| 4 | `conversion_136_47356_keV` | Electron | DiscreteLines | 0.01593619 |
| 5 | `conversion_230_27_keV` | Electron | DiscreteLines | 1.6648E-8 |
| 6 | `conversion_339_67_keV` | Electron | DiscreteLines | 6.33053E-8 |
| 7 | `conversion_352_34_keV` | Electron | DiscreteLines | 4.77934E-8 |
| 8 | `conversion_366_74_keV` | Electron | DiscreteLines | 2.32566E-8 |
| 9 | `conversion_569_94_keV` | Electron | DiscreteLines | 7.6554E-8 |
| 10 | `conversion_692_01_keV` | Electron | DiscreteLines | 5.80152E-7 |
| 11 | `conversion_706_415_keV` | Electron | DiscreteLines | 2.41978E-8 |

Every selected line, energy uncertainty, absolute yield, and available yield uncertainty is listed in [reference.json](reference.json). LARA photon intensities in percent are divided by exactly 100. PenNuc EK/EL/EM/EN records are already absolute per-parent yields; they are not multiplied by gamma or EC probabilities. Group yield is the sum of the selected absolute line yields. Group uncertainty is not inferred by treating correlated lines as independent.

The compact LARA X-ray entries retain their supplied representative energies and absolute intensities, consistent with existing GGEMS discrete-line references. More detailed relative atomic-line tables are not added again. PenNuc grouped M/N energies are retained as supplied; small differences between conversion-line plus binding energies and photon energies are not silently corrected.

Conversion electrons are grouped by nuclear transition. This preserves every positive retained shell line, including lines too small to receive a ticket if all conversion transitions were collapsed into one conditional distribution. It does not amplify any physical yield or introduce emission correlations. The existing finite-ticket allocator and Source semantics are unchanged.

## BetaShape selection and continuous references

The retained command is reproduced verbatim:

```text
.\betashape.exe .\Co57\Co-57.txt -csv
```

The retained executable output identifies **BetaShape 2.4 (06/2024)**. No new run was performed. This is an EC nuclide; there is no selected continuous beta spectrum. Capture outputs document shell probabilities and transition energies, not additional source-particle groups.

## Cross-checks and disagreements

M. R. Bhat, NDS 85, 415 (1998), cutoff September 24, 1998: 271.74(6) d and Q=836.0(4) keV, versus LNHB 271.81(4) d and 836.2(5) keV. The 706.76 keV level feeding is 0.174(10)% versus LNHB 0.183(14)%.

The comments and tables differ in the dominant EC feeding (99.82(20)% versus 99.80(23)%) and the limit on direct 14.4 keV feeding (<0.003% versus <0.0003%). These are capture-scheme cross-checks, not additional source groups.

The comments quote total K X rays 57.1(9)% and K-alpha 50.0(6)%; the selected direct LARA inventory sums to 57.55% and 50.62%, respectively. No rescaling is applied.

MIRD detailed atomic and conversion inventories have different line granularity and yields from the selected LNHB inventory. They are not added, averaged, or used to close an energy or yield balance.

Retained MIRD/MIRDsoft summary CSV totals are shown below. Their evaluation identity and parent half-life are not established by the CSV itself. Different atomic granularity, low-energy rows, and a different selected evaluation can change these totals. They are not used to fill missing selected LNHB energy laws. DPK files are dose-deposition data, not emission probabilities.

| MIRD type | Rows | Yield per nuclear transformation | Conditional mean (keV) |
| --- | --- | --- | --- |
| X-ray | 25 | 11.02400679715074 | 0.332398800266 |
| Gamma | 10 | 1.056181 | 115.099035667 |
| Auger electron | 7 | 8.6413272 | 0.929289877920 |
| Conversion electron | 40 | 0.8204053254796594 | 12.9276438149 |

## Exclusions and scope

LNHB gives nonzero Auger family totals: L=2.506 +/- 0.016 and K=1.046 +/- 0.011 electrons per parent decay. It supplies energy ranges rather than a complete conditional energy law in the retained selected inventory. These Auger emissions are therefore excluded from this source definition; they are not claimed to be absent physically. Substituting arbitrary monoenergies, uniform ranges, or MIRD lines would add an unsupported selection.

No source-level 511 keV annihilation pair is included. EC probabilities do not generate particle placeholders. Internal-pair coefficients without a complete selected energy law do not define additional groups. Neutrinos, recoil nuclei, later daughter decays, zero-energy placeholders, and unobserved upper-limit branches are outside this model. Nuclear cascade correlations and daughter-level delays are not represented by these independent prompt marginals.

## Compiled representation and validation limits

Integer Energy storage is in micro-eV. The beta grid uses `ceil(endpoint / 0.5 keV)` bins and the largest even integer-meV width leaving a strictly positive lower edge while preserving the exact selected upper endpoint. Conditional bin masses are integrated offline from the independent piecewise-linear reference, normalized over retained grid support, then passed to the existing ticket constructor. The full continuous reference retains the small lower-edge mass excluded by this finite representation.

The default generic campaign uses 32 consecutive half-open windows, requests four compiled half-lives, seed 77777, 256 workers per device, a target of 1000 expected total primaries in the final window, and 128 host population replicates. For Co-57, four half-lives exceed the unsigned picosecond time range; the existing runner automatically caps duration. This is a protocol limitation, not a change to the half-life or source model. Rare conversion groups can legitimately have much less than one expected observation. Their stochastic checks may remain `insufficient_samples`; all selected lines still require deterministic energy/yield/ticket checks. No rare group is amplified.

Nuclear-data uncertainty, tabulated-continuous interpolation, finite-grid/ticket representation, and Monte Carlo acceptance are separate issues. The generic deterministic spectrum diagnostic remains `comparison_only`; stochastic tests use the unchanged fixed budgets. CPU/GPU runs with the same seed share the host population experiment and are not independent population evidence. Source validation does not establish transport-dose accuracy or the physical completeness of excluded channels.

The `implementation_audit` uses the current source-comment convention to check the compiled file's BetaShape version. Campaign results and exported-definition checks belong in the accompanying integration report; they do not redefine the reference.

## Immutable provenance

All retained raw files are listed with byte counts and SHA-256 hashes in `reference.json`. The inventory includes LNHB tables/comments/LARA/PenNuc/input, retained BetaShape command/input/output, ENSDF, and MIRD/MIRDsoft files. Raw bytes are not regenerated, edited, renamed, or included as modifications in the integration patch.
