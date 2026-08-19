#pragma once

#include <cstdint>
#include <span>
#include <string>
#include <string_view>
#include <vector>

#include "GGEMS/units/GGEMSDensityUnits.hh"

namespace ggems::core::materials {

struct GGEMSMaterialComponent {
  std::uint32_t atomic_number;
  long double mass_fraction;
};

struct GGEMSMaterialConstituent {
  std::uint32_t atomic_number;
  long double mass_fraction;
  long double number_density_per_cubic_centimeter;
};

class GGEMSMaterial {
public:
  GGEMSMaterial(std::string name, units::Density density,
                std::vector<GGEMSMaterialComponent> composition);

  [[nodiscard]] auto GetName() const noexcept -> std::string_view {
    return name_;
  }

  [[nodiscard]] auto GetDensity() const noexcept -> units::Density {
    return density_;
  }

  [[nodiscard]] auto GetConstituents() const noexcept
      -> std::span<GGEMSMaterialConstituent const> {
    return constituents_;
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
  std::string name_;
  units::Density density_;
  std::vector<GGEMSMaterialConstituent> constituents_;
  long double total_atom_density_per_cubic_centimeter_{0.0L};
  long double electron_density_per_cubic_centimeter_{0.0L};
};

} // namespace ggems::core::materials
