#pragma once

#include <array>
#include <compare>
#include <cstdint>
#include <span>
#include <vector>

#include "GGEMS/materials/GGEMSEMMaterialPackage.hh"
#include "GGEMS/processes/GGEMSProductionCutPolicy.hh"
#include "GGEMS/units/GGEMSEnergyUnits.hh"

namespace ggems::core::processes {

struct GGEMSMaterialCutCouple {
  std::uint32_t material_id;
  std::array<units::Energy, 4U> thresholds;

  [[nodiscard]] auto operator<=>(GGEMSMaterialCutCouple const &) const
      -> std::strong_ordering = default;
};

class GGEMSMaterialCutCouplePackage {
public:
  GGEMSMaterialCutCouplePackage(
      materials::GGEMSEMMaterialPackage const &materials,
      GGEMSProductionCutPolicy const &policy,
      std::span<GGEMSProductionCutContext const> contexts);

  [[nodiscard]] auto GetCouples() const noexcept
      -> std::span<GGEMSMaterialCutCouple const> {
    return couples_;
  }

  [[nodiscard]] auto GetContextCoupleIds() const noexcept
      -> std::span<std::uint32_t const> {
    return context_couple_ids_;
  }

private:
  std::vector<GGEMSMaterialCutCouple> couples_;
  std::vector<std::uint32_t> context_couple_ids_;
};

} // namespace ggems::core::processes
