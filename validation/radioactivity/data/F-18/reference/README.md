# GGEMS declared F-18 source-model validation reference

This package validates the **three emissions currently modeled by GGEMS**. It
does not certify complete F-18 atomic relaxation. The known physical K-Auger
marginal is documented below and deliberately absent from the generated-group
reference because its selected conditional energy law remains unresolved.

All scientific source paths below are relative to `../raw/`. The reference was
recovered from those files independently of the production arrays and GoogleTest
expectations.

## Selected physical evidence

The authority is the LNHB/KRI F-18/O-18 evaluation by **V. Chiste, M.-M. Be and
N. K. Kuzmenko**, updated in 2014. `lnhb/F-18_com.pdf`, p. 1, identifies the
original evaluation and the June 2014 half-life/Q update. Nuclear values and
atomic marginals come from `lnhb/F-18_tables.pdf`, `lnhb/F-18.lara.txt` and
`lnhb/F-18.PenNuc.txt`.

| Selected quantity | Central value and published uncertainty | Source |
|---|---|---|
| Half-life | 1.82890(23) h = **6584.04 +/- 0.828 s** | Tables p. 1; LARA lines 6-7 |
| Q+ | **1655.9 +/- 0.5 keV** | Tables p. 1; PenNuc line 8 |
| Positron endpoint | **633.9 +/- 0.5 keV** | Tables p. 1; PenNuc line 10 |
| Beta-plus yield | **0.9686 +/- 0.0019** per parent | Tables p. 1; PenNuc line 10 |
| EC probability | **0.0314 +/- 0.0019** per parent | Tables p. 1; comments p. 3 |
| K capture | **0.0291 +/- 0.0018** per parent | PenNuc line 11 |
| L1 capture | **0.00229 +/- 0.00021** per parent | PenNuc line 12 |
| K-Auger | **0.0289 +/- 0.0018** electron per parent; KLL **0.456-0.502 keV** | Tables p. 2, sections 3.1.2 and 4 |
| L-Auger | **0.00229 +/- 0.00021** electron per parent at **0.0143 keV** | Tables p. 2, section 4 |
| O K-alpha-2 | **0.00007 +/- 0.00002** photon per parent at **0.525 keV** | Tables p. 3; LARA line 14 |
| O K-alpha-1 | **0.00013 +/- 0.00004** photon per parent at **0.525 keV** | Tables p. 3; LARA line 15 |

The half-life uses the more precise published hour value converted with exactly
3600 s/h. LARA's separate 6584.0 +/- 0.8 s entry is rounded. No uncertainty is
invented for the atomic energies. The two X-ray uncertainties are retained
individually; a combined independent-error uncertainty is not assumed.

The daughter is stable O-18 in its ground state. EC is a parent decay mode, not
an incident particle. The evaluated annihilation signature is 511 keV with
1.9372(38) photons per parent (`lnhb/F-18_tables.pdf`, p. 3). **It is excluded
from Source groups because GGEMS emits the positron itself.** It is distinct
from Q+ and from the positron kinetic-energy endpoint.

## Declared modeled subset and K-Auger limitation

| Group | Particle | Distribution | Yield per parent | Selected energy law |
|---:|---|---|---:|---|
| 0 | Positron | RegularSpectrum | 0.9686 | Experimental BetaShape 2.4 spectrum, 0-633.9 keV |
| 1 | Electron | Mono | 0.00229 | LNHB L-Auger, 0.0143 keV |
| 2 | Gamma | Mono | 0.00020 | O K-alpha-2 and K-alpha-1 merged at their equal 0.525-keV energy |

The current modeled total is **0.97109 particles per parent decay**. This is the
yield used for automatic activity selection and all ActivityDriven population
expectations. It is not normalized to one and is not increased to compensate for
excluded emissions.

**K-Auger: KNOWN PHYSICAL EMISSION; NOT CURRENTLY MODELED IN GGEMS.** The selected
yield is established, and the central K-vacancy balance is

```text
0.0289 K-Auger + 0.00020 K X-rays = 0.02910 K capture.
```

The LNHB atomic table assigns 100% of the K-Auger family to KLL. It supplies a
grouped energy interval, not the component probabilities or a supported mean.
`lnhb/F-18_com.pdf`, p. 4, attributes atomic energies and fluorescence data to
SAISINUC and absolute K-Auger/X-ray probabilities to EMISSION.

The focused representation audit did not recover a sufficiently defined
conditional KLL energy law for this selected evaluation. The separate MIRD-format
0.5200-keV starred quantity is a mean from another data lineage and lies outside
the selected interval. The available EADL KLL transitions use a different atomic
model and do not reproduce that interval. Neither was transplanted. The omission
does not mean K-Auger electrons are absent, impossible, non-transportable or
negligible. No cut, midpoint, uniform law, synthetic line, zero-yield group or
placeholder is introduced.

Adding the known unmodeled central K-Auger yield would give **0.99999** in
explanatory bookkeeping; this is **not** the current source expectation. The
physical inventory therefore exceeds the modeled inventory.

The generic electron emission figure contains only the modeled L-Auger marginal;
it must not be interpreted as the complete physical F-18 electron spectrum. The
gamma emission figure contains only the modeled O X-rays, never 511-keV photons.
Validation applies to this declared subset: **VALIDATED WITH DECLARED MODEL
LIMITATION** is appropriate only after the compiled and statistical comparisons
have passed.

A future extension requires either the detailed oxygen KLL data underlying the
selected SAISINUC/EMISSION evaluation or an explicitly adopted, documented
alternative atomic-relaxation model. Neither selection is made here.

## Positron spectrum and BetaShape provenance

The retained command in `betashape/v2.4/command.txt` is:

```text
.\betashape.exe F18\F-18.txt fixint=1 -csv
```

The output identifies **BetaShape 2.4 (06/2024)** and an allowed transition.
`fixint=1` retains the evaluated beta-plus/EC split, 96.86(19)% / 3.14(19)%.
There is no `-qval`, endpoint rescaling or new BetaShape run.

`betashape/v2.4/output/beta+_F18_trans0.bs`, lines 19-25, selects the experimental
factor **1 + 0.0034 W**, citing **1964HO28, I. Hofmann, Acta Phys. Austriaca 18,
309 (1964)**. The measurement range is **70-600 keV**. This is shape-factor
evidence over that interval; the retained BetaShape experimental calculation
provides the full 0-633.9-keV spectrum. The output also records screening,
radiative and atomic-overlap corrections.

The calculated header mean is **249.65(21) keV**; the experimental header mean
is **250.50(21) keV**. The report's rounded coefficient card is
`C1=0.003 9 (1964HO28)`, while the spectrum header gives 0.0034. That card is
retained as provenance, not used to recalculate the spectrum. The selected data
are the actual **dN/dE exp.** column and its adjacent uncertainty column, not the
calculated allowed column. The retained `.trans` file also includes the generic
warning that information about this decay is not totally sure; no uncertainty
or shape correction is invented in response.

`beta_plus_633_9_keV.csv` copies **318** raw energy/density/uncertainty triples
without numerical changes: 0 to 632 keV in 2-keV steps, then 633.9 keV. No GGEMS
bin is inserted. Piecewise-linear integration gives:

- Experimental raw integral: **0.9686009838625**.
- Experimental conditional reference mean: **250.495951191927184 keV**.
- Calculated conditional mean, for comparison only: **249.652472047409652 keV**.

Only the conditional spectrum is divided by its integral. The physical
positron yield remains 0.9686. Pointwise uncertainties are retained, but no
covariance is available; deterministic/statistical comparisons use the central
shape without treating those uncertainties as independent errors.

The historical embedded production table originated from BetaShape 2.2. Its
unchanged bin masses are independently reproduced by the retained 2.4
experimental data. The active evidence for this reference is 2.4 only. The
compiled grid and its finite-ticket approximation remain separate from the
full-support scientific reference; low-energy reference mass outside the
half-open compiled support is not discarded from the comparison.

## Source precedence and disagreements

LNHB/LARA/PenNuc remains authoritative for nuclear quantities and modeled atomic
marginals. BetaShape supplies the selected conditional positron shape. Its
calculated K=0.03044(45) and L=0.002272(35) capture values are comparisons, not
replacements for the selected PenNuc capture values. `fixint=1` preserves the
overall EC/beta-plus split; it does not make every calculated shell probability
identical to the selected atomic data.

ENSDF is a cross-check: `ensdf/ec_decay.pdf` uses an older evaluation with
109.77(5) min, Q=1655.50(63) keV, beta-plus 96.73% and EC 3.27%. No values are
averaged with LNHB. The MIRD-format summary gives Auger-K yield 0.0307, versus
LNHB 0.0289(18); the difference is 0.0018, one quoted LNHB uncertainty, without
implying a combined significance test. MIRD supplies no selected modeled group
in this package.
