# C-11 source-model validation reference

This independent reference describes the selected single-positron C-11 source
model. It was recovered from the raw evaluation and BetaShape files before
comparison with GGEMS constants, tables or GoogleTests. It does not certify
complete atomic relaxation or assert that atomic emissions are physically absent.

## Selected nuclear evaluation

The primary authority is V. Chisté and M.-M. Bé, LNHB/DDEP, **C-11 / B-11**.
[The comments](../raw/lnhb/C-11_com.pdf), p. 1, state that the evaluation was
completed in 2002 and its half-life updated in 2011 to include Wood's result.
[The tables](../raw/lnhb/C-11_tables.pdf) carry `23/07/2002 - 3/11/2011`;
the retained ENSDF-format LNHB input records `30-MAR-2002` and `01-NOV-2011`,
and the [PenNuc export](../raw/lnhb/C-11.PenNuc.txt) is dated `04/11/2011`.
These are distinct evaluation/export dates.

| Selected quantity | Central value and standard uncertainty | Evidence |
|---|---|---|
| Half-life | 20.361 ± 0.023 min = **1221.66 ± 1.38 s** | LNHB tables p. 1; LARA half-life rows |
| Q+ | 1982.5 ± 0.9 keV | LNHB tables p. 1; PenNuc `Q` |
| Positron endpoint | 960.5 ± 0.9 keV | LNHB tables pp. 1–2; PenNuc `BEP` |
| Positron yield | **0.99750 ± 0.00013 per parent decay** | LNHB tables §2.1; PenNuc `BEP` |
| EC probability | 0.00250 ± 0.00013 per parent decay | LNHB tables §2.2; LNHB input `E` record |
| Daughter | Stable B-11 ground state, 3/2− | LNHB decay scheme and input `L` record |

[LARA](../raw/lnhb/C-11.lara.txt) rounds the seconds representation to
`1.2217E3 ± 0.0014E3 s`. BetaShape's retained transition report likewise uses
`1221.7 (14) s`. The reference converts the more precise selected minute value
with exactly 60 s/min. It does not adopt a new central value to match a rounded
export. The LNHB comments also report a measured endpoint average of 959.8(5) keV;
the selected Q-derived endpoint remains 960.5(9) keV.

## Selected positron spectrum

Active evidence is **BetaShape 2.4 (06/2024)**. The exact
[retained command](../raw/betashape/v2.4/command.txt) is:

```text
.\betashape.exe C11\C-11.txt fixint=1 -csv
```

`fixint=1` retains the evaluated 99.750% beta-plus / 0.250% EC split. No `-qval`,
endpoint rescaling, physical-yield normalization or new BetaShape run is used.
The [transition spectrum](../raw/betashape/v2.4/output/beta+_C11_trans0.bs)
classifies 3/2− → 3/2− as **allowed** and includes numerical screening, radiative
and atomic-overlap corrections. Its selected experimental factor is
**`1 - 0.0074*W`**, from **1975BE28**, H. Behrens, M. Kobelt, L. Szybisz and
W.-G. Thies, *Nucl. Phys. A* **246**, 317 (1975), measured over **266–892 keV**.
The report's rounded additional card reads `C1=-0.007 7 (1975BE28)` and
`Cexp(W) = 1+C1 W`. The retained experimental density is used directly; no factor
is reconstructed from that rounded card.

| Mean energy | keV |
|---|---:|
| LNHB/input historical evaluated mean | 385.7 ± 0.4 |
| BetaShape calculated header | 385.02 ± 0.39 |
| BetaShape experimental-shape header | 385.33 ± 0.39 |
| Full retained calculated piecewise-linear density | 385.023899531362 |
| **Selected experimental piecewise-linear density** | **385.332057465573** |

`beta_plus_960_5_keV.csv` copies the original energy, **experimental** density
and pointwise uncertainty tokens: 322 points from 0 to 960.5 keV. Its full
support is a BetaShape calculation using the measured factor; the measurement
itself did not cover that entire interval. The raw experimental trapezoidal
integral is 0.997486231362515. Conditional energy comparisons divide by this
integral; the physical positron yield remains 0.99750 separately. Pointwise
uncertainties are retained, but no covariance is supplied, so comparisons use
the selected central density. Neither the historical nor calculated mean
overrides the experimental spectrum.

The `.trans` file includes the generic warning `information about this decay
is not totally sure.` The selected report nevertheless explicitly provides the
allowed transition and experimental spectrum. No correction is invented from
that warning. The production table historically attributed to BetaShape 2.2
is reproduced numerically by this retained 2.4 experimental column; 2.2 is not
used as an active reference or copied into this package.

## Electron-capture and atomic-emission audit

The LNHB tables §2.2 give capture fractions **PK = 0.9174(91)** and
**PL = 0.0826(91)** conditional on EC. PenNuc's absolute shell probabilities are
K = **0.00229 ± 0.00012** and L = **0.000206 ± 0.000025** per parent decay.
Multiplying the central conditional fractions by 0.00250 gives 0.0022935 and
0.0002065, consistent with rounded capture records. These numbers count shell
captures, **not emitted Auger electrons**. The LNHB-format input separately
prints `CK=0.002294 23` and `CL=0.000207 23`; its rounding/uncertainties are
recorded without substituting them for the selected PenNuc representation.

The LNHB electron-emission table (§3, p. 2) lists only the beta-plus spectrum.
The photon table (§4.1) and LARA list only annihilation photons. **No Auger or
characteristic B X-ray marginal with an energy and emission yield is tabulated.**
The evaluation comments supply no explicit rationale for that absence. Thus
the evidence establishes non-tabulation, not a claim of physical nonexistence.
No selected LNHB atomic-emission row is missing from this source-model mapping.

BetaShape's calculated shell probabilities differ: K = 0.00208(7),
L = 0.000179(6), EC/beta-plus = 0.00227(7). Its report preserves the selected
overall EC/beta-plus split under `fixint=1`, while reporting these calculated
shell quantities separately. They do not replace LNHB or create atomic groups.

The legacy [MIRD-format table](../raw/mird/mird-C-11-table-0.csv) contains:

| Legacy row | Emissions per parent decay | Tabulated energy, converted from MeV to keV |
|---|---:|---:|
| Auger-K | 0.00221 | 0.1699* |
| K-alpha-1 X-ray | 0.00000243 | 0.1829 |
| K-alpha-2 X-ray | 0.00000124 | 0.1829 |

The starred Auger entry is a grouped/mean-energy MIRD-format summary, not a
resolved physical line list. This retained CSV omits its column header, star
legend and atomic-model generation record; it cannot establish a specific
C-11 cascade law or justify transplanting that alternative atomic model.
It also rounds the positron yield to 0.997 and the annihilation yield to 1.99.
The distinct [summary-spectrum CSV](../raw/mird/C-11%20Summary%20Spectrum.csv),
whose headers explicitly define MeV and emissions per nuclear transformation,
contains only annihilation (1.99534) and positron (0.997668, mean 385.623 keV)
rows. It supplies no detailed C-11 atomic line list. The two MIRD products are
cross-checks, not interchangeable selected evaluations. No MIRD Auger/X-ray
row is imported into this LNHB-selected source model.

[ENSDF](../raw/ensdf/ec_decay.pdf), J. H. Kelley and C. G. Sheu,
*Nucl. Phys. A* **880**, 88 (2012), cutoff 1 January 2011, reports a different
evaluation: half-life 1221.8(8) s, Q = 1982.4(10) keV, beta-plus 99.7669(25)%,
EC 0.2331(25)%, and mean positron energy 385.70(44) keV. These differences remain
visible; they are not averaged with or substituted for LNHB.

## Modeled emission and boundaries

| Group | Particle | Conditional law | Physical yield per parent decay |
|---:|---|---|---:|
| 0 | Positron | Experimental RegularSpectrum, endpoint 960.5 keV | 0.99750 |

The modeled flattened total is **0.99750**, without normalization to one.
EC creates no placeholder particle. No atomic electron, X-ray, ion or daughter
group is introduced. The model does not claim complete atomic relaxation.

LNHB/LARA's **511-keV, 1.99500 ± 0.00026 photons per parent decay** is the
annihilation signature, twice the selected positron yield. It is explicitly
excluded from source groups because GGEMS emits the positron itself and
annihilation belongs to subsequent transport. No source-level 511-keV photon
is generated or included in the source-emission figures.

The independent CSV retains the reference below the compiled lower edge.
Grid binning, finite-ticket apportionment, half-open support and within-bin CDF
residuals are implementation comparisons, not changes to this reference.
