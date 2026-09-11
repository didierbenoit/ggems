# Am-241 selected reference

This package maps recovered local evidence to the six current GGEMS Am-241
Source emission groups. It preserves the current numerical choices, including
an unresolved evaluated upper limit. It is not a new nuclear evaluation.

All scientific source paths below are relative to [../raw](../raw/).
[reference.json](reference.json) contains every selected line, its physical
yield, source location, available uncertainty, and group mapping. No yields
are normalized. BetaShape is not applicable to this alpha-decay reference.

## Selected sources

- `lnhb/Am-241.lara.txt`: KRI 2009, V. P. Chechev and N. K. Kuzmenko;
  selected numerical half-life, alpha, direct nuclear gamma, and compact Np
  X-ray emissions.
- `lnhb/Am-241.PenNuc.txt`: `EvaluationDate 29/09/2009`; selected positive
  shell-resolved conversion-electron yields and energies.
- `lnhb/Am-241_com.pdf`: evaluation updated September 2009, with the same
  literature cutoff. `lnhb/Am-241_tables.pdf` carries the printed footer
  `10/10/2000 - 7/7/2010`. These describe the evaluation, atomic methodology,
  and qualifiers that the direct numerical exports do not always preserve.
- `mird/Am-241 Summary Spectrum.csv`: only the 15 rows labeled `Auger electron`.
  This is the sole MIRD contribution to the current definition; the LNHB
  compact Auger tables give ranges rather than these detailed lines. The
  recovered CSV does not establish a release date, retrieval date, or line
  uncertainties. No such provenance is invented here.
- `ensdf/a_decay.pdf`: independent M. S. Basunia evaluation, NDS 107, 3323
  (2006), cutoff March 15, 2006. Its parent half-life is also 432.6(6) years;
  its detailed emissions are not merged into the selected LNHB data.

The half-life used for comparison is the explicit LARA seconds value,
**13.652E9 s**, with reported uncertainty **0.019E9 s**. The package separately
preserves **432.6(6) a**. Both are published representations; no new year-to-second
conversion replaces the rounded seconds value already used by GGEMS.

## Ordered mapping

All six distributions are `DiscreteLines`. Within each group, energies are in
increasing order. `weight` is the absolute line yield per parent decay; the
group yield is its exact decimal sum. Dividing by the group sum defines only
the conditional energy law, not a normalization of physical emissions.

| Index | Identity | Particle | Lines | Yield per parent decay |
|---|---|---|---:|---:|
| 0 | Alpha | Alpha | 23 | 1.00022736 |
| 1 | Nuclear gamma | Gamma | 179 | 0.3851753802 |
| 2 | Compact Np X-ray | Gamma | 9 | 0.37661828 |
| 3 | Detailed Auger electron | Electron | 15 | 10.1459821737038 |
| 4 | Conversion electron, main | Electron | 268 | 0.910731347024 |
| 5 | Conversion electron, weak | Electron | 248 | 0.000000040000082 |

The 516 positive PenNuc conversion lines are partitioned at absolute yield
`1e-9` per parent: main includes equality, weak is below the threshold. This
reproduces the existing numerical split that keeps every line reachable in
the finite-ticket representation. It introduces no physical branch and drops
no positive line. The four zero central conversion records are retained as
excluded evidence in JSON, with their original reported uncertainties.

LARA intensities in percent are divided by 100. PenNuc and MIRD yields are
already per parent. MIRD energies in MeV follow the existing nearest-meV
embedding, then use canonical micro-eV integers. Only the first Auger line
has a nonzero energy residual: raw `0.0000789857 MeV` maps to `0.078986 keV`,
or `78986000 micro-eV`, a shift of **+300 micro-eV**. Its raw energy is retained
alongside the mapped energy. Every other selected energy converts exactly.

Recoil nuclei, subsequent daughter-chain decays, spontaneous fission, and
MIRD non-Auger rows are excluded. Immediate Np de-excitation and atomic
relaxation associated with the Am-241 parent are included in the stated groups.

## Source discrepancies and uncertainty

For the **5469.47(12) keV alpha line**, LARA line 222 gives numerical **0.04%**,
whereas the companion evaluation gives **<0.04%** (`Am-241_tables.pdf`, section
4, page 7; `Am-241_com.pdf`, section 2.1, Table 2). The mapped weight is the
current GGEMS choice, **0.0004 per parent**. This is agreement with the numerical
export, not proof of agreement with a strict upper limit. The conflict remains
unresolved. In particular, the alpha group sum is not forced to one.

For the compact Np L-gamma X-rays, LARA reports intensity **4.83 +/- 0.03%**;
the comments, section 6.1, Table 4, print **4.83(3)**. The complementary
LNHB ENSDF-format file `lnhb/Am-241.txt`, line 36, instead encodes **4.83(30)**,
or uncertainty **0.30%**. Both are recorded; the selected LARA uncertainty is
not replaced. The central yield agrees and this difference does not affect
the current central-value campaign.

Available uncertainties and missing fields are preserved; PenNuc zero
uncertainty tokens are not interpreted as proof of exactness. No covariance
is supplied, so no combined group uncertainty is invented. The validator
compares the selected central yields and mapped line energies; it does not
fit parameters or resolve evaluation qualifiers through sampling.

## Existing campaign

From the repository root, use the existing `run_campaign.py` with this
`reference.json`, the rebuilt exporter, and a new scratch output directory.
The normal options are `--windows 32 --horizon-half-lives 4
--target-last-window 1000 --seed 77777 --workers 256`. Run `--device cpu`,
then `--device gpu` with a different output directory. The existing policy
caps this long-lived nuclide at 90% of the representable Time range; the
half-life is never rescaled. Use the unchanged `plot.py` on each output.

Sparse groups or individual lines cannot be claimed statistically validated
merely because a group-level comparison passes. Rare emissions are not
amplified. Existing discrete-energy figures may contain empty comparison
panels; the machine-readable results carry the statistical conclusions.
