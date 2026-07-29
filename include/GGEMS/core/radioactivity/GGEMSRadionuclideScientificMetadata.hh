#pragma once

#include <cstddef>
#include <cstdint>
#include <optional>
#include <span>
#include <string>
#include <string_view>
#include <vector>

#include "GGEMS/core/particles/GGEMSParticleTypes.hh"
#include "GGEMS/core/radioactivity/GGEMSBetaSpectrumBuilder.hh"
#include "GGEMS/core/radioactivity/GGEMSBetaTransition.hh"
#include "GGEMS/core/radioactivity/GGEMSRadionuclideProvenance.hh"

namespace ggems::core::radioactivity {

enum class GGEMSRadionuclideModelKind : std::uint8_t { PhysicalEvaluated };

enum class GGEMSRadionuclideCompleteness : std::uint8_t { EvaluatedSubset };

enum class GGEMSEvaluatedValueQualifier : std::uint8_t {
  CentralValue,
  Approximate,
  UpperLimit,
  Range,
  Grouped,
  NotSupplied
};

enum class GGEMSEvaluatedQuantityUnit : std::uint8_t {
  Dimensionless,
  Second,
  PerParentDecay,
  MilliElectronVolt
};

enum class GGEMSEvaluatedEmissionDisposition : std::uint8_t {
  Included,
  Deferred,
  ExcludedGeneratedByTransport,
  NoDirectTransportedParticle
};

class GGEMSNuclideIdentity {
public:
  GGEMSNuclideIdentity(std::uint32_t atomic_number, std::uint32_t mass_number,
                       std::string display_name, bool stable);

  [[nodiscard]] auto GetAtomicNumber() const noexcept -> std::uint32_t {
    return atomic_number_;
  }

  [[nodiscard]] auto GetMassNumber() const noexcept -> std::uint32_t {
    return mass_number_;
  }

  [[nodiscard]] auto GetDisplayName() const noexcept -> std::string_view {
    return display_name_;
  }

  [[nodiscard]] auto IsStable() const noexcept -> bool { return stable_; }

private:
  std::uint32_t atomic_number_;
  std::uint32_t mass_number_;
  std::string display_name_;
  bool stable_;
};

class GGEMSEvaluatedQuantity {
public:
  GGEMSEvaluatedQuantity(
      std::string label, std::string original_printed_value,
      GGEMSEvaluatedQuantityUnit unit, GGEMSEvaluatedValueQualifier qualifier,
      std::optional<long double> central_value = std::nullopt,
      std::optional<long double> standard_uncertainty = std::nullopt,
      std::optional<long double> range_lower = std::nullopt,
      std::optional<long double> range_upper = std::nullopt);

  [[nodiscard]] auto GetLabel() const noexcept -> std::string_view {
    return label_;
  }

  [[nodiscard]] auto GetOriginalPrintedValue() const noexcept
      -> std::string_view {
    return original_printed_value_;
  }

  [[nodiscard]] auto GetUnit() const noexcept -> GGEMSEvaluatedQuantityUnit {
    return unit_;
  }

  [[nodiscard]] auto GetQualifier() const noexcept
      -> GGEMSEvaluatedValueQualifier {
    return qualifier_;
  }

  [[nodiscard]] auto GetCentralValue() const noexcept
      -> std::optional<long double> {
    return central_value_;
  }

  [[nodiscard]] auto GetStandardUncertainty() const noexcept
      -> std::optional<long double> {
    return standard_uncertainty_;
  }

  [[nodiscard]] auto GetRangeLower() const noexcept
      -> std::optional<long double> {
    return range_lower_;
  }

  [[nodiscard]] auto GetRangeUpper() const noexcept
      -> std::optional<long double> {
    return range_upper_;
  }

private:
  std::string label_;
  std::string original_printed_value_;
  GGEMSEvaluatedQuantityUnit unit_;
  GGEMSEvaluatedValueQualifier qualifier_;
  std::optional<long double> central_value_;
  std::optional<long double> standard_uncertainty_;
  std::optional<long double> range_lower_;
  std::optional<long double> range_upper_;
};

class GGEMSEvaluatedEnergy {
public:
  GGEMSEvaluatedEnergy(
      std::string label, std::string original_printed_value,
      GGEMSEvaluatedValueQualifier qualifier,
      std::optional<std::uint64_t> central_milli_eV = std::nullopt,
      std::optional<std::uint64_t> standard_uncertainty_milli_eV = std::nullopt,
      std::optional<std::uint64_t> range_lower_milli_eV = std::nullopt,
      std::optional<std::uint64_t> range_upper_milli_eV = std::nullopt);

  [[nodiscard]] auto GetLabel() const noexcept -> std::string_view {
    return label_;
  }

  [[nodiscard]] auto GetOriginalPrintedValue() const noexcept
      -> std::string_view {
    return original_printed_value_;
  }

  [[nodiscard]] auto GetQualifier() const noexcept
      -> GGEMSEvaluatedValueQualifier {
    return qualifier_;
  }

  [[nodiscard]] auto GetCentralMilliElectronVolt() const noexcept
      -> std::optional<std::uint64_t> {
    return central_milli_eV_;
  }

  [[nodiscard]] auto GetStandardUncertaintyMilliElectronVolt() const noexcept
      -> std::optional<std::uint64_t> {
    return standard_uncertainty_milli_eV_;
  }

  [[nodiscard]] auto GetRangeLowerMilliElectronVolt() const noexcept
      -> std::optional<std::uint64_t> {
    return range_lower_milli_eV_;
  }

  [[nodiscard]] auto GetRangeUpperMilliElectronVolt() const noexcept
      -> std::optional<std::uint64_t> {
    return range_upper_milli_eV_;
  }

private:
  std::string label_;
  std::string original_printed_value_;
  GGEMSEvaluatedValueQualifier qualifier_;
  std::optional<std::uint64_t> central_milli_eV_;
  std::optional<std::uint64_t> standard_uncertainty_milli_eV_;
  std::optional<std::uint64_t> range_lower_milli_eV_;
  std::optional<std::uint64_t> range_upper_milli_eV_;
};

class GGEMSRadionuclideEvaluationSource {
public:
  GGEMSRadionuclideEvaluationSource(
      GGEMSRadionuclideProvenance provenance,
      std::vector<std::string> evaluators,
      std::string source_document_identifier, std::string source_location,
      std::string source_snapshot_identifier,
      std::optional<std::string> scheme_date = std::nullopt);

  [[nodiscard]] auto GetProvenance() const noexcept
      -> GGEMSRadionuclideProvenance const & {
    return provenance_;
  }

  [[nodiscard]] auto GetEvaluators() const noexcept
      -> std::span<std::string const> {
    return evaluators_;
  }

  [[nodiscard]] auto GetSourceDocumentIdentifier() const noexcept
      -> std::string_view {
    return source_document_identifier_;
  }

  [[nodiscard]] auto GetSourceLocation() const noexcept -> std::string_view {
    return source_location_;
  }

  [[nodiscard]] auto GetSourceSnapshotIdentifier() const noexcept
      -> std::string_view {
    return source_snapshot_identifier_;
  }

  [[nodiscard]] auto GetSchemeDate() const noexcept
      -> std::optional<std::string> const & {
    return scheme_date_;
  }

private:
  GGEMSRadionuclideProvenance provenance_;
  std::vector<std::string> evaluators_;
  std::string source_document_identifier_;
  std::string source_location_;
  std::string source_snapshot_identifier_;
  std::optional<std::string> scheme_date_;
};

class GGEMSBetaApproximationMetadata {
public:
  GGEMSBetaApproximationMetadata(std::size_t included_channel_index,
                                 GGEMSBetaTransition transition,
                                 GGEMSBetaSpectrumBuildOptions options,
                                 GGEMSBetaSpectrumDiagnostics diagnostics,
                                 std::string reference_citation,
                                 std::string constants_reference_citation,
                                 std::string non_exact_statement);

  [[nodiscard]] auto GetIncludedChannelIndex() const noexcept -> std::size_t {
    return included_channel_index_;
  }

  [[nodiscard]] auto GetTransition() const noexcept
      -> GGEMSBetaTransition const & {
    return transition_;
  }

  [[nodiscard]] auto GetOptions() const noexcept
      -> GGEMSBetaSpectrumBuildOptions const & {
    return options_;
  }

  [[nodiscard]] auto GetDiagnostics() const noexcept
      -> GGEMSBetaSpectrumDiagnostics const & {
    return diagnostics_;
  }

  [[nodiscard]] auto GetReferenceCitation() const noexcept -> std::string_view {
    return reference_citation_;
  }

  [[nodiscard]] auto GetConstantsReferenceCitation() const noexcept
      -> std::string_view {
    return constants_reference_citation_;
  }

  [[nodiscard]] auto GetNonExactStatement() const noexcept -> std::string_view {
    return non_exact_statement_;
  }

private:
  std::size_t included_channel_index_;
  GGEMSBetaTransition transition_;
  GGEMSBetaSpectrumBuildOptions options_;
  GGEMSBetaSpectrumDiagnostics diagnostics_;
  std::string reference_citation_;
  std::string constants_reference_citation_;
  std::string non_exact_statement_;
};

class GGEMSEvaluatedEmissionMetadata {
public:
  GGEMSEvaluatedEmissionMetadata(
      std::string label,
      std::optional<particles::GGEMSParticleType> particle_type,
      GGEMSEvaluatedQuantity yield, std::optional<GGEMSEvaluatedEnergy> energy,
      GGEMSEvaluatedEmissionDisposition disposition,
      std::optional<std::size_t> included_channel_index = std::nullopt,
      std::string reason = {});

  [[nodiscard]] auto GetLabel() const noexcept -> std::string_view {
    return label_;
  }

  [[nodiscard]] auto GetParticleType() const noexcept
      -> std::optional<particles::GGEMSParticleType> {
    return particle_type_;
  }

  [[nodiscard]] auto GetYield() const noexcept
      -> GGEMSEvaluatedQuantity const & {
    return yield_;
  }

  [[nodiscard]] auto GetEnergy() const noexcept
      -> std::optional<GGEMSEvaluatedEnergy> const & {
    return energy_;
  }

  [[nodiscard]] auto GetDisposition() const noexcept
      -> GGEMSEvaluatedEmissionDisposition {
    return disposition_;
  }

  [[nodiscard]] auto GetIncludedChannelIndex() const noexcept
      -> std::optional<std::size_t> {
    return included_channel_index_;
  }

  [[nodiscard]] auto GetReason() const noexcept -> std::string_view {
    return reason_;
  }

private:
  std::string label_;
  std::optional<particles::GGEMSParticleType> particle_type_;
  GGEMSEvaluatedQuantity yield_;
  std::optional<GGEMSEvaluatedEnergy> energy_;
  GGEMSEvaluatedEmissionDisposition disposition_;
  std::optional<std::size_t> included_channel_index_;
  std::string reason_;
};

class GGEMSRadionuclideScientificMetadata {
public:
  GGEMSRadionuclideScientificMetadata(
      GGEMSRadionuclideModelKind model_kind,
      GGEMSRadionuclideCompleteness completeness, GGEMSNuclideIdentity parent,
      GGEMSNuclideIdentity daughter,
      GGEMSRadionuclideEvaluationSource evaluation_source,
      std::vector<GGEMSEvaluatedQuantity> evaluated_quantities = {},
      std::vector<GGEMSEvaluatedEmissionMetadata> evaluated_emissions = {},
      std::vector<GGEMSBetaApproximationMetadata> beta_approximations = {});

  [[nodiscard]] auto GetModelKind() const noexcept
      -> GGEMSRadionuclideModelKind {
    return model_kind_;
  }

  [[nodiscard]] auto GetCompleteness() const noexcept
      -> GGEMSRadionuclideCompleteness {
    return completeness_;
  }

  [[nodiscard]] auto GetParent() const noexcept
      -> GGEMSNuclideIdentity const & {
    return parent_;
  }

  [[nodiscard]] auto GetDaughter() const noexcept
      -> GGEMSNuclideIdentity const & {
    return daughter_;
  }

  [[nodiscard]] auto GetEvaluationSource() const noexcept
      -> GGEMSRadionuclideEvaluationSource const & {
    return evaluation_source_;
  }

  [[nodiscard]] auto GetEvaluatedQuantities() const noexcept
      -> std::span<GGEMSEvaluatedQuantity const> {
    return evaluated_quantities_;
  }

  [[nodiscard]] auto GetEvaluatedEmissions() const noexcept
      -> std::span<GGEMSEvaluatedEmissionMetadata const> {
    return evaluated_emissions_;
  }

  [[nodiscard]] auto GetBetaApproximations() const noexcept
      -> std::span<GGEMSBetaApproximationMetadata const> {
    return beta_approximations_;
  }

private:
  friend class GGEMSRadionuclideDefinition;

  auto ValidateChannelLinks(std::size_t channel_count) const -> void;

  GGEMSRadionuclideModelKind model_kind_;
  GGEMSRadionuclideCompleteness completeness_;
  GGEMSNuclideIdentity parent_;
  GGEMSNuclideIdentity daughter_;
  GGEMSRadionuclideEvaluationSource evaluation_source_;
  std::vector<GGEMSEvaluatedQuantity> evaluated_quantities_;
  std::vector<GGEMSEvaluatedEmissionMetadata> evaluated_emissions_;
  std::vector<GGEMSBetaApproximationMetadata> beta_approximations_;
};

} // namespace ggems::core::radioactivity
