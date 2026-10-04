# Sc-47 selected source reference

## Authority and model

CEA/LNE-LNHB, X. Mougeot, November 2013; tables 29/11/2013 - 5/3/2014; PenNuc 29/11/2013. The retained LNHB evaluation is the selected nuclear authority. ENSDF and MIRD/MIRDsoft are independent cross-checks; no conflicting evaluations are averaged.

100% beta-minus decay to Ti-47. The definition contains independent marginal emissions per parent decay. Their physical yields are not a categorical distribution and need not sum to one. Selected beta branches remain separate, in increasing endpoint order, followed by nuclear gamma rays, compact X rays, and conversion-electron groups in increasing parent-transition energy order.

The 5 groups have total expected yield **1.68606735 primaries per parent decay** within the selected model. This is not a complete decay-energy balance: the exclusions below are physically significant.

## Half-life and decay energy

Select `3.3485 +/- 0.0009 d` from the evaluation. Multiply the evaluated value and standard uncertainty by exactly 86400 s/d; retain the evaluated central value rather than rounded LARA/BetaShape seconds. This gives **289310.4000 s**, with propagated standard uncertainty **77.7600 s**. The exact unit conversion introduces no additional uncertainty.

The parent Q value is **600.8 +/- 1.9 keV**. Individual beta endpoints differ where a daughter excited state is populated. EC Q values are not positron kinetic endpoints.

## Ordered source inventory

| Index | Group | Particle | Law | Yield per parent decay |
| --- | --- | --- | --- | --- |
| 0 | `beta_minus_441_4_keV` | Electron | RegularSpectrum | 0.685 |
| 1 | `beta_minus_600_8_keV` | Electron | RegularSpectrum | 0.315 |
| 2 | `nuclear_gamma` | Gamma | Mono | 0.681 |
| 3 | `atomic_xray` | Gamma | DiscreteLines | 0.000862 |
| 4 | `conversion_159_373_keV` | Electron | DiscreteLines | 0.00420535 |

Every selected line, energy uncertainty, absolute yield, and available yield uncertainty is listed in [reference.json](reference.json). LARA photon intensities in percent are divided by exactly 100. PenNuc EK/EL/EM/EN records are already absolute per-parent yields; they are not multiplied by gamma or EC probabilities. Group yield is the sum of the selected absolute line yields. Group uncertainty is not inferred by treating correlated lines as independent.

The compact LARA X-ray entries retain their supplied representative energies and absolute intensities, consistent with existing GGEMS discrete-line references. More detailed relative atomic-line tables are not added again. PenNuc grouped M/N energies are retained as supplied; small differences between conversion-line plus binding energies and photon energies are not silently corrected.

Conversion electrons are grouped by nuclear transition. This preserves every positive retained shell line, including lines too small to receive a ticket if all conversion transitions were collapsed into one conditional distribution. It does not amplify any physical yield or introduce emission correlations. The existing finite-ticket allocator and Source semantics are unchanged.

## BetaShape selection and continuous references

The retained command is reproduced verbatim:

```text
.\betashape.exe .\Sc47\Sc-47.txt -csv
```

The retained executable output identifies **BetaShape 2.4 (06/2024)**. No new run was performed. Every selected transition explicitly reports no tabulated experimental shape factor. Each reference CSV copies the raw energy token, `dN/dE calc.` token, and adjacent pointwise uncertainty token unchanged. The full raw support is retained, including zero-energy and endpoint entries. No resampling or density renormalization is stored in these CSVs.

For adjacent points `(a,c)` and `(b,d)`, the independent area is `(b-a)(c+d)/2`; the first moment is `(b-a)[a(2c+d)+b(c+2d)]/6`. Exact rational arithmetic on the retained decimal tokens is used before decimal rendering. Conditional analysis divides by the independently integrated area. These source densities may already carry a rounded branch intensity; their area is not substituted for the selected physical yield. Pointwise uncertainty tokens are retained, but their correlations are unavailable, so no uncertainty on the integrated mean is invented.

| Spectrum | Raw points | Piecewise-linear area | Conditional mean (keV) |
| --- | --- | --- | --- |
| [beta_minus_441_4_keV.csv](beta_minus_441_4_keV.csv) | 443 | 0.685029459589013 | 142.0082573689330 |
| [beta_minus_600_8_keV.csv](beta_minus_600_8_keV.csv) | 302 | 0.315011881386908 | 203.3405879709307 |

Exact raw output paths, endpoint uncertainties, branch-yield uncertainties, and BetaShape header statements are recorded in `reference.json`. Branch spectra are not combined or weighted a second time at runtime.

## Cross-checks and disagreements

S. Ota and E. A. McCutchan, NDS 203, 1 (2025), cutoff April 1, 2025: 3.3492(6) d, Q=600.8(19) keV, beta intensities 68.6(4)% and 31.4(4)%, and gamma 159.381(15) keV at 68.3(4)%. LNHB gives 3.3485(9) d, 68.5(5)% and 31.5(5)%, and 159.373(12) keV at 68.1(5)%.

LNHB tables retain older LOGFT beta means 142.8(7) and 204.2(8) keV. BetaShape 2.4 gives 142.0(7) and 203.3(8) keV, consistent with the more recent ENSDF means. The selected continuous laws are the retained BetaShape 2.4 calculated spectra.

MIRD detailed atomic and conversion inventories have different line granularity and yields from the selected LNHB inventory. They are not added, averaged, or used to close an energy or yield balance.

Retained MIRD/MIRDsoft summary CSV totals are shown below. Their evaluation identity and parent half-life are not established by the CSV itself. Different atomic granularity, low-energy rows, and a different selected evaluation can change these totals. They are not used to fill missing selected LNHB energy laws. DPK files are dose-deposition data, not emission probabilities.

| MIRD type | Rows | Yield per nuclear transformation | Conditional mean (keV) |
| --- | --- | --- | --- |
| X-ray | 25 | 0.02297286604237599 | 0.126141784017 |
| Gamma | 1 | 0.683 | 159.381000 |
| Beta | 2 | 1.000 | 161.930384 |
| Auger electron | 7 | 0.0192871422 | 0.578178725927 |
| Conversion electron | 4 | 0.00307350461 | 154.809574810 |

The independent piecewise-linear integral of the retained MIRD beta CSV is 1.0020672234569, with conditional mean 162.5602537337954 keV. This is separate from its summary-spectrum mean and from the selected BetaShape branch laws.

## Exclusions and scope

LNHB gives nonzero Auger family totals: L=0.000349 +/- 0.000008 and K=0.00295 +/- 0.00007 electrons per parent decay. It supplies energy ranges rather than a complete conditional energy law in the retained selected inventory. These Auger emissions are therefore excluded from this source definition; they are not claimed to be absent physically. Substituting arbitrary monoenergies, uniform ranges, or MIRD lines would add an unsupported selection.

No source-level 511 keV annihilation pair is included. EC probabilities do not generate particle placeholders. Internal-pair coefficients without a complete selected energy law do not define additional groups. Neutrinos, recoil nuclei, later daughter decays, zero-energy placeholders, and unobserved upper-limit branches are outside this model. Nuclear cascade correlations and daughter-level delays are not represented by these independent prompt marginals.

## Compiled representation and validation limits

Integer Energy storage is in micro-eV. The beta grid uses `ceil(endpoint / 0.5 keV)` bins and the largest even integer-meV width leaving a strictly positive lower edge while preserving the exact selected upper endpoint. Conditional bin masses are integrated offline from the independent piecewise-linear reference, normalized over retained grid support, then passed to the existing ticket constructor. The full continuous reference retains the small lower-edge mass excluded by this finite representation.

| Branch | Bins | Width (micro-eV) | Lower edge (micro-eV) | Excluded conditional mass | Grid mean (keV) | Minimum tickets |
| --- | --- | --- | --- | --- | --- | --- |
| `beta_minus_441_4_keV` | 883 | 499886000 | 662000 | 0.00000336045889104 | 142.0088402794814 | 15 |
| `beta_minus_600_8_keV` | 1202 | 499832000 | 1936000 | 0.00000554121394752 | 203.3417743172006 | 15 |

These pre-build exact-integration results satisfy the unchanged 0.005 keV full-reference mean budget inherited from H-3/C-14/P-32/P-33. Exported binary64 weights and their actual finite tickets are checked independently before campaigns.

The default generic campaign uses 32 consecutive half-open windows, requests four compiled half-lives, seed 77777, 256 workers per device, a target of 1000 expected total primaries in the final window, and 128 host population replicates. Rare conversion groups can legitimately have much less than one expected observation. Their stochastic checks may remain `insufficient_samples`; all selected lines still require deterministic energy/yield/ticket checks. No rare group is amplified.

Nuclear-data uncertainty, tabulated-continuous interpolation, finite-grid/ticket representation, and Monte Carlo acceptance are separate issues. The generic deterministic spectrum diagnostic remains `comparison_only`; stochastic tests use the unchanged fixed budgets. CPU/GPU runs with the same seed share the host population experiment and are not independent population evidence. Source validation does not establish transport-dose accuracy or the physical completeness of excluded channels.

The `implementation_audit` uses the current source-comment convention to check the compiled file's BetaShape version. Campaign results and exported-definition checks belong in the accompanying integration report; they do not redefine the reference.

## Immutable provenance

All retained raw files are listed with byte counts and SHA-256 hashes in `reference.json`. The inventory includes LNHB tables/comments/LARA/PenNuc/input, retained BetaShape command/input/output, ENSDF, and MIRD/MIRDsoft files. Raw bytes are not regenerated, edited, renamed, or included as modifications in the integration patch.
