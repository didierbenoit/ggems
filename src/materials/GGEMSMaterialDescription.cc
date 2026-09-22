#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <format>
#include <optional>
#include <string>
#include <string_view>

#include "GGEMS/logging/GGEMSLogger.hh"
#include "GGEMS/logging/GGEMSLogMacros.hh"
#include "GGEMS/materials/GGEMSElementCatalog.hh"
#include "GGEMS/materials/GGEMSIsotope.hh"
#include "GGEMS/materials/GGEMSIsotopeProfile.hh"
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

[[nodiscard]] auto
IsotopeProfileName(std::optional<materials::GGEMSIsotopeProfile> profile)
    -> std::string_view {
  if (!profile.has_value()) {
    return "not retained";
  }

  switch (*profile) {
  case materials::GGEMSIsotopeProfile::Nist41Natural:
    return "NIST 4.1 representative natural composition";
  case materials::GGEMSIsotopeProfile::LegacyReferenceIsotope:
    return "legacy reference isotope (not a natural composition)";
  }
  return "unknown";
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
                       inspection.manager_index.value_or(0U));
  case materials::GGEMSMaterialRegistration::Unregistered:
    return "available built-in, not registered";
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

  auto const authored = material.GetConstituents();

  for (auto const &values : material.GetElementalConstituents()) {
    auto const &element = RequireElementByAtomicNumber(values.atomic_number);

    auto const retained =
        std::ranges::find(authored, values.atomic_number,
                          &GGEMSMaterialConstituent::atomic_number);

    inspection.elements.push_back({
        .values = values,
        .symbol = std::string{element.GetSymbol()},
        .name = std::string{element.GetName()},
        .isotope_profile = retained != authored.end()
                               ? retained->isotope_profile
                               : std::nullopt,
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

  auto inspection = InspectMaterial(builtins::BuildBuiltInMaterial(name));
  inspection.registration = GGEMSMaterialRegistration::Unregistered;
  return inspection;
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
        "\n      electron density      : {:.8g} {}"
        "\n      isotope profile       : {}",
        element.symbol, element.name, values.atomic_number,
        values.mass_fraction, values.number_density_per_cubic_centimeter,
        NumberDensityUnit(), values.electron_density_per_cubic_centimeter,
        NumberDensityUnit(), IsotopeProfileName(element.isotope_profile));

    // Isotope rows follow the canonical element order.
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

[[nodiscard]] auto
DescribeRegisteredMaterials(GGEMSMaterialManager const &manager)
    -> std::string {
  auto const registered = manager.GetMaterials();

  std::string description =
      std::format("Registered Materials: {}", registered.size());

  for (std::size_t index = 0U; index < registered.size(); ++index) {
    description += std::format("\n  manager material index {}: {}", index,
                               registered[index].GetName());
  }

  return description;
}

// =============================================================================
// =============================================================================

auto VerboseMaterial(GGEMSMaterial const &material) -> void {
  GGEMS_INFO("Material", "{}", DescribeMaterial(material));
}

// =============================================================================
// =============================================================================

auto VerboseMaterial(GGEMSMaterialInspection const &inspection) -> void {
  GGEMS_INFO("Material", "{}", DescribeMaterial(inspection));
}

// =============================================================================
// =============================================================================

auto VerboseAvailableMaterials() -> void {
  GGEMS_INFO("Material", "{}", DescribeAvailableMaterials());
}

// =============================================================================
// =============================================================================

auto VerboseRegisteredMaterials(GGEMSMaterialManager const &manager) -> void {
  GGEMS_INFO("Material", "{}", DescribeRegisteredMaterials(manager));
}

} // namespace ggems::core::materials
