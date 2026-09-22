#include <cstddef>
#include <cstdint>
#include <format>
#include <optional>
#include <span>
#include <string_view>
#include <utility>
#include <vector>
#include <algorithm>

#include "GGEMS/GGEMSException.hh"
#include "GGEMS/materials/GGEMSMaterial.hh"
#include "GGEMS/materials/GGEMSMaterialManager.hh"
#include "GGEMS/materials/builtins/GGEMSBuiltInMaterials.hh"

namespace ggems::core::materials {

namespace {

// =============================================================================
// =============================================================================

[[nodiscard]] auto IsBuiltInMaterialName(std::string_view name) noexcept
  -> bool {
  return std::ranges::any_of(
    builtins::GetAvailableMaterialNames(),
    [name](std::string_view built_in_name) noexcept -> bool {
      return built_in_name == name;
    });
}

// =============================================================================
// =============================================================================

[[nodiscard]] auto AddMaterial(std::vector<GGEMSMaterial> &materials,
                               GGEMSMaterial material) -> std::uint32_t {
  auto const material_index = static_cast<std::uint32_t>(materials.size());
  materials.push_back(std::move(material));
  return material_index;
}

} // namespace

// =============================================================================
// =============================================================================

[[nodiscard]] auto GGEMSMaterialManager::GetInstance() noexcept
  -> GGEMSMaterialManager & {
  static GGEMSMaterialManager instance;
  return instance;
}

// -----------------------------------------------------------------------------

[[nodiscard]] auto GGEMSMaterialManager::GetOrAdd(std::string_view name)
  -> std::uint32_t {
  if (auto const material_index = FindIndex(name); material_index.has_value()) {
    return *material_index;
  }

  if (IsBuiltInMaterialName(name)) {
    return AddMaterial(materials_, builtins::BuildBuiltInMaterial(name));
  }

  if (auto const *material = FindCustom(name); material != nullptr) {
    return AddMaterial(materials_, *material);
  }

  throw GGEMSRecoverable{std::format("Unknown Material '{}'.", name)};
}

// -----------------------------------------------------------------------------

auto GGEMSMaterialManager::AddCustomMaterial(GGEMSMaterial material) -> void {
  auto const name = material.GetName();

  if (IsBuiltInMaterialName(name)) {
    throw GGEMSRecoverable{std::format(
      "Custom Material '{}' conflicts with a built-in Material.", name)};
  }

  if (FindCustom(name) != nullptr) {
    throw GGEMSRecoverable{
      std::format("Material '{}' is already defined.", name)};
  }

  custom_materials_.push_back(std::move(material));
}

// -----------------------------------------------------------------------------

[[nodiscard]] auto GGEMSMaterialManager::GetMaterials() const noexcept
  -> std::span<GGEMSMaterial const> {
  return materials_;
}

// -----------------------------------------------------------------------------

[[nodiscard]] auto
GGEMSMaterialManager::FindIndex(std::string_view name) const noexcept
  -> std::optional<std::uint32_t> {
  for (std::size_t index = 0U; index < materials_.size(); ++index) {
    if (materials_[index].GetName() == name) {
      return static_cast<std::uint32_t>(index);
    }
  }

  return std::nullopt;
}

// -----------------------------------------------------------------------------

[[nodiscard]] auto
GGEMSMaterialManager::Find(std::string_view name) const noexcept
  -> GGEMSMaterial const * {
  auto const material_index = FindIndex(name);

  if (!material_index.has_value()) {
    return nullptr;
  }

  return &materials_[*material_index];
}

// -----------------------------------------------------------------------------

[[nodiscard]] auto
GGEMSMaterialManager::FindCustom(std::string_view name) const noexcept
  -> GGEMSMaterial const * {
  for (auto const &material : custom_materials_) {
    if (material.GetName() == name) {
      return &material;
    }
  }

  return nullptr;
}

// -----------------------------------------------------------------------------

[[nodiscard]] auto GGEMSMaterialManager::GetCustomMaterials() const noexcept
  -> std::span<GGEMSMaterial const> {
  return custom_materials_;
}

// -----------------------------------------------------------------------------

[[nodiscard]] auto
GGEMSMaterialManager::Require(std::uint32_t material_index) const
  -> GGEMSMaterial const & {
  if (material_index >= materials_.size()) {
    throw GGEMSRecoverable{
      std::format("Unknown Material index {}.", material_index)};
  }

  return materials_[material_index];
}

} // namespace ggems::core::materials
