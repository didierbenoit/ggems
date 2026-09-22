#include <algorithm>
#include <utility>
#include <vector>

#include "GGEMS/GGEMSException.hh"
#include "GGEMS/materials/GGEMSIsotopicComposition.hh"
#include "GGEMS/materials/detail/GGEMSFractionCanonicalization.hh"

namespace ggems::core::materials {

// =============================================================================
// =============================================================================

GGEMSIsotopicComposition::GGEMSIsotopicComposition(
  GGEMSFractionBasis basis, std::vector<GGEMSIsotopeFraction> fractions)
    : basis_{basis}, fractions_{std::move(fractions)} {
  // Every explicit entry, including an explicit zero, must name the same Z.
  if (!fractions_.empty()) {
    auto const atomic_number = fractions_.front().isotope.GetAtomicNumber();

    if (!std::ranges::all_of(
          fractions_,
          [atomic_number](GGEMSIsotopeFraction const &entry) -> bool {
            return entry.isotope.GetAtomicNumber() == atomic_number;
          })) {
      throw GGEMSRecoverable{
        "Isotopic composition must contain a single chemical element."};
    }
  }

  detail::CanonicalizeFractions(fractions_, &GGEMSIsotopeFraction::isotope,
                                &GGEMSIsotopeFraction::fraction, "Isotope");
}

} // namespace ggems::core::materials
