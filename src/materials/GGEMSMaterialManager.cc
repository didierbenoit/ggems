#include <cstddef>
#include <cstdint>
#include <format>
#include <limits>
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
  if (materials.size() >=
      static_cast<std::size_t>(std::numeric_limits<std::uint32_t>::max())) {
    throw GGEMSRecoverable{"Too many Materials."};
  }

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
//
// Built-in Materials are created lazily. Repeated requests return the same
// process-wide material index.
//
// -----------------------------------------------------------------------------

[[nodiscard]] auto
GGEMSMaterialManager::GetOrAddBuiltIn(std::string_view canonical_name)
    -> std::uint32_t {
  if (auto const material_index = FindIndex(canonical_name);
      material_index.has_value()) {
    return *material_index;
  }

  return AddMaterial(materials_,
                     builtins::BuildBuiltInMaterial(canonical_name));
}

// -----------------------------------------------------------------------------
//
// Custom Materials share the same process-wide index space as built-ins.
// Built-in canonical names cannot be shadowed.
//
// -----------------------------------------------------------------------------

[[nodiscard]] auto
GGEMSMaterialManager::AddCustomMaterial(GGEMSMaterial material)
    -> std::uint32_t {
  auto const name = material.GetName();

  if (IsBuiltInMaterialName(name)) {
    throw GGEMSRecoverable{std::format(
        "Custom Material '{}' conflicts with a built-in Material.", name)};
  }

  if (Find(name) != nullptr) {
    throw GGEMSRecoverable{
        std::format("Material '{}' is already registered.", name)};
  }

  return AddMaterial(materials_, std::move(material));
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
GGEMSMaterialManager::Require(std::uint32_t material_index) const
    -> GGEMSMaterial const & {
  if (material_index >= materials_.size()) {
    throw GGEMSRecoverable{
        std::format("Unknown Material index {}.", material_index)};
  }

  return materials_[material_index];
}

// -----------------------------------------------------------------------------

[[nodiscard]] auto GGEMSMaterialManager::Require(std::string_view name) const
    -> GGEMSMaterial const & {
  auto const *material = Find(name);

  if (material == nullptr) {
    throw GGEMSRecoverable{std::format("Unknown Material '{}'.", name)};
  }

  return *material;
}

} // namespace ggems::core::materials
