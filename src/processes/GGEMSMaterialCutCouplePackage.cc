#include <algorithm>
#include <array>
#include <compare>
#include <cstddef>
#include <cstdint>
#include <format>
#include <limits>
#include <span>
#include <vector>

#include "GGEMS/GGEMSException.hh"
#include "GGEMS/materials/GGEMSEMMaterialPackage.hh"
#include "GGEMS/processes/GGEMSMaterialCutCouplePackage.hh"
#include "GGEMS/processes/GGEMSProductionCutConverter.hh"
#include "GGEMS/processes/GGEMSProductionCutPolicy.hh"
#include "GGEMS/units/GGEMSEnergyUnits.hh"

namespace ggems::core::processes {

namespace {

// =============================================================================
// =============================================================================

struct ResolvedContext {
  std::uint32_t material_id;
  GGEMSResolvedProductionCutLengths lengths;

  [[nodiscard]] auto operator<=>(ResolvedContext const &) const
      -> std::strong_ordering = default;
};

// =============================================================================
// =============================================================================

[[nodiscard]] auto
RequireMaterialId(std::span<std::uint32_t const> material_ids,
                  std::uint32_t material_index) -> std::uint32_t {
  if (material_index >= material_ids.size()) {
    throw GGEMSRecoverable{
        std::format("Production-Cut policy references unknown Material {}.",
                    material_index)};
  }
  return material_ids[material_index];
}

// =============================================================================
// =============================================================================

template <typename Value>
[[nodiscard]] auto SortedUnique(std::vector<Value> values)
    -> std::vector<Value> {
  std::ranges::sort(values);
  auto const duplicates = std::ranges::unique(values);
  values.erase(duplicates.begin(), duplicates.end());
  return values;
}

// =============================================================================
// =============================================================================

template <typename Value>
[[nodiscard]] auto RankOf(std::vector<Value> const &sorted, Value const &value)
    -> std::size_t {
  return static_cast<std::size_t>(std::ranges::lower_bound(sorted, value) -
                                  sorted.begin());
}

} // namespace

// =============================================================================
// =============================================================================

GGEMSMaterialCutCouplePackage::GGEMSMaterialCutCouplePackage(
    materials::GGEMSEMMaterialPackage const &materials,
    GGEMSProductionCutPolicy const &policy,
    std::span<GGEMSProductionCutContext const> contexts) {

  RequireAdmissibleProductionCutPolicy(policy);

  auto const material_ids = materials.GetMaterialIds();

  for (auto const &material : policy.materials) {
    static_cast<void>(RequireMaterialId(material_ids, material.material_index));
  }

  std::vector<ResolvedContext> resolved_contexts;
  resolved_contexts.reserve(contexts.size());
  for (auto const &context : contexts) {
    resolved_contexts.push_back({
        .material_id = RequireMaterialId(material_ids, context.material_index),
        .lengths = ResolveProductionCutLengths(policy, context),
    });
  }

  auto const conversion_inputs = SortedUnique(resolved_contexts);

  std::vector<GGEMSMaterialCutCouple> converted;
  converted.reserve(conversion_inputs.size());
  for (auto const &input : conversion_inputs) {
    std::array<units::Energy, 4U> thresholds{};
    for (auto const channel : k_production_cut_channels) {
      auto const index = ProductionCutChannelIndex(channel);
      thresholds[index] = ConvertProductionCutLength(
          channel, input.lengths[index], materials, input.material_id);
    }
    converted.push_back({
        .material_id = input.material_id,
        .thresholds = thresholds,
    });
  }

  couples_ = SortedUnique(converted);

  context_couple_ids_.reserve(resolved_contexts.size());
  for (auto const &context : resolved_contexts) {
    auto const &couple = converted[RankOf(conversion_inputs, context)];
    context_couple_ids_.push_back(
        static_cast<std::uint32_t>(RankOf(couples_, couple)));
  }
}

} // namespace ggems::core::processes
