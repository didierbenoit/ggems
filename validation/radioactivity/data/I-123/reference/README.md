# I-123 selected source-model reference

This reference independently reconstructs the four selected prompt marginal
emission groups from retained scientific files. It is not a reconstruction
from GGEMS arrays, GoogleTest expectations or compiled exports. All numerical
source data come from [the retained raw evidence](../raw/). No continuous
spectrum is needed for this entirely discrete source model.

## Authority and nuclear quantities

The primary authority is V. Chiste and M. M. Be, CEA/LNE-LNHB/DDEP,
I-123/Te-123. The documentary dates remain distinct: the commentary carries
2001; PenNuc explicitly states `16/07/2003`; the main tables carry
`02/03/01 - 5/8/2004`, and the decay diagrams carry
`02/03/2001 - 16/07/2003`. The commentary includes later half-life references.
See [tables](../raw/lnhb/I-123_tables.pdf),
[commentary](../raw/lnhb/I-123_com.pdf),
[LARA](../raw/lnhb/I-123.lara.txt) and
[PenNuc](../raw/lnhb/I-123.PenNuc.txt).

The selected half-life is **13.2234(37) h**, or exactly **47604.24 s** with
**13.32 s** standard uncertainty after multiplication by 3600. LARA also
publishes rounded **47.604(13)e3 s**; retained BetaShape input uses
**4.7604e4(13) s**. These rounded seconds do not replace the selected hour
central value. The nuclear decay energy is **Q+ = 1234(3) keV**; it is not a
runtime particle energy.

## Evaluated electron capture and BetaShape

The selected decay is EC to Te-123. The complete selected EC inventory below
comes from the level and E records of [the LNHB input](../raw/lnhb/I-123.txt),
corroborated by table section 2.1. Intensities are per 100 parent decays.

| Te-123 level (keV) | EC intensity (%) | Standard uncertainty (%) |
|---:|---:|---:|
| 158.99 | 97.18 | 0.32 |
| 440.00 | 0.419 | 0.005 |
| 489.70 | 0.0025 | 0.0010 |
| 505.34 | 0.349 | 0.042 |
| 687.95 | 1.40 | 0.12 |
| 697.52 | 0.419 | 0.013 |
| 769.26 | 0.0037 | 0.0006 |
| 783.60 | 0.1461 | 0.0020 |
| 894.74 | 0.0744 | 0.0013 |
| 996.06 | 0.0035 | 0.0003 |
| 1036.64 | 0.0025 | 0.0009 |
| 1068.18 | 0.0079 | 0.0004 |

Rounded central intensities sum to **100.0076%**; they are not renormalized.
Commentary pp. 1-2 states that the 247-keV isomer is not populated in this
selected EC scheme. Feeding through the 532-keV level, if present, would be
very small. The evaluation's intensity balance assumes no EC to the ground
state or the 599-keV level and discusses possible unobserved weak feeding.
Those speculative branches are not added to the selected inventory.

The exact [retained command](../raw/betashape/v2.4/command.txt) is:

```text
.\betashape.exe I123\I-123.txt fixint=1 -csv
```

BetaShape **2.4 (06/2024)** is supporting EC evidence, not an energy-spectrum
generator for this reference. `I-123.trans`, `I-123.rpt`, `I-123_ecbp.csv` and
the twelve `capt_I123_trans*.bs` outputs preserve the twelve selected EC
intensities. For the 158.99-keV level, the report considers a possible
**53(3)-keV** positron transition and calculates **EC/b+ = 4.1(16)e6**;
with `fixint=1` its selected branching remains **Iec = 97.18(32)% and
Ib+ = 0%**. Thus absence of a positron group follows the evaluated selection,
not a claim that Q lies below threshold. No theoretical positron branch or
EC placeholder is introduced.
The retained transition listing also marks the possible beta-plus decay
information as uncertain; that warning is not a selected positive intensity.

Calculated shell probabilities are supporting comparisons, not replacement
evaluated emissions. For example, dominant-branch K/L probabilities are
0.85137(31)/0.11643(13), versus LNHB input 0.8533(14)/0.1163(10).
The 996.06-keV EC transition is classified `1N`, with `A` used in the retained
calculation; other selected calculations also use `A`. This approximation
does not supply or alter a runtime beta spectrum. No BetaShape rerun,
`-qval`, endpoint transformation or physical-yield normalization is used.

## Four ordered emission groups

| Group | Particle / distribution | Selected signatures | Yield per parent decay | Numerical authority |
|---:|---|---:|---:|---|
| 0 | Gamma / DiscreteLines | 40 nuclear gamma | 0.86195334 | LNHB/LARA, corroborated by PenNuc GA |
| 1 | Gamma / DiscreteLines | 5 compact X rays | 0.9569 | LNHB/LARA |
| 2 | Electron / DiscreteLines | 13 Auger entries | 13.705816482 | Retained MIRDspecs summary |
| 3 | Electron / DiscreteLines | 36 K/L conversion | 0.155575455 | LNHB/PenNuc EK/EL |

The flattened total is **15.680245277 particles per parent decay**. These are
emitted-particle multiplicities, not mutually exclusive parent branches.
Physical yields remain unnormalized, including yields above one. Only the
conditional DiscreteLines sampling probabilities divide by a group's sum.

**Nuclear gamma:** all 40 `g` entries in LARA's 45-line direct photon export,
including **158.97(5) keV at 0.8325(21) per parent**. All energies and absolute
yields exactly agree with the PenNuc GA records. The direct photon inventory
is selected rather than the older relative-intensity appendix or rounded
level differences. In particular, the direct 158.97-keV photon energy and the
158.99-keV daughter level are different tabulated quantities.

**X rays:** the remaining five LARA entries are retained as published:

| Signature | Representative energy (keV) | Yield per parent decay | Underlying LNHB entries |
|---|---:|---:|---|
| XL | 4.078 | 0.09 | Total L range 3.336-4.82 keV |
| K-alpha-2 | 27.202 | 0.2469 | K-alpha-2 |
| K-alpha-1 | 27.4726 | 0.4598 | K-alpha-1 |
| K-beta-1 group | 31.1044 | 0.1316 | K-beta-3, K-beta-1, K-beta-5II/I at 30.9446, 30.996, 31.236, 31.241 keV |
| K-beta-2 group | 31.7623 | 0.0286 | K-beta-2, K-beta-4, KO23 at 31.7008, 31.774, 31.812 keV |

The grouped energies are the explicit LARA representatives; unavailable
microscopic line weights are not inferred. Table section 5.1 and LNHB input
T records establish the component identities. No detailed MIRD X rays are
transplanted into these five groups.

**Auger:** take exactly the 13 `Auger electron` rows, source indices 94-106,
from [I-123 Summary Spectrum.csv](../raw/mird/I-123%20Summary%20Spectrum.csv).
The original columns are `Energy [MeV]` and `Yield [#/nt]`.
[MIRDspecs identifies ICRP Publication 107 as its spectra source](https://mirdsoft.org/products/MIRDspecs/MIRDspecs_HTMLs/I-123.htm).
These are the selected summary-spectrum channels, not a claim to resolve
every microscopic subshell transition. ICRP's model includes outer-shell
Auger/CK cascades and collects emissions into composite channels; its yields
count emitted particles per nuclear transformation, and tabulated energies
can be unique or average energies.
[ICRP 107, sections 2.6 and 3.2.2-3.2.4](https://journals.sagepub.com/doi/pdf/10.1177/ANIB_38_3).

LNHB instead tabulates KLL 21.804-22.989, KLX 25.814-27.470 and KXY
29.80-31.81 keV with relative family probabilities 100:45.3:5.13, a combined
K yield **0.124(4)**, and L electrons at 2.3-4.8 keV with yield **0.953(6)**.
The selected MIRD rows within the K and L energy regions sum to
**0.12003215** and **0.9570959**, respectively. The other seven rows below
1 keV sum to **12.628688432**; this independently accounts for the large
total in the more extensive atomic-cascade representation. Exact subshell
labels are not supplied by this summary CSV and are not invented.
The legacy `mird-I-123-table-0.csv` grouped Auger rows are a cross-check,
not the selected 13-row source. The DPK file is not an emission inventory.

LNHB commentary attributes X/Auger probabilities to EMISSION and atomic
parameters to E. Schonfeld and H. Janssen, NIM A 369, 527 (1996), `1996Sc06`.
Compact LNHB X rays and MIRD Auger electrons are a deliberate selection of
marginals from different atomic calculations. They are not asserted to be
one identical cascade calculation. No photon/electron signature is duplicated;
the selected Auger energies end at 30.3461 keV, below the first selected
conversion electron at 127.18 keV. MIRD conversion rows are not also included.

**Conversion:** all 36 EK/EL records from PenNuc, sorted by electron energy.
Every JSON line retains its K/L shell, parent GA energy, upper and lower
level indices, source row and available uncertainties. These are 18 K/L
pairs, not records in LARA's direct photon export. Commentary attributes
conversion coefficients to Icc99v3a/GETICC with Rosel tables and obtains
electron intensities from coefficients and photon yields; it notes the lack
of measured conversion intensities. No extra M/N/O shell inventory is added.

## Representation and cross-source differences

`reference.json` preserves source energies and physical intensities. Following
the established reference convention, `energy_keV` identifies the documented
represented line energy for discrete CDF comparisons. Historical nearest-meV
rounding affects two Auger entries only: **0.0229238 -> 0.022924 keV**
(+200 micro-eV) and **0.0249261 -> 0.024926 keV** (-100 micro-eV).
Both raw MeV/keV values and residuals remain explicit. All other selected
energies are exactly representable. This numerical mapping neither changes
a physical yield nor substitutes a scientific energy silently.

ENSDF's retained Jun Chen evaluation, NDS 174, 1 (2021), has a 15-Apr-2021
literature cutoff. It selects **13.2230(19) h**, **Q = 1228(3) keV**, and
includes **0.0044(5)% EC to 532.82 keV**; dominant 159.00-keV-level feeding
is **97.0(5)%**. These differ from selected LNHB values and are not averaged
or used to add isomer feeding. MIRD's separate nuclear lineage uses a
13.27-h half-life and lists a weak Te-123m daughter contribution; it does
not replace LNHB nuclear data. Its Auger marginal is used only as the
explicitly selected atomic representation.

No runtime positron, EC placeholder, source-level 511-keV signature, capture
neutrino, daughter recoil or later Te-123 decay is represented. The compiled
source's historical BetaShape 2.2 comment and attribution of conversion
electrons to direct LARA are documentary issues; the separate proposed
comments-only correction identifies active 2.4 EC evidence and PenNuc EK/EL.
Production numerical data and group order are unchanged.

The standard campaign uses the modeled total above, without rare-line
amplification. Empty samples and complete DKW limits >= 1 are
`insufficient_samples`. Group energy laws are tested; a weak individual
line need not appear in a finite sample.
