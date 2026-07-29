#include <cmath>
#include <format>
#include <optional>
#include <span>
#include <string>
#include <string_view>
#include <unordered_set>
#include <utility>
#include <vector>
#include <algorithm>

#include "GGEMS/core/GGEMSException.hh"
#include "GGEMS/core/GGEMSMacros.hh"
#include "GGEMS/core/radioactivity/GGEMSRadionuclideDefinition.hh"
#include "GGEMS/core/radioactivity/GGEMSRadionuclideEmission.hh"
#include "GGEMS/core/radioactivity/GGEMSRadionuclideProvenance.hh"
#include "GGEMS/core/radioactivity/GGEMSRadionuclideScientificMetadata.hh"

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
  return std::ranges::any_of(text, [](char character) noexcept -> bool {
    return !IsAsciiWhitespace(character);
  });
}

// =============================================================================
// =============================================================================

[[nodiscard]] auto
ComputeTotalYieldPerDecay(std::span<GGEMSRadionuclideEmission const> emissions)
    -> long double {
  long double sum{0.0L};
  long double compensation{0.0L};

  for (GGEMSRadionuclideEmission const &emission : emissions) {
    long double const value = emission.GetYieldPerDecay();
    long double const next = sum + value;

    GGEMS_CHECK_RECOVERABLE(std::isfinite(next),
                            "Radionuclide total emission yield is not finite.");

    if (std::abs(sum) >= std::abs(value)) {
      compensation += (sum - next) + value;
    } else {
      compensation += (value - next) + sum;
    }

    GGEMS_CHECK_RECOVERABLE(std::isfinite(compensation),
                            "Radionuclide total emission yield in not finite.");

    sum = next;
  }

  long double const total = sum + compensation;

  GGEMS_CHECK_RECOVERABLE(std::isfinite(total),
                          "Radionuclide total emission yield is not finite.");
  GGEMS_CHECK_RECOVERABLE(
      total > 0.0L,
      "Radionuclide total emission yield must be strictly positive.");

  return total;
}

} // namespace

// =============================================================================
// =============================================================================

GGEMSRadionuclideProvenance::GGEMSRadionuclideProvenance(
    std::string authority, std::string citation,
    std::string evaluation_date_or_version,
    std::optional<std::string> reference_identifier)
    : authority_{std::move(authority)}, citation_{std::move(citation)},
      evaluation_date_or_version_{std::move(evaluation_date_or_version)},
      reference_identifier_{std::move(reference_identifier)} {
  GGEMS_CHECK_RECOVERABLE(
      HasVisibleText(authority_),
      "Radionuclide provenance authority must contain non-whitespace text.");
  GGEMS_CHECK_RECOVERABLE(
      HasVisibleText(citation_),
      "Radionuclide provenance citation must contain non-whitespace text.");
  GGEMS_CHECK_RECOVERABLE(
      HasVisibleText(evaluation_date_or_version_),
      "Radionuclide provenance evaluation date or version must contain "
      "non-whitespace text.");

  if (reference_identifier_.has_value()) {
    GGEMS_CHECK_RECOVERABLE(
        HasVisibleText(*reference_identifier_),
        "Radionuclide provenance reference identifier must contain "
        "non-whitespace text when present.");
  }
}

// =============================================================================
// =============================================================================

[[nodiscard]] auto
GGEMSRadionuclideDefinition::NormalizeLookupName(std::string_view name)
    -> std::string {
  while (!name.empty() && IsAsciiWhitespace(name.front())) {
    name.remove_prefix(1U);
  }

  while (!name.empty() && IsAsciiWhitespace(name.back())) {
    name.remove_suffix(1U);
  }

  std::string normalized;
  normalized.reserve(name.size());

  for (char character : name) {
    if (character >= 'A' && character <= 'Z') {
      normalized.push_back(static_cast<char>(character - 'A' + 'a'));
    } else {
      normalized.push_back(character);
    }
  }

  return normalized;
}

// -----------------------------------------------------------------------------

[[nodiscard]] auto GGEMSRadionuclideDefinition::BuildLookupKeys(
    std::string_view canonical_name, std::span<std::string const> aliases)
    -> std::vector<std::string> {
  std::vector<std::string> keys;
  std::unordered_set<std::string> unique_keys;

  auto add_key = [&](std::string_view name, std::string_view kind) -> void {
    std::string normalized = NormalizeLookupName(name);

    GGEMS_CHECK_RECOVERABLE(
        !normalized.empty(),
        std::format("Radionuclide {} must contain non-whitespace text.", kind));
    GGEMS_CHECK_RECOVERABLE(
        unique_keys.insert(normalized).second,
        std::format("Duplicate radionuclide {} '{}' after ASCII lookup "
                    "normalization.",
                    kind, name));

    keys.push_back(std::move(normalized));
  };

  add_key(canonical_name, "canonical name");

  for (std::string const &alias : aliases) {
    add_key(alias, "alias");
  }

  return keys;
}

// -----------------------------------------------------------------------------

GGEMSRadionuclideDefinition::GGEMSRadionuclideDefinition(
    std::string canonical_name, std::vector<std::string> aliases,
    long double half_life_seconds, GGEMSRadionuclideProvenance provenance,
    std::vector<GGEMSRadionuclideEmission> emissions)
    : canonical_name_{std::move(canonical_name)}, aliases_{std::move(aliases)},
      half_life_seconds_{half_life_seconds}, provenance_{std::move(provenance)},
      emissions_{std::move(emissions)} {
  ValidateAndBuildDerivedState();
}

// -----------------------------------------------------------------------------

GGEMSRadionuclideDefinition::GGEMSRadionuclideDefinition(
    std::string canonical_name, std::vector<std::string> aliases,
    long double half_life_seconds,
    GGEMSRadionuclideScientificMetadata scientific_metadata,
    std::vector<GGEMSRadionuclideEmission> emissions)
    : canonical_name_{std::move(canonical_name)}, aliases_{std::move(aliases)},
      half_life_seconds_{half_life_seconds},
      provenance_{scientific_metadata.GetEvaluationSource().GetProvenance()},
      emissions_{std::move(emissions)},
      scientific_metadata_{std::move(scientific_metadata)} {
  scientific_metadata_->ValidateChannelLinks(emissions_.size());
  ValidateAndBuildDerivedState();
}

// -----------------------------------------------------------------------------

auto GGEMSRadionuclideDefinition::ValidateAndBuildDerivedState() -> void {
  GGEMS_CHECK_RECOVERABLE(std::isfinite(half_life_seconds_),
                          "Radionuclide half-life must be finite.");
  GGEMS_CHECK_RECOVERABLE(half_life_seconds_ > 0.0L,
                          "Radionuclide half-life must be strictly positive.");
  GGEMS_CHECK_RECOVERABLE(
      !emissions_.empty(),
      "Radionuclide definition requires at least one emission channel.");

  lookup_keys_ = BuildLookupKeys(canonical_name_, aliases_);
  total_yield_per_decay_ = ComputeTotalYieldPerDecay(emissions_);
  channel_selection_weights_.reserve(emissions_.size());

  for (GGEMSRadionuclideEmission const &emission : emissions_) {
    long double const selection_weight =
        emission.GetYieldPerDecay() / total_yield_per_decay_;
    GGEMS_CHECK_INTERNAL(
        std::isfinite(selection_weight),
        "Radionuclide channel selection weight is not finite.");
    channel_selection_weights_.push_back(selection_weight);
  }
}
} // namespace ggems::core::radioactivity
