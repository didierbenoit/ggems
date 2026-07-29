#include <cmath>
#include <cstddef>
#include <cstdint>
#include <optional>
#include <string>
#include <string_view>
#include <utility>
#include <vector>

#include "GGEMS/core/GGEMSException.hh"
#include "GGEMS/core/GGEMSMacros.hh"
#include "GGEMS/core/particles/GGEMSParticleTypes.hh"
#include "GGEMS/core/radioactivity/GGEMSBetaSpectrumBuilder.hh"
#include "GGEMS/core/radioactivity/GGEMSBetaTransition.hh"
#include "GGEMS/core/radioactivity/GGEMSRadionuclideScientificMetadata.hh"
#include "GGEMS/core/radioactivity/GGEMSRadionuclideProvenance.hh"

namespace ggems::core::radioactivity {
namespace {

// =============================================================================
// =============================================================================

[[nodiscard]] constexpr auto IsAsciiWhitespace(char character) noexcept
    -> bool {
  return character == ' ' || character == '\t' || character == '\n' ||
         character == '\r' || character == '\f' || character == '\v';
}

// =============================================================================
// =============================================================================

[[nodiscard]] auto HasVisibleText(std::string_view text) noexcept -> bool {
  for (char character : text) {
    if (!IsAsciiWhitespace(character)) {
      return true;
    }
  }

  return false;
}

// =============================================================================
// =============================================================================

[[nodiscard]] constexpr auto
IsKnownModelKind(GGEMSRadionuclideModelKind model_kind) noexcept -> bool {
  switch (model_kind) {
  case GGEMSRadionuclideModelKind::PhysicalEvaluated:
    return true;
  }

  return false;
}

// =============================================================================
// =============================================================================

[[nodiscard]] constexpr auto
IsKnownCompleteness(GGEMSRadionuclideCompleteness completeness) noexcept
    -> bool {
  switch (completeness) {
  case GGEMSRadionuclideCompleteness::EvaluatedSubset:
    return true;
  }

  return false;
}

// =============================================================================
// =============================================================================

[[nodiscard]] constexpr auto
IsKnownQualifier(GGEMSEvaluatedValueQualifier qualifier) noexcept -> bool {
  switch (qualifier) {
  case GGEMSEvaluatedValueQualifier::CentralValue:
  case GGEMSEvaluatedValueQualifier::Approximate:
  case GGEMSEvaluatedValueQualifier::UpperLimit:
  case GGEMSEvaluatedValueQualifier::Range:
  case GGEMSEvaluatedValueQualifier::Grouped:
  case GGEMSEvaluatedValueQualifier::NotSupplied:
    return true;
  }

  return false;
}

// =============================================================================
// =============================================================================

[[nodiscard]] constexpr auto
IsKnownQuantityUnit(GGEMSEvaluatedQuantityUnit unit) noexcept -> bool {
  switch (unit) {
  case GGEMSEvaluatedQuantityUnit::Dimensionless:
  case GGEMSEvaluatedQuantityUnit::Second:
  case GGEMSEvaluatedQuantityUnit::PerParentDecay:
  case GGEMSEvaluatedQuantityUnit::MilliElectronVolt:
    return true;
  }

  return false;
}

// =============================================================================
// =============================================================================

[[nodiscard]] constexpr auto
IsKnownDisposition(GGEMSEvaluatedEmissionDisposition disposition) noexcept
    -> bool {
  switch (disposition) {
  case GGEMSEvaluatedEmissionDisposition::Included:
  case GGEMSEvaluatedEmissionDisposition::Deferred:
  case GGEMSEvaluatedEmissionDisposition::ExcludedGeneratedByTransport:
  case GGEMSEvaluatedEmissionDisposition::NoDirectTransportedParticle:
    return true;
  }

  return false;
}

// =============================================================================
// =============================================================================

[[nodiscard]] constexpr auto
IsPhysicalParticleType(particles::GGEMSParticleType particle_type) noexcept
    -> bool {
  switch (particle_type) {
  case particles::GGEMSParticleType::Gamma:
  case particles::GGEMSParticleType::Electron:
  case particles::GGEMSParticleType::Positron:
  case particles::GGEMSParticleType::Proton:
  case particles::GGEMSParticleType::Neutron:
  case particles::GGEMSParticleType::Alpha:
    return true;
  case particles::GGEMSParticleType::Unknown:
  case particles::GGEMSParticleType::Aionino:
    return false;
  }

  return false;
}

// =============================================================================
// =============================================================================

[[nodiscard]] constexpr auto
IsKnownBetaModel(GGEMSBetaSpectrumModel model) noexcept -> bool {
  switch (model) {
  case GGEMSBetaSpectrumModel::BarePhaseSpaceDiagnostic:
  case GGEMSBetaSpectrumModel::AllowedPointCoulomb:
    return true;
  }

  return false;
}

// =============================================================================
// =============================================================================

auto ValidateFiniteOptional(std::optional<long double> value,
                            char const *message) -> void {
  GGEMS_CHECK_RECOVERABLE(!value.has_value() || std::isfinite(*value), message);
}

// =============================================================================
// =============================================================================

auto ValidateTypedQuantityValue(GGEMSEvaluatedQuantityUnit unit,
                                long double value) -> void {
  switch (unit) {
  case GGEMSEvaluatedQuantityUnit::Dimensionless:
    return;
  case GGEMSEvaluatedQuantityUnit::Second:
  case GGEMSEvaluatedQuantityUnit::MilliElectronVolt:
    GGEMS_CHECK_RECOVERABLE(
        value > 0.0L,
        "Evaluated positive physical quantity must be strictly positive.");
    return;
  case GGEMSEvaluatedQuantityUnit::PerParentDecay:
    GGEMS_CHECK_RECOVERABLE(
        value >= 0.0L,
        "Evaluated per-parent-decay quantity must be non-negative.");
    return;
  }

  GGEMS_RECOVERABLE("Evaluated quantity unit is not recognized.");
}

} // namespace

// =============================================================================
// =============================================================================

GGEMSNuclideIdentity::GGEMSNuclideIdentity(std::uint32_t atomic_number,
                                           std::uint32_t mass_number,
                                           std::string display_name,
                                           bool stable)
    : atomic_number_{atomic_number}, mass_number_{mass_number},
      display_name_{std::move(display_name)}, stable_{stable} {
  GGEMS_CHECK_RECOVERABLE(atomic_number_ > 0U,
                          "Nuclide atomic number must be strictly positive.");
  GGEMS_CHECK_RECOVERABLE(mass_number_ > 0U,
                          "Nuclide mass number must be strictly positive.");
  GGEMS_CHECK_RECOVERABLE(
      atomic_number_ <= mass_number_,
      "Nuclide atomic number must not exceed its mass number.");
  GGEMS_CHECK_RECOVERABLE(
      HasVisibleText(display_name_),
      "Nuclide display name must contain non-whitespace text.");
}

// =============================================================================
// =============================================================================

GGEMSEvaluatedQuantity::GGEMSEvaluatedQuantity(
    std::string label, std::string original_printed_value,
    GGEMSEvaluatedQuantityUnit unit, GGEMSEvaluatedValueQualifier qualifier,
    std::optional<long double> central_value,
    std::optional<long double> standard_uncertainty,
    std::optional<long double> range_lower,
    std::optional<long double> range_upper)
    : label_{std::move(label)},
      original_printed_value_{std::move(original_printed_value)}, unit_{unit},
      qualifier_{qualifier}, central_value_{central_value},
      standard_uncertainty_{standard_uncertainty}, range_lower_{range_lower},
      range_upper_{range_upper} {
  GGEMS_CHECK_RECOVERABLE(
      HasVisibleText(label_),
      "Evaluated quantity label must contain non-whitespace text.");
  GGEMS_CHECK_RECOVERABLE(
      HasVisibleText(original_printed_value_),
      "Evaluated quantity printed value must contain non-whitespace text.");
  GGEMS_CHECK_RECOVERABLE(IsKnownQuantityUnit(unit_),
                          "Evaluated quantity unit is not recognized.");
  GGEMS_CHECK_RECOVERABLE(IsKnownQualifier(qualifier_),
                          "Evaluated quantity qualifier is not recognized.");

  ValidateFiniteOptional(central_value_,
                         "Evaluated quantity central value must be finite.");
  ValidateFiniteOptional(
      standard_uncertainty_,
      "Evaluated quantity standard uncertainty must be finite.");
  ValidateFiniteOptional(
      range_lower_, "Evaluated quantity range lower bound must be finite.");
  ValidateFiniteOptional(
      range_upper_, "Evaluated quantity range upper bound must be finite.");

  GGEMS_CHECK_RECOVERABLE(
      !standard_uncertainty_.has_value() || *standard_uncertainty_ >= 0.0L,
      "Evaluated quantity standard uncertainty must be non-negative.");

  if (qualifier_ == GGEMSEvaluatedValueQualifier::Range) {
    GGEMS_CHECK_RECOVERABLE(
        range_lower_.has_value() && range_upper_.has_value(),
        "A range-qualified evaluated quantity requires both range bounds.");
    GGEMS_CHECK_RECOVERABLE(
        !central_value_.has_value() && !standard_uncertainty_.has_value(),
        "A range-qualified evaluated quantity must not provide a central "
        "value or standard uncertainty.");
    GGEMS_CHECK_RECOVERABLE(
        *range_lower_ < *range_upper_,
        "Evaluated quantity range lower bound must be below its upper bound.");
    ValidateTypedQuantityValue(unit_, *range_lower_);
    ValidateTypedQuantityValue(unit_, *range_upper_);
    return;
  }

  GGEMS_CHECK_RECOVERABLE(
      !range_lower_.has_value() && !range_upper_.has_value(),
      "A non-range evaluated quantity must not provide range bounds.");

  if (qualifier_ == GGEMSEvaluatedValueQualifier::NotSupplied) {
    GGEMS_CHECK_RECOVERABLE(
        !central_value_.has_value() && !standard_uncertainty_.has_value(),
        "A not-supplied evaluated quantity must not provide a numeric value.");
    return;
  }

  GGEMS_CHECK_RECOVERABLE(
      central_value_.has_value(),
      "Evaluated quantity qualifier requires a central numeric value.");
  ValidateTypedQuantityValue(unit_, *central_value_);
}

// =============================================================================
// =============================================================================

GGEMSEvaluatedEnergy::GGEMSEvaluatedEnergy(
    std::string label, std::string original_printed_value,
    GGEMSEvaluatedValueQualifier qualifier,
    std::optional<std::uint64_t> central_milli_eV,
    std::optional<std::uint64_t> standard_uncertainty_milli_eV,
    std::optional<std::uint64_t> range_lower_milli_eV,
    std::optional<std::uint64_t> range_upper_milli_eV)
    : label_{std::move(label)},
      original_printed_value_{std::move(original_printed_value)},
      qualifier_{qualifier}, central_milli_eV_{central_milli_eV},
      standard_uncertainty_milli_eV_{standard_uncertainty_milli_eV},
      range_lower_milli_eV_{range_lower_milli_eV},
      range_upper_milli_eV_{range_upper_milli_eV} {
  GGEMS_CHECK_RECOVERABLE(
      HasVisibleText(label_),
      "Evaluated energy label must contain non-whitespace text.");
  GGEMS_CHECK_RECOVERABLE(
      HasVisibleText(original_printed_value_),
      "Evaluated energy printed value must contain non-whitespace text.");
  GGEMS_CHECK_RECOVERABLE(IsKnownQualifier(qualifier_),
                          "Evaluated energy qualifier is not recognized.");

  if (qualifier_ == GGEMSEvaluatedValueQualifier::Range) {
    GGEMS_CHECK_RECOVERABLE(
        range_lower_milli_eV_.has_value() && range_upper_milli_eV_.has_value(),
        "A range-qualified evaluated energy requires both range bounds.");
    GGEMS_CHECK_RECOVERABLE(
        !central_milli_eV_.has_value() &&
            !standard_uncertainty_milli_eV_.has_value(),
        "A range-qualified evaluated energy must not provide a central value "
        "or standard uncertainty.");
    GGEMS_CHECK_RECOVERABLE(
        *range_lower_milli_eV_ > 0ULL &&
            *range_lower_milli_eV_ < *range_upper_milli_eV_,
        "Evaluated energy range must have positive ordered bounds.");
    return;
  }

  GGEMS_CHECK_RECOVERABLE(
      !range_lower_milli_eV_.has_value() && !range_upper_milli_eV_.has_value(),
      "A non-range evaluated energy must not provide range bounds.");

  if (qualifier_ == GGEMSEvaluatedValueQualifier::NotSupplied) {
    GGEMS_CHECK_RECOVERABLE(
        !central_milli_eV_.has_value() &&
            !standard_uncertainty_milli_eV_.has_value(),
        "A not-supplied evaluated energy must not provide a numeric value.");
    return;
  }

  GGEMS_CHECK_RECOVERABLE(
      central_milli_eV_.has_value() && *central_milli_eV_ > 0ULL,
      "Evaluated energy central value must be strictly positive.");
}

// =============================================================================
// =============================================================================

GGEMSRadionuclideEvaluationSource::GGEMSRadionuclideEvaluationSource(
    GGEMSRadionuclideProvenance provenance, std::vector<std::string> evaluators,
    std::string source_document_identifier, std::string source_location,
    std::string source_snapshot_identifier,
    std::optional<std::string> scheme_date)
    : provenance_{std::move(provenance)}, evaluators_{std::move(evaluators)},
      source_document_identifier_{std::move(source_document_identifier)},
      source_location_{std::move(source_location)},
      source_snapshot_identifier_{std::move(source_snapshot_identifier)},
      scheme_date_{std::move(scheme_date)} {
  GGEMS_CHECK_RECOVERABLE(
      !evaluators_.empty(),
      "Radionuclide evaluation source requires at least one evaluator.");

  for (std::string const &evaluator : evaluators_) {
    GGEMS_CHECK_RECOVERABLE(
        HasVisibleText(evaluator),
        "Radionuclide evaluator must contain non-whitespace text.");
  }

  GGEMS_CHECK_RECOVERABLE(
      HasVisibleText(source_document_identifier_),
      "Evaluation source document identifier must contain non-whitespace "
      "text.");
  GGEMS_CHECK_RECOVERABLE(
      HasVisibleText(source_location_),
      "Evaluation source location must contain non-whitespace text.");
  GGEMS_CHECK_RECOVERABLE(
      HasVisibleText(source_snapshot_identifier_),
      "Evaluation source snapshot identifier must contain non-whitespace "
      "text.");

  if (scheme_date_.has_value()) {
    GGEMS_CHECK_RECOVERABLE(
        HasVisibleText(*scheme_date_),
        "Evaluation source scheme date must contain non-whitespace text when "
        "present.");
  }
}

// =============================================================================
// =============================================================================

GGEMSBetaApproximationMetadata::GGEMSBetaApproximationMetadata(
    std::size_t included_channel_index, GGEMSBetaTransition transition,
    GGEMSBetaSpectrumBuildOptions options,
    GGEMSBetaSpectrumDiagnostics diagnostics, std::string reference_citation,
    std::string constants_reference_citation, std::string non_exact_statement)
    : included_channel_index_{included_channel_index},
      transition_{std::move(transition)}, options_{options},
      diagnostics_{diagnostics},
      reference_citation_{std::move(reference_citation)},
      constants_reference_citation_{std::move(constants_reference_citation)},
      non_exact_statement_{std::move(non_exact_statement)} {
  GGEMS_CHECK_RECOVERABLE(IsKnownBetaModel(options_.model),
                          "Beta approximation model is not recognized.");
  GGEMS_CHECK_RECOVERABLE(IsKnownBetaModel(diagnostics_.model),
                          "Beta diagnostics model is not recognized.");
  GGEMS_CHECK_RECOVERABLE(
      options_.model == diagnostics_.model,
      "Beta approximation options and diagnostics models must match.");
  GGEMS_CHECK_RECOVERABLE(
      options_.grid.target_maximum_bin_width_milli_eV > 0ULL,
      "Beta approximation target maximum bin width must be strictly positive.");
  GGEMS_CHECK_RECOVERABLE(
      diagnostics_.lower_edge_milli_eV < diagnostics_.upper_edge_milli_eV &&
          diagnostics_.bin_width_milli_eV > 0ULL && diagnostics_.bin_count > 0U,
      "Beta approximation diagnostics require a valid non-empty grid.");
  GGEMS_CHECK_RECOVERABLE(
      diagnostics_.upper_edge_milli_eV ==
          transition_.GetEndpointKineticEnergyMilliElectronVolt(),
      "Beta approximation diagnostics endpoint must match the transition.");
  GGEMS_CHECK_RECOVERABLE(
      diagnostics_.bin_width_milli_eV <=
          options_.grid.target_maximum_bin_width_milli_eV,
      "Beta approximation diagnostic bin width exceeds the target maximum.");

  GGEMS_CHECK_RECOVERABLE(
      std::isfinite(diagnostics_.full_unnormalized_integral) &&
          diagnostics_.full_unnormalized_integral > 0.0L &&
          std::isfinite(diagnostics_.represented_unnormalized_integral) &&
          diagnostics_.represented_unnormalized_integral > 0.0L &&
          std::isfinite(diagnostics_.excluded_probability) &&
          diagnostics_.excluded_probability >= 0.0L &&
          diagnostics_.excluded_probability < 1.0L &&
          std::isfinite(diagnostics_.continuous_mean_energy_milli_eV) &&
          diagnostics_.continuous_mean_energy_milli_eV > 0.0L &&
          std::isfinite(diagnostics_.represented_mean_energy_milli_eV) &&
          diagnostics_.represented_mean_energy_milli_eV > 0.0L &&
          std::isfinite(diagnostics_.normalization_residual) &&
          std::isfinite(diagnostics_.minimum_positive_bin_probability) &&
          diagnostics_.minimum_positive_bin_probability > 0.0L &&
          diagnostics_.minimum_assigned_ticket_count > 0ULL,
      "Beta approximation diagnostics contain invalid numeric values.");

  GGEMS_CHECK_RECOVERABLE(
      HasVisibleText(reference_citation_),
      "Beta approximation reference citation must contain non-whitespace "
      "text.");
  GGEMS_CHECK_RECOVERABLE(
      HasVisibleText(constants_reference_citation_),
      "Beta approximation constants citation must contain non-whitespace "
      "text.");
  GGEMS_CHECK_RECOVERABLE(
      HasVisibleText(non_exact_statement_),
      "Beta approximation scope statement must contain non-whitespace text.");
}

// =============================================================================
// =============================================================================

GGEMSEvaluatedEmissionMetadata::GGEMSEvaluatedEmissionMetadata(
    std::string label,
    std::optional<particles::GGEMSParticleType> particle_type,
    GGEMSEvaluatedQuantity yield, std::optional<GGEMSEvaluatedEnergy> energy,
    GGEMSEvaluatedEmissionDisposition disposition,
    std::optional<std::size_t> included_channel_index, std::string reason)
    : label_{std::move(label)}, particle_type_{particle_type},
      yield_{std::move(yield)}, energy_{std::move(energy)},
      disposition_{disposition},
      included_channel_index_{included_channel_index},
      reason_{std::move(reason)} {
  GGEMS_CHECK_RECOVERABLE(
      HasVisibleText(label_),
      "Evaluated emission label must contain non-whitespace text.");
  GGEMS_CHECK_RECOVERABLE(IsKnownDisposition(disposition_),
                          "Evaluated emission disposition is not recognized.");
  GGEMS_CHECK_RECOVERABLE(
      yield_.GetUnit() == GGEMSEvaluatedQuantityUnit::PerParentDecay,
      "Evaluated emission yield must use per-parent-decay units.");
  GGEMS_CHECK_RECOVERABLE(
      yield_.GetCentralValue().has_value() && *yield_.GetCentralValue() > 0.0L,
      "Evaluated emission yield must be strictly positive.");

  if (particle_type_.has_value()) {
    GGEMS_CHECK_RECOVERABLE(
        IsPhysicalParticleType(*particle_type_),
        "Evaluated emission particle type must be a physical particle.");
  }

  switch (disposition_) {
  case GGEMSEvaluatedEmissionDisposition::Included:
    GGEMS_CHECK_RECOVERABLE(
        particle_type_.has_value() && energy_.has_value() &&
            included_channel_index_.has_value(),
        "An included evaluated emission requires particle, energy, and "
        "channel metadata.");
    return;
  case GGEMSEvaluatedEmissionDisposition::Deferred:
  case GGEMSEvaluatedEmissionDisposition::ExcludedGeneratedByTransport:
    GGEMS_CHECK_RECOVERABLE(
        particle_type_.has_value() && energy_.has_value(),
        "A deferred or transport-generated evaluated emission requires "
        "particle and energy metadata.");
    GGEMS_CHECK_RECOVERABLE(
        !included_channel_index_.has_value(),
        "A non-included evaluated emission must not link to a channel.");
    GGEMS_CHECK_RECOVERABLE(
        HasVisibleText(reason_),
        "A deferred or transport-generated evaluated emission requires a "
        "reason.");
    return;
  case GGEMSEvaluatedEmissionDisposition::NoDirectTransportedParticle:
    GGEMS_CHECK_RECOVERABLE(
        !particle_type_.has_value() && !energy_.has_value() &&
            !included_channel_index_.has_value(),
        "A no-direct-particle evaluated record must not provide particle, "
        "energy, or channel metadata.");
    GGEMS_CHECK_RECOVERABLE(
        HasVisibleText(reason_),
        "A no-direct-particle evaluated record requires a reason.");
    return;
  }

  GGEMS_RECOVERABLE("Evaluated emission disposition is not recognized.");
}

// =============================================================================
// =============================================================================

GGEMSRadionuclideScientificMetadata::GGEMSRadionuclideScientificMetadata(
    GGEMSRadionuclideModelKind model_kind,
    GGEMSRadionuclideCompleteness completeness, GGEMSNuclideIdentity parent,
    GGEMSNuclideIdentity daughter,
    GGEMSRadionuclideEvaluationSource evaluation_source,
    std::vector<GGEMSEvaluatedQuantity> evaluated_quantities,
    std::vector<GGEMSEvaluatedEmissionMetadata> evaluated_emissions,
    std::vector<GGEMSBetaApproximationMetadata> beta_approximations)
    : model_kind_{model_kind}, completeness_{completeness},
      parent_{std::move(parent)}, daughter_{std::move(daughter)},
      evaluation_source_{std::move(evaluation_source)},
      evaluated_quantities_{std::move(evaluated_quantities)},
      evaluated_emissions_{std::move(evaluated_emissions)},
      beta_approximations_{std::move(beta_approximations)} {
  GGEMS_CHECK_RECOVERABLE(IsKnownModelKind(model_kind_),
                          "Radionuclide model kind is not recognized.");
  GGEMS_CHECK_RECOVERABLE(IsKnownCompleteness(completeness_),
                          "Radionuclide completeness is not recognized.");
  GGEMS_CHECK_RECOVERABLE(
      !evaluated_emissions_.empty(),
      "Scientific radionuclide metadata requires evaluated emission records.");
}

// =============================================================================
// =============================================================================

auto GGEMSRadionuclideScientificMetadata::ValidateChannelLinks(
    std::size_t channel_count) const -> void {
  std::vector<bool> linked_channels(channel_count, false);

  for (GGEMSEvaluatedEmissionMetadata const &emission : evaluated_emissions_) {
    if (emission.GetDisposition() !=
        GGEMSEvaluatedEmissionDisposition::Included) {
      continue;
    }

    std::size_t const channel_index = *emission.GetIncludedChannelIndex();
    GGEMS_CHECK_RECOVERABLE(
        channel_index < channel_count,
        "Included evaluated emission channel index is out of bounds.");
    GGEMS_CHECK_RECOVERABLE(
        !linked_channels[channel_index],
        "Multiple evaluated emissions link to the same included channel.");
    linked_channels[channel_index] = true;
  }

  for (bool linked : linked_channels) {
    GGEMS_CHECK_RECOVERABLE(
        linked,
        "Every runtime emission channel requires one included metadata link.");
  }

  std::vector<bool> approximation_channels(channel_count, false);
  for (GGEMSBetaApproximationMetadata const &approximation :
       beta_approximations_) {
    std::size_t const channel_index = approximation.GetIncludedChannelIndex();
    GGEMS_CHECK_RECOVERABLE(
        channel_index < channel_count && linked_channels[channel_index],
        "Beta approximation included channel index is out of bounds.");
    GGEMS_CHECK_RECOVERABLE(
        !approximation_channels[channel_index],
        "Multiple beta approximations link to the same included channel.");
    approximation_channels[channel_index] = true;
  }
}

} // namespace ggems::core::radioactivity
