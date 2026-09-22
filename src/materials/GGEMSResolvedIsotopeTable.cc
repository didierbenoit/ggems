#include <algorithm>
#include <format>
#include <utility>
#include <vector>

#include "GGEMS/GGEMSException.hh"
#include "GGEMS/materials/GGEMSIsotope.hh"
#include "GGEMS/materials/GGEMSResolvedIsotopeTable.hh"

namespace ggems::core::materials {

// =============================================================================
// =============================================================================

GGEMSResolvedIsotopeTable::GGEMSResolvedIsotopeTable(
  std::vector<GGEMSResolvedIsotope> resolved_isotopes)
    : resolved_isotopes_{std::move(resolved_isotopes)} {
  std::ranges::sort(resolved_isotopes_, {}, &GGEMSResolvedIsotope::isotope);
}

// -----------------------------------------------------------------------------

[[nodiscard]] auto
GGEMSResolvedIsotopeTable::Find(GGEMSIsotope const &isotope) const noexcept
  -> GGEMSResolvedIsotope const * {
  auto const found = std::ranges::lower_bound(resolved_isotopes_, isotope, {},
                                              &GGEMSResolvedIsotope::isotope);

  if (found == resolved_isotopes_.end() || found->isotope != isotope) {
    return nullptr;
  }

  return &*found;
}

// -----------------------------------------------------------------------------

[[nodiscard]] auto
GGEMSResolvedIsotopeTable::Require(GGEMSIsotope const &isotope) const
  -> GGEMSResolvedIsotope const & {
  auto const *resolved_isotope = Find(isotope);

  if (resolved_isotope == nullptr) {
    throw GGEMSRecoverable{
      std::format("No resolved molar mass for isotope (Z={}, A={}, M={}).",
                  isotope.GetAtomicNumber(), isotope.GetMassNumber(),
                  isotope.GetIsomerState())};
  }

  return *resolved_isotope;
}

} // namespace ggems::core::materials
