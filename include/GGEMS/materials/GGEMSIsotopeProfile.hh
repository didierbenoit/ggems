#pragma once

#include <cstdint>

#include "GGEMS/materials/GGEMSIsotopicComposition.hh"

namespace ggems::core::materials {

enum class GGEMSIsotopeProfile : std::uint8_t {
  Nist41Natural,
  LegacyReferenceIsotope,
};

[[nodiscard]] auto HasIsotopeProfile(GGEMSIsotopeProfile profile,
                                     std::uint32_t atomic_number) noexcept
    -> bool;

[[nodiscard]] auto ResolveIsotopeProfile(GGEMSIsotopeProfile profile,
                                         std::uint32_t atomic_number)
    -> GGEMSIsotopicComposition;

[[nodiscard]] auto
SelectLegacyElementalIsotopeProfile(std::uint32_t atomic_number)
    -> GGEMSIsotopeProfile;

} // namespace ggems::core::materials
