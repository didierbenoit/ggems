# Ra-223 selected reference

Select the retained LNHB/KRI evaluation (2010), converting 11.43 d
with exactly 86400 s/d. Keep all 26 positive direct parent alpha lines as
one discrete law, without renormalizing their evaluated sum. LNHB/LARA
supplies nuclear photons and compact Rn X rays; PenNuc supplies conversion
electrons grouped by nuclear transition. These relax levels populated
directly by the parent. Nanosecond level relaxation (including the 15.4 ns
4.47 keV level) uses the existing prompt-marginal approximation; its timing
is not modeled. Later radioactive Rn-219 decay and its descendants are excluded.
The retained BetaShape run supplies no continuous alpha law and is not used
as an alpha generator. The MIRD DPK is not an emission probability law.

Unsupported Auger energy laws, neutrinos, recoil and additional radioactive
daughter decays are excluded. Selected groups are independent marginals;
ENSDF and MIRD are cross-checks only and are not averaged with LNHB.

Complete evidence remains in [raw/](../raw/). Use
[reference.json](reference.json) with the
[generic validation commands](../../../README.md).
