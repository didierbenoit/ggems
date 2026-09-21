#include <algorithm>
#include <array>
#include <cmath>
#include <cstddef>
#include <cstdint>
#include <format>
#include <numbers>
#include <span>
#include <string_view>
#include <vector>

#include "GGEMS/GGEMSException.hh"
#include "GGEMS/materials/GGEMSEMMaterialPackage.hh"
#include "GGEMS/processes/GGEMSProductionCutConverter.hh"
#include "GGEMS/processes/GGEMSProductionCutPolicy.hh"
#include "GGEMS/units/GGEMSEnergyUnits.hh"
#include "GGEMS/units/GGEMSLengthUnits.hh"
#include "GGEMS/units/GGEMSUnitConversion.hh"

namespace ggems::core::processes {

namespace {

// =============================================================================
// =============================================================================
using units::operator""_eV;
using units::operator""_GeV;

// =============================================================================
// =============================================================================

using Constituents = std::span<materials::GGEMSEMElementalConstituent const>;

// =============================================================================
// =============================================================================

constexpr units::Energy k_converter_min_energy{990_eV};
constexpr units::Energy k_converter_max_energy{10_GeV};

// =============================================================================
// =============================================================================

constexpr long double k_electron_mass_mev{0.510998910L};
constexpr long double k_twopi_mc2_rcl2_mev_cm2{2.549549767053704080525235e-25L};
constexpr long double k_barn_cm2{1.0e-24L};
constexpr long double k_gamma_range_absorption_lengths{5.0L};

constexpr long double k_lepton_low_energy_mev{0.01L};
constexpr long double k_lepton_brems_reference_mev{1000.0L};
constexpr long double k_density_correction_energy_mev{0.03L};
constexpr long double k_density_correction_tune_g_cm2{0.0025L};

constexpr std::size_t k_lepton_cells_per_decade{50U};

// =============================================================================
// =============================================================================

constexpr std::array<long double, 5U> k_gauss_nodes{
    -0.906179845938663992797626878299392965L,
    -0.538469310105683091036314420700208805L,
    0.0L,
    0.538469310105683091036314420700208805L,
    0.906179845938663992797626878299392965L,
};

// =============================================================================
// =============================================================================

constexpr std::array<long double, 5U> k_gauss_weights{
    0.236926885056189087514264040719917363L,
    0.478628670499366468041291514835638193L,
    0.568888888888888888888888888888888889L,
    0.478628670499366468041291514835638193L,
    0.236926885056189087514264040719917363L,
};

// =============================================================================
// =============================================================================

[[nodiscard]] auto ToMeV(units::Energy energy) -> long double {
  return *units::ConvertTo(energy, "MeV");
}

// =============================================================================
// =============================================================================

template <typename Range>
[[nodiscard]] auto Bisect(Range const &range, long double lower,
                          long double upper, long double length)
    -> long double {
  for (;;) {
    auto const middle = lower + ((upper - lower) / 2.0L);
    if (middle <= lower || middle >= upper) {
      return upper;
    }
    if (range(middle) >= length) {
      upper = middle;
    } else {
      lower = middle;
    }
  }
}

// =============================================================================
// =============================================================================

struct GammaAbsorptionElement {
  long double number_density;
  long double atomic_number;
  long double tmin;
  long double tlow;
  long double smin;
  long double s200;
  long double cmin;
  long double slow;
  long double clow;
  long double logtlow;
  long double chigh;
};

// =============================================================================
// =============================================================================

[[nodiscard]] auto MakeGammaAbsorptionElement(
    materials::GGEMSEMElementalConstituent const &constituent)
    -> GammaAbsorptionElement {
  auto const z = static_cast<long double>(constituent.atomic_number);
  auto const zsquare = z * z;
  auto const zlog = std::log(z);
  auto const zlogsquare = zlog * zlog;

  auto const tmin = 0.552L + (218.5L / z) + (557.17L / zsquare);
  auto const tlow = 0.2L * std::exp(-7.355L / std::sqrt(z));
  auto const smin = (0.01239L + (0.005585L * zlog) - (0.000923L * zlogsquare)) *
                    std::exp(1.5L * zlog);
  auto const s200 =
      (0.2651L - (0.1501L * zlog) + (0.02283L * zlogsquare)) * zsquare;
  auto const cminlog = std::log(tmin / 0.2L);
  auto const cmin = std::log(s200 / smin) / (cminlog * cminlog);
  auto const slowlog = std::log(0.2L / tlow);
  auto const slow = s200 * std::exp(0.042L * z * slowlog * slowlog);
  auto const logtlow = std::log(tlow / 1.0e-3L);
  auto const clow = std::log(300.0L * zsquare / slow) / logtlow;
  auto const chigh =
      (7.55e-5L - (0.0542e-5L * z)) * zsquare * z / std::log(100.0L / tmin);

  return {
      .number_density = constituent.number_density_per_cubic_centimeter,
      .atomic_number = z,
      .tmin = tmin,
      .tlow = tlow,
      .smin = smin,
      .s200 = s200,
      .cmin = cmin,
      .slow = slow,
      .clow = clow,
      .logtlow = logtlow,
      .chigh = chigh,
  };
}

// =============================================================================
// =============================================================================

[[nodiscard]] auto
GammaAbsorptionCrossSectionBarn(GammaAbsorptionElement const &element,
                                long double energy) -> long double {
  if (energy < element.tlow) {
    return energy < 1.0e-3L
               ? element.slow * std::exp(element.clow * element.logtlow)
               : element.slow *
                     std::exp(element.clow * std::log(element.tlow / energy));
  }
  if (energy < 0.2L) {
    auto const x = std::log(0.2L / energy);
    return element.s200 * std::exp(0.042L * element.atomic_number * x * x);
  }
  if (energy < element.tmin) {
    auto const x = std::log(element.tmin / energy);
    return element.smin * std::exp(element.cmin * x * x);
  }
  return element.smin + (element.chigh * std::log(energy / element.tmin));
}

// =============================================================================
// =============================================================================

[[nodiscard]] auto ConvertGamma(Constituents constituents,
                                long double length_cm) -> long double {
  std::vector<GammaAbsorptionElement> elements;
  elements.reserve(constituents.size());
  for (auto const &constituent : constituents) {
    elements.push_back(MakeGammaAbsorptionElement(constituent));
  }

  auto const range = [&elements](long double energy) -> long double {
    long double sigma{0.0L};
    for (auto const &element : elements) {
      sigma += element.number_density *
               GammaAbsorptionCrossSectionBarn(element, energy);
    }
    return k_gamma_range_absorption_lengths / (sigma * k_barn_cm2);
  };

  auto const min_energy = ToMeV(k_converter_min_energy);
  auto const max_energy =
      std::ranges::min_element(elements, {}, &GammaAbsorptionElement::tmin)
          ->tmin;

  auto const min_range = range(min_energy);

  if (min_range > length_cm) {
    throw GGEMSRecoverable{
        "Gamma Production Cut resolves below the 0.99 keV converter domain."};
  }

  if (min_range == length_cm) {
    return min_energy;
  }

  if (range(max_energy) < length_cm) {
    throw GGEMSRecoverable{
        "Gamma Production Cut resolves above the monotone converter domain "
        "of the Material."};
  }

  return Bisect(range, min_energy, max_energy, length_cm);
}

// =============================================================================
// =============================================================================

class LeptonSurrogateStopping {
public:
  LeptonSurrogateStopping(Constituents constituents, bool positron)
      : constituents_{constituents}, positron_{positron} {
    ionisation_log_.reserve(constituents.size());
    for (auto const &constituent : constituents) {
      auto const z = static_cast<long double>(constituent.atomic_number);
      ionisation_log_.push_back(std::log(
          1.6e-5L * std::exp(0.9L * std::log(z)) / k_electron_mass_mev));
    }

    auto const tau = k_lepton_low_energy_mev / k_electron_mass_mev;
    long double sum{0.0L};
    for (std::size_t index = 0U; index < constituents_.size(); ++index) {
      auto const z =
          static_cast<long double>(constituents_[index].atomic_number);
      sum += constituents_[index].number_density_per_cubic_centimeter * z *
             CollisionTerm(tau, ionisation_log_[index]);
    }
    low_branch_at_low_energy_ = sum * k_twopi_mc2_rcl2_mev_cm2;
  }

  [[nodiscard]] auto operator()(long double energy) const -> long double {
    auto const tau = energy / k_electron_mass_mev;
    auto const beta2 = tau * (tau + 2.0L) / ((tau + 1.0L) * (tau + 1.0L));
    auto const brems_log =
        1.0L + (0.072L * std::log(energy / k_lepton_brems_reference_mev));

    long double sum{0.0L};
    for (std::size_t index = 0U; index < constituents_.size(); ++index) {
      auto const z =
          static_cast<long double>(constituents_[index].atomic_number);
      auto const cbrem = (0.02L - (5.7e-5L * z)) * brems_log;
      sum += constituents_[index].number_density_per_cubic_centimeter *
             ((z * CollisionTerm(tau, ionisation_log_[index])) +
              (z * (z + 1.0L) * cbrem * 0.1L * tau / beta2));
    }
    return sum * k_twopi_mc2_rcl2_mev_cm2;
  }

  [[nodiscard]] auto LowEnergyRange(long double energy) const -> long double {
    return 2.0L * energy * std::sqrt(energy / k_lepton_low_energy_mev) /
           (3.0L * low_branch_at_low_energy_);
  }

  [[nodiscard]] auto InverseLowEnergyRange(long double length) const
      -> long double {
    return std::cbrt(std::pow(1.5L * length * low_branch_at_low_energy_, 2.0L) *
                     k_lepton_low_energy_mev);
  }

private:
  [[nodiscard]] auto CollisionTerm(long double tau,
                                   long double ionisation_log) const
      -> long double {
    auto const t1 = tau + 1.0L;
    auto const t2 = tau + 2.0L;
    auto const tsq = tau * tau;
    auto const beta2 = tau * t2 / (t1 * t1);
    auto const f = positron_
                       ? (2.0L * std::log(tau)) -
                             (((6.0L * tau) + (1.5L * tsq) -
                               (tau * (1.0L - (tsq / 3.0L)) / t2) -
                               (tsq * (0.5L - (tsq / 12.0L)) / (t2 * t2))) /
                              (t1 * t1))
                       : 1.0L - beta2 + std::log(tsq / 2.0L) +
                             ((0.5L + (0.25L * tsq) +
                               ((1.0L + (2.0L * tau)) * std::log(0.5L))) /
                              (t1 * t1));
    return (std::log((2.0L * tau) + 4.0L) - (2.0L * ionisation_log) + f) /
           beta2;
  }

  Constituents constituents_;
  bool positron_;
  std::vector<long double> ionisation_log_;
  long double low_branch_at_low_energy_{0.0L};
};

// =============================================================================
// =============================================================================

[[nodiscard]] auto
IntegrateInverseStopping(LeptonSurrogateStopping const &stopping,
                         long double lower_log, long double upper_log)
    -> long double {
  auto const half = (upper_log - lower_log) / 2.0L;
  auto const middle = lower_log + half;
  long double sum{0.0L};
  for (std::size_t index = 0U; index < k_gauss_nodes.size(); ++index) {
    auto const energy = std::exp(middle + (half * k_gauss_nodes[index]));
    sum += k_gauss_weights[index] * energy / stopping(energy);
  }
  return sum * half;
}

// =============================================================================
// =============================================================================

[[nodiscard]] auto ConvertLepton(Constituents constituents,
                                 long double density_g_cm3,
                                 long double length_cm, bool positron)
    -> long double {
  LeptonSurrogateStopping const stopping{constituents, positron};

  auto const min_energy = ToMeV(k_converter_min_energy);
  auto const max_energy = ToMeV(k_converter_max_energy);
  auto const particle =
      positron ? std::string_view{"Positron"} : std::string_view{"Electron"};

  long double provisional{0.0L};

  auto const low_energy_range =
      stopping.LowEnergyRange(k_lepton_low_energy_mev);
  if (length_cm <= low_energy_range) {
    provisional = stopping.InverseLowEnergyRange(length_cm);
  } else {
    auto const lower_log = std::log(k_lepton_low_energy_mev);
    auto const upper_log = std::log(max_energy);
    auto const cells = static_cast<std::size_t>(
        std::ceil((upper_log - lower_log) / std::numbers::ln10_v<long double> *
                  static_cast<long double>(k_lepton_cells_per_decade)));

    auto cell_lower_log = lower_log;
    auto cell_lower_range = low_energy_range;
    for (std::size_t cell = 1U;; ++cell) {
      auto const cell_upper_log =
          cell == cells ? upper_log
                        : lower_log + ((upper_log - lower_log) *
                                       static_cast<long double>(cell) /
                                       static_cast<long double>(cells));
      auto const cell_upper_range =
          cell_lower_range +
          IntegrateInverseStopping(stopping, cell_lower_log, cell_upper_log);

      if (cell_upper_range >= length_cm) {
        auto const range = [&](long double energy) -> long double {
          return cell_lower_range + IntegrateInverseStopping(stopping,
                                                             cell_lower_log,
                                                             std::log(energy));
        };
        provisional = Bisect(
            range, std::exp(cell_lower_log),
            cell == cells ? max_energy : std::exp(cell_upper_log), length_cm);
        break;
      }
      if (cell == cells) {
        throw GGEMSRecoverable{std::format(
            "{} Production Cut resolves above the 10 GeV converter domain.",
            particle)};
      }
      cell_lower_log = cell_upper_log;
      cell_lower_range = cell_upper_range;
    }
  }

  // The density correction only lowers the energy; reject first so that a
  // zero length never reaches its division.
  if (provisional < min_energy) {
    throw GGEMSRecoverable{std::format(
        "{} Production Cut resolves below the 0.99 keV converter domain.",
        particle)};
  }

  auto energy = provisional;
  if (provisional < k_density_correction_energy_mev) {
    energy = provisional /
             (1.0L +
              ((1.0L - (provisional / k_density_correction_energy_mev)) *
               k_density_correction_tune_g_cm2 / (length_cm * density_g_cm3)));
  }

  if (energy < min_energy) {
    throw GGEMSRecoverable{std::format(
        "{} Production Cut resolves below the 0.99 keV converter domain.",
        particle)};
  }

  return energy;
}

// =============================================================================
// =============================================================================

[[nodiscard]] auto RequireMatter(Constituents constituents) -> Constituents {
  if (constituents.empty()) {
    throw GGEMSRecoverable{
        "Production-Cut conversion is undefined for a Material without "
        "matter constituents."};
  }
  return constituents;
}

// =============================================================================
// =============================================================================

[[nodiscard]] auto QuantizeConvertedEnergy(long double energy_mev)
    -> units::Energy {
  return *units::MakeQuantity<units::Energy>(energy_mev, "MeV");
}

// =============================================================================
// =============================================================================

[[nodiscard]] auto ConvertProton(units::Length length) -> units::Energy {
  auto const energy = units::MakeQuantity<units::Energy>(
      100.0L * *units::ConvertTo(length, "mm"), "keV");
  if (!energy.has_value()) {
    throw GGEMSRecoverable{
        "Proton Production Cut exceeds the canonical Energy range."};
  }
  return *energy;
}

} // namespace

// =============================================================================
// =============================================================================

auto ConvertProductionCutLength(
    GGEMSProductionCutChannel channel, units::Length length,
    materials::GGEMSEMMaterialPackage const &materials,
    std::uint32_t material_id) -> units::Energy {
  auto const descriptors = materials.GetDescriptors();
  if (material_id >= descriptors.size()) {
    throw GGEMSRecoverable{std::format(
        "Production-Cut conversion references unknown Material ID {}.",
        material_id)};
  }

  auto const &descriptor = descriptors[material_id];
  auto const constituents = materials.GetElementalConstituents().subspan(
      descriptor.first_constituent, descriptor.constituent_count);
  auto const length_cm = *units::ConvertTo(length, "cm");

  switch (channel) {
  case GGEMSProductionCutChannel::Gamma:
    return QuantizeConvertedEnergy(
        ConvertGamma(RequireMatter(constituents), length_cm));
  case GGEMSProductionCutChannel::Electron:
    return QuantizeConvertedEnergy(ConvertLepton(
        RequireMatter(constituents),
        *units::ConvertTo(descriptor.density, "g/cm3"), length_cm, false));
  case GGEMSProductionCutChannel::Positron:
    return QuantizeConvertedEnergy(ConvertLepton(
        RequireMatter(constituents),
        *units::ConvertTo(descriptor.density, "g/cm3"), length_cm, true));
  case GGEMSProductionCutChannel::Proton:
    return ConvertProton(length);
  }

  throw GGEMSRecoverable{"Unknown Production-Cut channel."};
}

} // namespace ggems::core::processes
