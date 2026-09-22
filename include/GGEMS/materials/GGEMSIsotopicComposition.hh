#pragma once

#include <cstdint>
#include <span>
#include <vector>

#include "GGEMS/materials/GGEMSIsotope.hh"

namespace ggems::core::materials {

enum class GGEMSFractionBasis : std::uint8_t { AtomFraction, MassFraction };

struct GGEMSIsotopeFraction {
  GGEMSIsotope isotope;
  long double fraction;

  [[nodiscard]] auto operator==(GGEMSIsotopeFraction const &) const
    -> bool = default;
};

class GGEMSIsotopicComposition {
public:
  GGEMSIsotopicComposition(GGEMSFractionBasis basis,
                           std::vector<GGEMSIsotopeFraction> fractions);

  [[nodiscard]] auto GetAtomicNumber() const noexcept -> std::uint32_t {
    return fractions_.front().isotope.GetAtomicNumber();
  }

  [[nodiscard]] auto GetBasis() const noexcept -> GGEMSFractionBasis {
    return basis_;
  }

  [[nodiscard]] auto GetFractions() const noexcept
    -> std::span<GGEMSIsotopeFraction const> {
    return fractions_;
  }

  [[nodiscard]] auto operator==(GGEMSIsotopicComposition const &) const
    -> bool = default;

private:
  GGEMSFractionBasis basis_;
  std::vector<GGEMSIsotopeFraction> fractions_;
};

[[nodiscard]] auto BuildDefaultIsotopicComposition(std::uint32_t atomic_number)
  -> GGEMSIsotopicComposition;

} // namespace ggems::core::materials
