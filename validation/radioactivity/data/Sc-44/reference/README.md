# Sc-44 selected source reference

## Authority and model

LNHB/DDEP, E. Browne (LBNL), November 2001; tables 19/10/01 - 23/4/2004; PenNuc 19/10/2001. The retained LNHB evaluation is the selected nuclear authority. ENSDF and MIRD/MIRDsoft are independent cross-checks; no conflicting evaluations are averaged.

Electron capture and beta-plus decay to Ca-44; selected positron yield 0.9427 per parent decay. The definition contains independent marginal emissions per parent decay. Their physical yields are not a categorical distribution and need not sum to one. Selected beta branches remain separate, in increasing endpoint order, followed by nuclear gamma rays, compact X rays, and conversion-electron groups in increasing parent-transition energy order.

The 5 groups have total expected yield **1.96052797837 primaries per parent decay** within the selected model. This is not a complete decay-energy balance: the exclusions below are physically significant.

## Half-life and decay energy

Select `3.97 +/- 0.04 h` from the evaluation. Multiply the evaluated value and standard uncertainty by exactly 3600 s/h; retain the evaluated central value rather than rounded LARA/BetaShape seconds. This gives **14292.00 s**, with propagated standard uncertainty **144.00 s**. The exact unit conversion introduces no additional uncertainty.

The parent Q value is **3653.3 +/- 1.9 keV**. Individual beta endpoints differ where a daughter excited state is populated. EC Q values are not positron kinetic endpoints.

## Ordered source inventory

| Index | Group | Particle | Law | Yield per parent decay |
| --- | --- | --- | --- | --- |
| 0 | `beta_plus_1474_3_keV` | Positron | RegularSpectrum | 0.9427 |
| 1 | `nuclear_gamma` | Gamma | DiscreteLines | 1.009003 |
| 2 | `atomic_xray` | Gamma | DiscreteLines | 0.00876 |
| 3 | `conversion_1157_02_keV` | Electron | DiscreteLines | 0.000064689 |
| 4 | `conversion_1499_46_keV` | Electron | DiscreteLines | 2.8937E-7 |

Every selected line, energy uncertainty, absolute yield, and available yield uncertainty is listed in [reference.json](reference.json). LARA photon intensities in percent are divided by exactly 100. PenNuc EK/EL/EM/EN records are already absolute per-parent yields; they are not multiplied by gamma or EC probabilities. Group yield is the sum of the selected absolute line yields. Group uncertainty is not inferred by treating correlated lines as independent.

The compact LARA X-ray entries retain their supplied representative energies and absolute intensities, consistent with existing GGEMS discrete-line references. More detailed relative atomic-line tables are not added again. PenNuc grouped M/N energies are retained as supplied; small differences between conversion-line plus binding energies and photon energies are not silently corrected.

Conversion electrons are grouped by nuclear transition. This preserves every positive retained shell line, including lines too small to receive a ticket if all conversion transitions were collapsed into one conditional distribution. It does not amplify any physical yield or introduce emission correlations. The existing finite-ticket allocator and Source semantics are unchanged.

## BetaShape selection and continuous references

The retained command is reproduced verbatim:

```text
.\betashape.exe .\Sc44\Sc-44.txt fixint=1 -csv
```

The retained executable output identifies **BetaShape 2.4 (06/2024)**. No new run was performed. `fixint=1` intentionally preserves the evaluated EC/beta-plus split. Every selected transition explicitly reports no tabulated experimental shape factor. Each reference CSV copies the raw energy token, `dN/dE calc.` token, and adjacent pointwise uncertainty token unchanged. The full raw support is retained, including zero-energy and endpoint entries. No resampling or density renormalization is stored in these CSVs.

For adjacent points `(a,c)` and `(b,d)`, the independent area is `(b-a)(c+d)/2`; the first moment is `(b-a)[a(2c+d)+b(c+2d)]/6`. Exact rational arithmetic on the retained decimal tokens is used before decimal rendering. Conditional analysis divides by the independently integrated area. These source densities may already carry a rounded branch intensity; their area is not substituted for the selected physical yield. Pointwise uncertainty tokens are retained, but their correlations are unavailable, so no uncertainty on the integrated mean is invented.

| Spectrum | Raw points | Piecewise-linear area | Conditional mean (keV) |
| --- | --- | --- | --- |
| [beta_plus_1474_3_keV.csv](beta_plus_1474_3_keV.csv) | 370 | 0.942700036661795 | 630.4471823301455 |

Exact raw output paths, endpoint uncertainties, branch-yield uncertainties, and BetaShape header statements are recorded in `reference.json`. Branch spectra are not combined or weighted a second time at runtime.

## Cross-checks and disagreements

Jun Chen and Balraj Singh, NDS 190, 1 (2023), cutoff June 20, 2023: 4.0420(25) h, Q=3652.7(18) keV, dominant positron yield 94.278(11)% and EC 4.696(11)%. LNHB selects 3.97(4) h, Q=3653.3(19) keV and 94.27(5)% positrons. The newer ENSDF evaluation is a cross-check, not a replacement.

The retained fixint=1 run preserves the evaluated EC/beta-plus split. EC is about 5.73%; the separately rounded listed EC branch intensities sum to 5.7244%. No branch probability is forced to close a rounded sum.

LNHB reports beta mean 632.0(9) keV; retained BetaShape 2.4 reports 630.4(9) keV (the independent tabulated-density integral is listed below). PenNuc writes endpoint 1474.261 keV while the selected table/BetaShape support ends at 1474.3 keV. Neither law is stretched or shifted.

MIRD detailed atomic and conversion inventories have different line granularity and yields from the selected LNHB inventory. They are not added, averaged, or used to close an energy or yield balance.

Retained MIRD/MIRDsoft summary CSV totals are shown below. Their evaluation identity and parent half-life are not established by the CSV itself. Different atomic granularity, low-energy rows, and a different selected evaluation can change these totals. They are not used to fill missing selected LNHB energy laws. DPK files are dose-deposition data, not emission probabilities.

| MIRD type | Rows | Yield per nuclear transformation | Conditional mean (keV) |
| --- | --- | --- | --- |
| X-ray | 17 | 0.2150814506980577 | 0.174204285524 |
| Annihilation photon | 1 | 1.88708 | 511.000 |
| Gamma | 6 | 1.009254735 | 1161.83333694 |
| Positron | 1 | 0.943542 | 631.478000 |
| Auger electron | 6 | 0.151111685 | 1.11103418406 |
| Conversion electron | 24 | 0.0000650106497844079 | 1155.17385028 |

The independent piecewise-linear integral of the retained MIRD beta CSV is 0.9430898582512385, with conditional mean 632.9304024883901 keV. This is separate from its summary-spectrum mean and from the selected BetaShape branch laws.

## Exclusions and scope

LNHB gives nonzero Auger family totals: L=0.0871 +/- 0.0005 and K=0.0421 +/- 0.0003 electrons per parent decay. It supplies energy ranges rather than a complete conditional energy law in the retained selected inventory. These Auger emissions are therefore excluded from this source definition; they are not claimed to be absent physically. Substituting arbitrary monoenergies, uniform ranges, or MIRD lines would add an unsupported selection.

No source-level 511 keV annihilation pair is included. EC probabilities do not generate particle placeholders. Internal-pair coefficients without a complete selected energy law do not define additional groups. Neutrinos, recoil nuclei, later daughter decays, zero-energy placeholders, and unobserved upper-limit branches are outside this model. Nuclear cascade correlations and daughter-level delays are not represented by these independent prompt marginals.

## Compiled representation and validation limits

Integer Energy storage is in micro-eV. The beta grid uses `ceil(endpoint / 0.5 keV)` bins and the largest even integer-meV width leaving a strictly positive lower edge while preserving the exact selected upper endpoint. Conditional bin masses are integrated offline from the independent piecewise-linear reference, normalized over retained grid support, then passed to the existing ticket constructor. The full continuous reference retains the small lower-edge mass excluded by this finite representation.

| Branch | Bins | Width (micro-eV) | Lower edge (micro-eV) | Excluded conditional mass | Grid mean (keV) | Minimum tickets |
| --- | --- | --- | --- | --- | --- | --- |
| `beta_plus_1474_3_keV` | 2949 | 499932000 | 532000 | 1.09459009852e-13 | 630.4471823332674 | 6 |

These pre-build exact-integration results satisfy the unchanged 0.005 keV full-reference mean budget inherited from H-3/C-14/P-32/P-33. Exported binary64 weights and their actual finite tickets are checked independently before campaigns.

The default generic campaign uses 32 consecutive half-open windows, requests four compiled half-lives, seed 77777, 256 workers per device, a target of 1000 expected total primaries in the final window, and 128 host population replicates. Rare conversion groups can legitimately have much less than one expected observation. Their stochastic checks may remain `insufficient_samples`; all selected lines still require deterministic energy/yield/ticket checks. No rare group is amplified.

Nuclear-data uncertainty, tabulated-continuous interpolation, finite-grid/ticket representation, and Monte Carlo acceptance are separate issues. The generic deterministic spectrum diagnostic remains `comparison_only`; stochastic tests use the unchanged fixed budgets. CPU/GPU runs with the same seed share the host population experiment and are not independent population evidence. Source validation does not establish transport-dose accuracy or the physical completeness of excluded channels.

The `implementation_audit` uses the current source-comment convention to check the compiled file's BetaShape version. Campaign results and exported-definition checks belong in the accompanying integration report; they do not redefine the reference.

## Immutable provenance

All retained raw files are listed with byte counts and SHA-256 hashes in `reference.json`. The inventory includes LNHB tables/comments/LARA/PenNuc/input, retained BetaShape command/input/output, ENSDF, and MIRD/MIRDsoft files. Raw bytes are not regenerated, edited, renamed, or included as modifications in the integration patch.
