#include <algorithm>
#include <cmath>
#include <string>
#include <utility>
#include <vector>

#include "GGEMS/GGEMSException.hh"
#include "GGEMS/materials/GGEMSElementCatalog.hh"
#include "GGEMS/materials/GGEMSMaterial.hh"
#include "GGEMS/units/GGEMSDensityUnits.hh"
#include "GGEMS/units/GGEMSUnitConversion.hh"

namespace ggems::core::materials {

namespace {

constexpr long double k_mass_fraction_sum_tolerance{1.0e-5L};
constexpr long double k_avogadro_constant_per_mole{6.02214076e23L};

} // namespace

// =============================================================================
// =============================================================================

GGEMSMaterial::GGEMSMaterial(std::string name, units::Density density,
                             std::vector<GGEMSMaterialComponent> composition)
    : name_{std::move(name)}, density_{density} {
  if (name_.empty()) {
    throw GGEMSRecoverable{"Material name must not be empty."};
  }

  auto const density_grams_per_cubic_centimeter =
      units::ConvertTo(density_, "g/cm3");
  if (!density_grams_per_cubic_centimeter.has_value() ||
      !(*density_grams_per_cubic_centimeter > 0.0L)) {
    throw GGEMSRecoverable{
        "Material density must be finite and strictly positive."};
  }

  if (composition.empty()) {
    throw GGEMSRecoverable{"Material composition must not be empty."};
  }

  std::ranges::sort(composition, {}, &GGEMSMaterialComponent::atomic_number);

  auto const duplicate = std::ranges::adjacent_find(
      composition, {}, &GGEMSMaterialComponent::atomic_number);
  if (duplicate != composition.end()) {
    throw GGEMSRecoverable{
        "Material composition contains duplicate atomic numbers."};
  }

  long double mass_fraction_sum{0.0L};
  for (auto const &component : composition) {
    static_cast<void>(RequireElementByAtomicNumber(component.atomic_number));

    if (!std::isfinite(component.mass_fraction) ||
        !(component.mass_fraction > 0.0L)) {
      throw GGEMSRecoverable{
          "Material mass fractions must be finite and strictly positive."};
    }

    mass_fraction_sum += component.mass_fraction;
  }

  if (!std::isfinite(mass_fraction_sum) ||
      std::abs(mass_fraction_sum - 1.0L) > k_mass_fraction_sum_tolerance) {
    throw GGEMSRecoverable{
        "Material mass fractions must sum to one within 1.0e-5."};
  }

  constituents_.reserve(composition.size());

  for (auto const &component : composition) {
    auto const &element = RequireElementByAtomicNumber(component.atomic_number);

    long double const normalized_mass_fraction =
        component.mass_fraction / mass_fraction_sum;

    long double const number_density_per_cubic_centimeter =
        k_avogadro_constant_per_mole * *density_grams_per_cubic_centimeter *
        normalized_mass_fraction / element.GetMolarMass();

    long double const electron_density_per_cubic_centimeter =
        number_density_per_cubic_centimeter *
        static_cast<long double>(component.atomic_number);

    long double const next_total_atom_density =
        total_atom_density_per_cubic_centimeter_ +
        number_density_per_cubic_centimeter;

    long double const next_electron_density =
        electron_density_per_cubic_centimeter_ +
        electron_density_per_cubic_centimeter;

    if (!std::isfinite(number_density_per_cubic_centimeter) ||
        !(number_density_per_cubic_centimeter > 0.0L) ||
        !std::isfinite(next_total_atom_density) ||
        !std::isfinite(next_electron_density)) {
      throw GGEMSRecoverable{
          "Material number-density calculation is out of range."};
    }

    constituents_.push_back({.atomic_number = component.atomic_number,
                             .mass_fraction = normalized_mass_fraction,
                             .number_density_per_cubic_centimeter =
                                 number_density_per_cubic_centimeter});
    total_atom_density_per_cubic_centimeter_ = next_total_atom_density;
    electron_density_per_cubic_centimeter_ = next_electron_density;
  }
}

} // namespace ggems::core::materials
