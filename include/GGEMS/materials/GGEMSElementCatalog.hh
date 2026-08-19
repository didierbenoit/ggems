#pragma once

#include <cstdint>
#include <span>
#include <string_view>

#include "GGEMS/materials/GGEMSElement.hh"

namespace ggems::core::materials {

[[nodiscard]] auto GetElements() noexcept -> std::span<GGEMSElement const>;

[[nodiscard]] auto
FindElementByAtomicNumber(std::uint32_t atomic_number) noexcept
    -> GGEMSElement const *;

[[nodiscard]] auto FindElementBySymbol(std::string_view symbol) noexcept
    -> GGEMSElement const *;

[[nodiscard]] auto FindElementByName(std::string_view canonical_name) noexcept
    -> GGEMSElement const *;

[[nodiscard]] auto RequireElementByAtomicNumber(std::uint32_t atomic_number)
    -> GGEMSElement const &;

[[nodiscard]] auto RequireElementBySymbol(std::string_view symbol)
    -> GGEMSElement const &;

[[nodiscard]] auto RequireElementByName(std::string_view canonical_name)
    -> GGEMSElement const &;

} // namespace ggems::core::materials
