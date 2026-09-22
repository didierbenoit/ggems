#include <algorithm>
#include <cmath>
#include <cstddef>
#include <optional>
#include <span>
#include <string>
#include <utility>
#include <vector>

#include "GGEMS/GGEMSException.hh"
#include "GGEMS/materials/GGEMSElementCatalog.hh"
#include "GGEMS/materials/GGEMSIsotopeMassAuthority.hh"
#include "GGEMS/materials/GGEMSIsotopeProfile.hh"
#include "GGEMS/materials/GGEMSMaterial.hh"
#include "GGEMS/materials/GGEMSMaterialComposition.hh"
#include "GGEMS/units/GGEMSDensityUnits.hh"
#include "GGEMS/units/GGEMSUnitConversion.hh"

namespace ggems::core::materials {

// =============================================================================
// =============================================================================

GGEMSMaterial::GGEMSMaterial(std::string name, units::Density density)
    : name_{std::move(name)}, density_{density} {
  if (name_.empty()) {
    throw GGEMSRecoverable{"Material name must not be empty."};
  }

  auto const density_grams_per_cubic_centimeter =
    units::ConvertTo(density_, "g/cm3");

  if (!density_grams_per_cubic_centimeter.has_value() ||
      !std::isfinite(*density_grams_per_cubic_centimeter) ||
      *density_grams_per_cubic_centimeter < 0.0L) {
    throw GGEMSRecoverable{"Material density must be finite and non-negative."};
  }
}

// =============================================================================
// =============================================================================

GGEMSMaterial::GGEMSMaterial(
  std::string name, units::Density density,
  std::vector<GGEMSMaterialComponent> const &composition)
    : GGEMSMaterial{std::move(name), density} {
  if (density_.value == 0.0L) {
    if (!composition.empty()) {
      throw GGEMSRecoverable{
        "Zero-density Material must have an empty composition."};
    }
    return;
  }

  if (composition.empty()) {
    throw GGEMSRecoverable{
      "Positive-density Material must have a composition."};
  }

  std::vector<GGEMSElementalShare> elemental_shares;
  elemental_shares.reserve(composition.size());

  for (auto const &component : composition) {
    static_cast<void>(RequireElementByAtomicNumber(component.atomic_number));

    if (!std::isfinite(component.mass_fraction) ||
        !(component.mass_fraction > 0.0L)) {
      throw GGEMSRecoverable{
        "Material mass fractions must be finite and strictly positive."};
    }

    elemental_shares.push_back({
      .mass_fraction = component.mass_fraction,
      .isotopic_composition = ResolveIsotopeProfile(
        SelectLegacyElementalIsotopeProfile(component.atomic_number),
        component.atomic_number),
    });
  }

  Compile(std::move(elemental_shares), true);
}

// =============================================================================
// =============================================================================

[[nodiscard]] auto GGEMSMaterial::FromIsotopicComposition(
  std::string name, units::Density density,
  std::vector<GGEMSElementalShare> elemental_shares) -> GGEMSMaterial {
  GGEMSMaterial material{std::move(name), density};

  if (material.density_.value == 0.0L) {
    if (!elemental_shares.empty()) {
      throw GGEMSRecoverable{
        "Zero-density Material must have an empty composition."};
    }
    return material;
  }

  material.Compile(std::move(elemental_shares), false);
  return material;
}

// =============================================================================
// =============================================================================

auto GGEMSMaterial::Compile(std::vector<GGEMSElementalShare> elemental_shares,
                            bool resolved_from_profiles) -> void {
  auto const &composition = composition_.emplace(
    density_, std::move(elemental_shares), GetIsotopeMassAuthority());

  auto const shares = composition.GetElementalShares();
  auto const elements = composition.GetElementalConstituents();

  constituents_.reserve(elements.size());

  for (std::size_t index = 0U; index < elements.size(); ++index) {
    auto const atomic_number = elements[index].atomic_number;

    std::optional<GGEMSIsotopeProfile> isotope_profile;
    if (resolved_from_profiles) {
      isotope_profile = SelectLegacyElementalIsotopeProfile(atomic_number);
    }

    constituents_.push_back({
      .atomic_number = atomic_number,
      .mass_fraction = shares[index].mass_fraction,
      .number_density_per_cubic_centimeter =
        elements[index].number_density_per_cubic_centimeter,
      .isotope_profile = isotope_profile,
    });
  }
}

// =============================================================================
// =============================================================================

[[nodiscard]] auto GGEMSMaterial::GetIsotopeConstituents() const noexcept
  -> std::span<GGEMSIsotopeConstituent const> {
  if (!composition_.has_value()) {
    return {};
  }

  return composition_->GetIsotopeConstituents();
}

// =============================================================================
// =============================================================================

[[nodiscard]] auto GGEMSMaterial::GetElementalConstituents() const noexcept
  -> std::span<GGEMSDerivedElementalConstituent const> {
  if (!composition_.has_value()) {
    return {};
  }

  return composition_->GetElementalConstituents();
}

// =============================================================================
// =============================================================================

[[nodiscard]] auto
GGEMSMaterial::GetTotalAtomDensityPerCubicCentimeter() const noexcept
  -> long double {
  if (!composition_.has_value()) {
    return 0.0L;
  }

  return composition_->GetTotalAtomDensityPerCubicCentimeter();
}

// =============================================================================
// =============================================================================

[[nodiscard]] auto
GGEMSMaterial::GetElectronDensityPerCubicCentimeter() const noexcept
  -> long double {
  if (!composition_.has_value()) {
    return 0.0L;
  }

  return composition_->GetElectronDensityPerCubicCentimeter();
}

// =============================================================================
// =============================================================================

[[nodiscard]] auto
HasSameScientificIdentity(GGEMSMaterial const &first,
                          GGEMSMaterial const &second) noexcept -> bool {
  return first.GetDensity().value == second.GetDensity().value &&
         std::ranges::equal(first.GetIsotopeConstituents(),
                            second.GetIsotopeConstituents());
}

} // namespace ggems::core::materials
