#pragma once

#include <cstdint>
#include <span>
#include <vector>

#include "GGEMS/materials/GGEMSIsotope.hh"
#include "GGEMS/materials/GGEMSIsotopicComposition.hh"
#include "GGEMS/materials/GGEMSResolvedIsotopeTable.hh"
#include "GGEMS/units/GGEMSDensityUnits.hh"

namespace ggems::core::materials {

struct GGEMSElementalShare {
  long double mass_fraction;
  GGEMSIsotopicComposition isotopic_composition;
};

struct GGEMSIsotopeConstituent {
  GGEMSIsotope isotope;
  long double atom_fraction_in_element;
  long double number_density_per_cubic_centimeter;
};

struct GGEMSDerivedElementalConstituent {
  std::uint32_t atomic_number;
  long double mass_fraction;
  long double number_density_per_cubic_centimeter;
  long double electron_density_per_cubic_centimeter;
};

class GGEMSMaterialComposition {
public:
  GGEMSMaterialComposition(units::Density density,
                           std::vector<GGEMSElementalShare> elemental_shares,
                           GGEMSResolvedIsotopeTable const &resolved_isotopes);

  [[nodiscard]] auto GetElementalShares() const noexcept
      -> std::span<GGEMSElementalShare const> {
    return elemental_shares_;
  }

  [[nodiscard]] auto GetIsotopeConstituents() const noexcept
      -> std::span<GGEMSIsotopeConstituent const> {
    return isotope_constituents_;
  }

  [[nodiscard]] auto GetElementalConstituents() const noexcept
      -> std::span<GGEMSDerivedElementalConstituent const> {
    return elemental_constituents_;
  }

  [[nodiscard]] auto GetTotalAtomDensityPerCubicCentimeter() const noexcept
      -> long double {
    return total_atom_density_per_cubic_centimeter_;
  }

  [[nodiscard]] auto GetElectronDensityPerCubicCentimeter() const noexcept
      -> long double {
    return electron_density_per_cubic_centimeter_;
  }

private:
  std::vector<GGEMSElementalShare> elemental_shares_;
  std::vector<GGEMSIsotopeConstituent> isotope_constituents_;
  std::vector<GGEMSDerivedElementalConstituent> elemental_constituents_;
  long double total_atom_density_per_cubic_centimeter_{0.0L};
  long double electron_density_per_cubic_centimeter_{0.0L};
};

} // namespace ggems::core::materials
