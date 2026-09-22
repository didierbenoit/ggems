#pragma once

#include <cstdint>
#include <optional>
#include <span>
#include <string_view>
#include <vector>

#include "GGEMS/materials/GGEMSMaterial.hh"

namespace ggems::core::materials {

class GGEMSMaterialManager {
public:
  [[nodiscard]] static auto GetInstance() noexcept -> GGEMSMaterialManager &;

  GGEMSMaterialManager(GGEMSMaterialManager const &) = delete;
  GGEMSMaterialManager(GGEMSMaterialManager &&) = delete;
  auto operator=(GGEMSMaterialManager const &)
    -> GGEMSMaterialManager & = delete;
  auto operator=(GGEMSMaterialManager &&) -> GGEMSMaterialManager & = delete;

  [[nodiscard]] auto GetOrAdd(std::string_view name) -> std::uint32_t;

  auto AddCustomMaterial(GGEMSMaterial material) -> void;

  [[nodiscard]] auto GetMaterials() const noexcept
    -> std::span<GGEMSMaterial const>;

  [[nodiscard]] auto GetCustomMaterials() const noexcept
    -> std::span<GGEMSMaterial const>;

  [[nodiscard]] auto FindIndex(std::string_view name) const noexcept
    -> std::optional<std::uint32_t>;

  [[nodiscard]] auto Find(std::string_view name) const noexcept
    -> GGEMSMaterial const *;

  [[nodiscard]] auto FindCustom(std::string_view name) const noexcept
    -> GGEMSMaterial const *;

  [[nodiscard]] auto Require(std::uint32_t material_index) const
    -> GGEMSMaterial const &;

private:
  GGEMSMaterialManager() = default;

  std::vector<GGEMSMaterial> materials_;
  std::vector<GGEMSMaterial> custom_materials_;
};

} // namespace ggems::core::materials
