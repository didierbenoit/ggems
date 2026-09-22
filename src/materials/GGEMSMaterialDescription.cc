#include <cstddef>
#include <cstdint>
#include <format>
#include <optional>
#include <string>
#include <string_view>
#include <algorithm>

#include "GGEMS/logging/GGEMSLogger.hh"
#include "GGEMS/logging/GGEMSLogMacros.hh"
#include "GGEMS/materials/GGEMSElementCatalog.hh"
#include "GGEMS/materials/GGEMSIsotope.hh"
#include "GGEMS/materials/GGEMSMaterial.hh"
#include "GGEMS/materials/GGEMSMaterialDescription.hh"
#include "GGEMS/materials/builtins/GGEMSBuiltInMaterials.hh"
#include "GGEMS/materials/GGEMSMaterialManager.hh"
#include "GGEMS/units/GGEMSUnitFormatting.hh"

namespace {

// =============================================================================
// =============================================================================

namespace materials = ggems::core::materials;

// =============================================================================
// =============================================================================

[[nodiscard]] auto NumberDensityUnit() noexcept -> std::string_view {
  return ggems::core::GGEMSLogger::GetInstance().GetEncoding() ==
             ggems::core::Encoding::Ascii
           ? "1/cm3"
           : "1/cm³";
}

// =============================================================================
// =============================================================================

[[nodiscard]] auto IsotopeLabel(materials::GGEMSIsotope const &isotope)
  -> std::string {
  auto const symbol =
    materials::RequireElementByAtomicNumber(isotope.GetAtomicNumber())
      .GetSymbol();

  switch (isotope.GetIsomerState()) {
  case 0U:
    return std::format("{}-{}", symbol, isotope.GetMassNumber());
  case 1U:
    return std::format("{}-{}m", symbol, isotope.GetMassNumber());
  default:
    return std::format("{}-{}m{}", symbol, isotope.GetMassNumber(),
                       isotope.GetIsomerState());
  }
}

// =============================================================================
// =============================================================================

[[nodiscard]] auto
RegistrationLabel(materials::GGEMSMaterialInspection const &inspection)
  -> std::string {
  switch (inspection.registration) {
  case materials::GGEMSMaterialRegistration::Registered:
    return std::format("registered, manager material index {}",
                       *inspection.manager_index);
  case materials::GGEMSMaterialRegistration::Unregistered:
    return "available, not registered";
  case materials::GGEMSMaterialRegistration::Unknown:
    break;
  }
  return {};
}

} // namespace

namespace ggems::core::materials {

// =============================================================================
// =============================================================================

[[nodiscard]] auto InspectMaterial(GGEMSMaterial const &material)
  -> GGEMSMaterialInspection {
  GGEMSMaterialInspection inspection{
    .name = std::string{material.GetName()},
    .density = material.GetDensity(),
    .registration = GGEMSMaterialRegistration::Unknown,
    .manager_index = std::nullopt,
    .elements = {},
    .isotopes = {},
    .total_atom_density_per_cubic_centimeter =
      material.GetTotalAtomDensityPerCubicCentimeter(),
    .electron_density_per_cubic_centimeter =
      material.GetElectronDensityPerCubicCentimeter(),
  };

  for (auto const &values : material.GetElementalConstituents()) {
    auto const &element = RequireElementByAtomicNumber(values.atomic_number);

    inspection.elements.push_back({
      .values = values,
      .symbol = std::string{element.GetSymbol()},
      .name = std::string{element.GetName()},
    });
  }

  auto const isotopes = material.GetIsotopeConstituents();
  inspection.isotopes.assign(isotopes.begin(), isotopes.end());

  return inspection;
}

// =============================================================================
// =============================================================================

[[nodiscard]] auto InspectMaterial(GGEMSMaterialManager const &manager,
                                   std::uint32_t manager_index)
  -> GGEMSMaterialInspection {
  auto inspection = InspectMaterial(manager.Require(manager_index));
  inspection.registration = GGEMSMaterialRegistration::Registered;
  inspection.manager_index = manager_index;
  return inspection;
}

// =============================================================================
// =============================================================================

[[nodiscard]] auto InspectMaterial(GGEMSMaterialManager const &manager,
                                   std::string_view name)
  -> GGEMSMaterialInspection {
  if (auto const manager_index = manager.FindIndex(name);
      manager_index.has_value()) {
    return InspectMaterial(manager, *manager_index);
  }

  auto const builtin_names = builtins::GetAvailableMaterialNames();

  if (std::ranges::find(builtin_names, name) != builtin_names.end()) {
    auto inspection = InspectMaterial(builtins::BuildBuiltInMaterial(name));
    inspection.registration = GGEMSMaterialRegistration::Unregistered;
    return inspection;
  }

  if (auto const *material = manager.FindCustom(name); material != nullptr) {
    auto inspection = InspectMaterial(*material);
    inspection.registration = GGEMSMaterialRegistration::Unregistered;
    return inspection;
  }

  return InspectMaterial(builtins::BuildBuiltInMaterial(name));
}

// =============================================================================
// =============================================================================

[[nodiscard]] auto DescribeMaterial(GGEMSMaterial const &material)
  -> std::string {
  return DescribeMaterial(InspectMaterial(material));
}

// =============================================================================
// =============================================================================

[[nodiscard]] auto DescribeMaterial(GGEMSMaterialInspection const &inspection)
  -> std::string {
  std::string description = std::format("Material: {}", inspection.name);

  if (auto const registration = RegistrationLabel(inspection);
      !registration.empty()) {
    description += std::format("\n  Registration     : {}", registration);
  }

  description += std::format("\n  Density          : {}",
                             units::HumanReadable(inspection.density));

  if (inspection.elements.empty()) {
    description += "\n  Elements         : none";
  } else {
    description +=
      std::format("\n  Elements         : {}", inspection.elements.size());
  }

  std::size_t isotope_index{0U};
  for (auto const &element : inspection.elements) {
    auto const &values = element.values;

    description += std::format(
      "\n    {} {} Z={}"
      "\n      derived mass fraction : {:.8g}"
      "\n      number density        : {:.8g} {}"
      "\n      electron density      : {:.8g} {}",
      element.symbol, element.name, values.atomic_number, values.mass_fraction,
      values.number_density_per_cubic_centimeter, NumberDensityUnit(),
      values.electron_density_per_cubic_centimeter, NumberDensityUnit());

    for (; isotope_index < inspection.isotopes.size() &&
           inspection.isotopes[isotope_index].isotope.GetAtomicNumber() ==
             values.atomic_number;
         ++isotope_index) {
      auto const &isotope = inspection.isotopes[isotope_index];

      description += std::format(
        "\n      {:<8} Z={} A={} M={} atom fraction in element {:.8g}, "
        "number density {:.8g} {}",
        IsotopeLabel(isotope.isotope), isotope.isotope.GetAtomicNumber(),
        isotope.isotope.GetMassNumber(), isotope.isotope.GetIsomerState(),
        isotope.atom_fraction_in_element,
        isotope.number_density_per_cubic_centimeter, NumberDensityUnit());
    }
  }

  description += std::format(
    "\n  Atom density     : {:.8g} {}"
    "\n  Electron density : {:.8g} {}",
    inspection.total_atom_density_per_cubic_centimeter, NumberDensityUnit(),
    inspection.electron_density_per_cubic_centimeter, NumberDensityUnit());

  return description;
}

// =============================================================================
// =============================================================================

auto VerboseMaterial(GGEMSMaterial const &material) -> void {
  GGEMS_INFO("Material", "\n{}\n", DescribeMaterial(material));
}

// =============================================================================
// =============================================================================

auto VerboseMaterial(GGEMSMaterialInspection const &inspection) -> void {
  GGEMS_INFO("Material", "\n{}\n", DescribeMaterial(inspection));
}

} // namespace ggems::core::materials
