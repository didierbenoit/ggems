#include <format>
#include <string>
#include <string_view>

#include "GGEMS/logging/GGEMSLogger.hh"
#include "GGEMS/logging/GGEMSLogMacros.hh"
#include "GGEMS/materials/GGEMSMaterial.hh"
#include "GGEMS/materials/GGEMSElementCatalog.hh"
#include "GGEMS/materials/GGEMSMaterialDescription.hh"
#include "GGEMS/materials/builtins/GGEMSBuiltInMaterials.hh"
#include "GGEMS/units/GGEMSUnitFormatting.hh"

namespace {

[[nodiscard]] auto NumberDensityUnit() noexcept -> std::string_view {
  return ggems::core::GGEMSLogger::GetInstance().GetEncoding() ==
                 ggems::core::Encoding::Ascii
             ? "1/cm3"
             : "1/cm³";
}

} // namespace

namespace ggems::core::materials {

// =============================================================================
// =============================================================================

[[nodiscard]] auto DescribeMaterial(GGEMSMaterial const &material)
    -> std::string {
  auto const constituents = material.GetConstituents();

  std::string description = std::format(
      "Material: {}\n"
      "  Density          : {}\n"
      "  Constituents     : {}\n",
      material.GetName(), units::HumanReadable(material.GetDensity()),
      constituents.size());

  for (auto const &constituent : constituents) {
    auto const &element =
        RequireElementByAtomicNumber(constituent.atomic_number);

    description += std::format(
        "  {:<2} {:<12} w={:.8g}  n={:.8g} {}\n", element.GetSymbol(),
        element.GetName(), constituent.mass_fraction,
        constituent.number_density_per_cubic_centimeter, NumberDensityUnit());
  }

  description += std::format(
      "  Atom density     : {:.8g} {}\n"
      "  Electron density : {:.8g} {}",
      material.GetTotalAtomDensityPerCubicCentimeter(), NumberDensityUnit(),
      material.GetElectronDensityPerCubicCentimeter(), NumberDensityUnit());

  return description;
}

// =============================================================================
// =============================================================================

[[nodiscard]] auto DescribeAvailableMaterials() -> std::string {
  auto const names = builtins::GetAvailableMaterialNames();

  std::string description =
      std::format("Available built-in Materials: {}", names.size());

  for (auto const name : names) {
    description += std::format("\n  {}", name);
  }

  return description;
}

// =============================================================================
// =============================================================================

auto VerboseAvailableMaterials() -> void {
  GGEMS_INFO("Material", "{}", DescribeAvailableMaterials());
}

// =============================================================================
// =============================================================================

auto VerboseMaterial(GGEMSMaterial const &material) -> void {
  GGEMS_INFO("Material", "{}", DescribeMaterial(material));
}

} // namespace ggems::core::materials
