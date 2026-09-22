#pragma once

#include <vector>

#include "GGEMS/materials/GGEMSIsotope.hh"

namespace ggems::core::materials {

struct GGEMSResolvedIsotope {
  GGEMSIsotope isotope;
  long double molar_mass_grams_per_mole;
};

class GGEMSResolvedIsotopeTable {
public:
  explicit GGEMSResolvedIsotopeTable(
    std::vector<GGEMSResolvedIsotope> resolved_isotopes);

  [[nodiscard]] auto Find(GGEMSIsotope const &isotope) const noexcept
    -> GGEMSResolvedIsotope const *;

  [[nodiscard]] auto Require(GGEMSIsotope const &isotope) const
    -> GGEMSResolvedIsotope const &;

private:
  std::vector<GGEMSResolvedIsotope> resolved_isotopes_;
};

} // namespace ggems::core::materials
