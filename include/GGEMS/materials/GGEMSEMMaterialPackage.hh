#pragma once

#include <cstdint>
#include <span>
#include <vector>

#include "GGEMS/materials/GGEMSMaterial.hh"
#include "GGEMS/units/GGEMSDensityUnits.hh"

namespace ggems::core::materials {

struct GGEMSEMElementalConstituent {
  std::uint32_t atomic_number;
  long double mass_fraction;
  long double number_density_per_cubic_centimeter;
  long double electron_density_per_cubic_centimeter;
};

struct GGEMSEMMaterialDescriptor {
  units::Density density;
  std::uint32_t first_constituent;
  std::uint32_t constituent_count;
  long double total_atom_density_per_cubic_centimeter;
  long double electron_density_per_cubic_centimeter;
};

class GGEMSEMMaterialPackage {
public:
  explicit GGEMSEMMaterialPackage(std::span<GGEMSMaterial const> materials);

  [[nodiscard]] auto GetDescriptors() const noexcept
    -> std::span<GGEMSEMMaterialDescriptor const> {
    return descriptors_;
  }

  [[nodiscard]] auto GetElementalConstituents() const noexcept
    -> std::span<GGEMSEMElementalConstituent const> {
    return elemental_constituents_;
  }

  [[nodiscard]] auto GetMaterialIds() const noexcept
    -> std::span<std::uint32_t const> {
    return material_ids_;
  }

private:
  std::vector<GGEMSEMMaterialDescriptor> descriptors_;
  std::vector<GGEMSEMElementalConstituent> elemental_constituents_;
  std::vector<std::uint32_t> material_ids_;
};

} // namespace ggems::core::materials
