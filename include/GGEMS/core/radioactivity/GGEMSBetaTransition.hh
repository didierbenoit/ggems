#pragma once

#include <cstdint>

namespace ggems::core::radioactivity {

enum class GGEMSBetaSign : std::uint8_t { Minus, Plus };

enum class GGEMSBetaTransitionClass : std::uint8_t {
  Allowed,
  FirstForbidden,
  UniqueFirstForbidden,
  SecondForbidden,
  UniqueSecondForbidden,
  ThirdForbidden,
  UniqueThirdForbidden,
  Unclassified
};

class GGEMSBetaTransition {
public:
  GGEMSBetaTransition(GGEMSBetaSign sign, std::uint32_t daughter_atomic_number,
                      std::uint32_t daughter_mass_number,
                      std::uint64_t endpoint_kinetic_energy_milli_ev,
                      GGEMSBetaTransitionClass transition_class);

  ~GGEMSBetaTransition() = default;
  GGEMSBetaTransition(GGEMSBetaTransition const &) = default;
  GGEMSBetaTransition(GGEMSBetaTransition &&) noexcept = default;
  auto operator=(GGEMSBetaTransition const &)
      -> GGEMSBetaTransition & = default;
  auto operator=(GGEMSBetaTransition &&) noexcept
      -> GGEMSBetaTransition & = default;
  auto operator==(GGEMSBetaTransition const &) const noexcept -> bool = default;

  [[nodiscard]] auto GetSign() const noexcept -> GGEMSBetaSign { return sign_; }

  [[nodiscard]] auto GetDaughterAtomicNumber() const noexcept -> std::uint32_t {
    return daughter_atomic_number_;
  }

  [[nodiscard]] auto GetDaughterMassNumber() const noexcept -> std::uint32_t {
    return daughter_mass_number_;
  }

  [[nodiscard]] auto GetEndpointKineticEnergyMilliElectronVolt() const noexcept
      -> std::uint64_t {
    return endpoint_kinetic_energy_milli_eV_;
  }

  [[nodiscard]] auto GetTransitionClass() const noexcept
      -> GGEMSBetaTransitionClass {
    return transition_class_;
  }

private:
  GGEMSBetaSign sign_;
  std::uint32_t daughter_atomic_number_;
  std::uint32_t daughter_mass_number_;
  std::uint64_t endpoint_kinetic_energy_milli_eV_;
  GGEMSBetaTransitionClass transition_class_;
};
} // namespace ggems::core::radioactivity
