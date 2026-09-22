#pragma once

#include <compare>
#include <cstdint>

#include "GGEMS/GGEMSException.hh"

namespace ggems::core::materials {

class GGEMSIsotope {
public:
  constexpr GGEMSIsotope(std::uint32_t atomic_number, std::uint32_t mass_number,
                         std::uint32_t isomer_state)
      : atomic_number_{atomic_number}, mass_number_{mass_number},
        isomer_state_{isomer_state} {
    if (atomic_number < 1U || atomic_number > 92U) {
      throw GGEMSRecoverable{"Isotope atomic number must be in [1, 92]."};
    }

    if (mass_number < atomic_number) {
      throw GGEMSRecoverable{
        "Isotope mass number must not be smaller than its atomic number."};
    }
  }

  [[nodiscard]] constexpr auto GetAtomicNumber() const noexcept
    -> std::uint32_t {
    return atomic_number_;
  }

  [[nodiscard]] constexpr auto GetMassNumber() const noexcept -> std::uint32_t {
    return mass_number_;
  }

  [[nodiscard]] constexpr auto GetIsomerState() const noexcept
    -> std::uint32_t {
    return isomer_state_;
  }

  [[nodiscard]] constexpr auto operator<=>(GGEMSIsotope const &) const noexcept
    -> std::strong_ordering = default;

private:
  std::uint32_t atomic_number_;
  std::uint32_t mass_number_;
  std::uint32_t isomer_state_;
};

} // namespace ggems::core::materials
