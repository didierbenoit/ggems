#pragma once

#include <cstdint>
#include <optional>
#include <span>
#include <string>
#include <string_view>
#include <vector>

#include "GGEMS/materials/GGEMSIsotopeProfile.hh"
#include "GGEMS/materials/GGEMSMaterialComposition.hh"
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
  std::optional<GGEMSIsotopeProfile> isotope_profile;
};

class GGEMSMaterial {
public:
  GGEMSMaterial(std::string name, units::Density density,
                std::vector<GGEMSMaterialComponent> const &composition);

  [[nodiscard]] static auto
  FromIsotopicComposition(std::string name, units::Density density,
                          std::vector<GGEMSElementalShare> elemental_shares)
      -> GGEMSMaterial;

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

  [[nodiscard]] auto GetIsotopeConstituents() const noexcept
      -> std::span<GGEMSIsotopeConstituent const>;

  [[nodiscard]] auto GetTotalAtomDensityPerCubicCentimeter() const noexcept
      -> long double;

  [[nodiscard]] auto GetElectronDensityPerCubicCentimeter() const noexcept
      -> long double;

private:
  GGEMSMaterial(std::string name, units::Density density);

  auto Compile(std::vector<GGEMSElementalShare> elemental_shares,
               bool resolved_from_profiles) -> void;

  std::string name_;
  units::Density density_;
  std::optional<GGEMSMaterialComposition> composition_;
  std::vector<GGEMSMaterialConstituent> constituents_;
};

} // namespace ggems::core::materials
