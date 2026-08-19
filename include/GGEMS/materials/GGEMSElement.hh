#pragma once

#include <cstdint>
#include <limits>
#include <string_view>

#include "GGEMS/GGEMSException.hh"

namespace ggems::core::materials {

class GGEMSElement {
public:
  constexpr GGEMSElement(std::uint32_t atomic_number, std::string_view symbol,
                         std::string_view name, long double molar_mass)
      : atomic_number_{atomic_number}, symbol_{symbol}, name_{name},
        molar_mass_{molar_mass} {
    if (atomic_number < 1U || atomic_number > 118U) {
      throw GGEMSRecoverable{"Atomic number must be in [1, 118]."};
    }

    if (symbol.empty()) {
      throw GGEMSRecoverable{"Element symbol must not be empty."};
    }

    if (name.empty()) {
      throw GGEMSRecoverable{"Element name must not be empty."};
    }

    if (!(molar_mass > 0.0L) ||
        !(molar_mass <= std::numeric_limits<long double>::max())) {
      throw GGEMSRecoverable{
          "Molar mass must be finite and strictly positive."};
    }
  }

  [[nodiscard]] constexpr auto GetAtomicNumber() const noexcept
      -> std::uint32_t {
    return atomic_number_;
  }

  [[nodiscard]] constexpr auto GetSymbol() const noexcept -> std::string_view {
    return symbol_;
  }

  [[nodiscard]] constexpr auto GetName() const noexcept -> std::string_view {
    return name_;
  }

  [[nodiscard]] constexpr auto GetMolarMass() const noexcept -> long double {
    return molar_mass_;
  }

private:
  std::uint32_t atomic_number_;
  std::string_view symbol_;
  std::string_view name_;
  long double molar_mass_;
};

} // namespace ggems::core::materials
