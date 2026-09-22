#pragma once

#include <cstdint>
#include <optional>
#include <string>
#include <string_view>
#include <vector>

#include "GGEMS/materials/GGEMSMaterial.hh"
#include "GGEMS/materials/GGEMSMaterialComposition.hh"
#include "GGEMS/materials/GGEMSMaterialManager.hh"
#include "GGEMS/units/GGEMSDensityUnits.hh"

namespace ggems::core::materials {

enum class GGEMSMaterialRegistration : std::uint8_t {
  Unknown = 0U,
  Unregistered = 1U,
  Registered = 2U,
};

struct GGEMSElementInspection {
  GGEMSDerivedElementalConstituent values;
  std::string symbol;
  std::string name;
};

struct GGEMSMaterialInspection {
  std::string name;
  units::Density density;
  GGEMSMaterialRegistration registration;
  std::optional<std::uint32_t> manager_index;
  std::vector<GGEMSElementInspection> elements;
  std::vector<GGEMSIsotopeConstituent> isotopes;
  long double total_atom_density_per_cubic_centimeter;
  long double electron_density_per_cubic_centimeter;
};

[[nodiscard]] auto InspectMaterial(GGEMSMaterial const &material)
  -> GGEMSMaterialInspection;

[[nodiscard]] auto InspectMaterial(GGEMSMaterialManager const &manager,
                                   std::uint32_t manager_index)
  -> GGEMSMaterialInspection;

[[nodiscard]] auto InspectMaterial(GGEMSMaterialManager const &manager,
                                   std::string_view name)
  -> GGEMSMaterialInspection;

[[nodiscard]] auto DescribeMaterial(GGEMSMaterial const &material)
  -> std::string;

[[nodiscard]] auto DescribeMaterial(GGEMSMaterialInspection const &inspection)
  -> std::string;

auto VerboseMaterial(GGEMSMaterial const &Material) -> void;

auto VerboseMaterial(GGEMSMaterialInspection const &inspection) -> void;

} // namespace ggems::core::materials
