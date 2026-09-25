// *****************************************************************************
// * This file is part of GGEMS.                                               *
// *                                                                           *
// * SPDX-License-Identifier: GPL-3.0-or-later                                 *
// * Copyright (C) 2017-2026 CHRU de Brest, Université de Bretagne Occidentale,*
// * Inserm.                                                                   *
// *                                                                           *
// * GGEMS is free software: you can redistribute it and/or modify             *
// * it under the terms of the GNU General Public License as published by      *
// * the Free Software Foundation, either version 3 of the License, or         *
// * (at your option) any later version.                                       *
// *                                                                           *
// * GGEMS is distributed in the hope that it will be useful,                  *
// * but WITHOUT ANY WARRANTY; without even the implied warranty of            *
// * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the              *
// * GNU General Public License for more details.                              *
// *                                                                           *
// * You should have received a copy of the GNU General Public License         *
// * along with GGEMS. If not, see <https://www.gnu.org/licenses/>.            *
// *****************************************************************************

/*!
 * \file
 * \brief Implements host Gamma, lepton, and Proton production-cut conversion
 * models.
 *
 * \author Julien BERT <julien.bert@univ-brest.fr>
 * \author Didier BENOIT <didier.benoit@inserm.fr>
 */

/// \cond
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
/// \endcond

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

/*! \brief Borrows a material's isotope-derived elemental EM contributions. */
using Constituents = std::span<materials::GGEMSEMElementalConstituent const>;

// =============================================================================
// =============================================================================

/*!
 * \brief Defines the 990 eV lower admitted Gamma and lepton conversion energy.
 */
constexpr units::Energy k_converter_min_energy{990_eV};

/*! \brief Defines the 10 GeV upper lepton conversion energy. */
constexpr units::Energy k_converter_max_energy{10_GeV};

// =============================================================================
// =============================================================================

/*!
 * \brief Defines the electron rest energy in MeV used by the lepton surrogate.
 */
constexpr long double k_electron_mass_mev{0.510998910L};

/*! \brief Defines the lepton stopping scale 2*pi*m*c^2*r_e^2 in MeV cm2. */
constexpr long double k_twopi_mc2_rcl2_mev_cm2{2.549549767053704080525235e-25L};

/*! \brief Converts microscopic cross sections from barns to cm2. */
constexpr long double k_barn_cm2{1.0e-24L};

/*! \brief Defines the Gamma cut proxy as five absorption lengths. */
constexpr long double k_gamma_range_absorption_lengths{5.0L};

/*! \brief Defines the 10 keV analytic-to-integrated lepton range boundary. */
constexpr long double k_lepton_low_energy_mev{0.01L};

/*! \brief Defines the bremsstrahlung logarithm's reference energy in MeV. */
constexpr long double k_lepton_brems_reference_mev{1000.0L};

/*!
 * \brief Defines the 30 keV upper boundary of the lepton density correction.
 */
constexpr long double k_density_correction_energy_mev{0.03L};

/*! \brief Defines the empirical density-correction scale in g/cm2. */
constexpr long double k_density_correction_tune_g_cm2{0.0025L};

/*!
 * \brief Defines logarithmic integration-cell density for lepton range
 * conversion.
 */
constexpr std::size_t k_lepton_cells_per_decade{50U};

// =============================================================================
// =============================================================================

/*! \brief Stores five-point Gauss-Legendre nodes on the interval [-1, 1]. */
constexpr std::array<long double, 5U> k_gauss_nodes{
  -0.906179845938663992797626878299392965L,
  -0.538469310105683091036314420700208805L,
  0.0L,
  0.538469310105683091036314420700208805L,
  0.906179845938663992797626878299392965L,
};

// =============================================================================
// =============================================================================

/*! \brief Stores weights paired with the five Gauss-Legendre nodes. */
constexpr std::array<long double, 5U> k_gauss_weights{
  0.236926885056189087514264040719917363L,
  0.478628670499366468041291514835638193L,
  0.568888888888888888888888888888888889L,
  0.478628670499366468041291514835638193L,
  0.236926885056189087514264040719917363L,
};

// =============================================================================
// =============================================================================

/*!
 * \brief Converts canonical integer Energy to a host MeV working value.
 *
 * \param[in] energy Canonical energy in micro-eV.
 * \return The corresponding long double energy in MeV.
 */
[[nodiscard]] auto ToMeV(units::Energy energy) -> long double {
  return *units::ConvertTo(energy, "MeV");
}

// =============================================================================
// =============================================================================

/*!
 * \brief Finds the upper representable bracket of a monotone range crossing.
 *
 * Equality makes the midpoint the new upper endpoint. Iteration stops when no
 * representable midpoint lies strictly inside the bracket and returns upper.
 *
 * \pre The finite bracket must enclose the requested range crossing.
 *
 * \tparam Range Callable mapping a working energy to a monotone range.
 * \param[in] range Monotone nondecreasing range function.
 * \param[in] lower Lower working-energy bracket.
 * \param[in] upper Upper working-energy bracket.
 * \param[in] length Target length in the same units as range results.
 * \return The upper energy bracket after representable bisection convergence.
 */
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

/*!
 * \brief Caches one element's piecewise approximate absorption-law
 * coefficients.
 */
struct GammaAbsorptionElement {
  /*! \brief Atoms per cubic centimeter. */
  long double number_density;

  /*! \brief Proton number as a working floating value. */
  long double atomic_number;

  /*! \brief Energy of minimum proxy cross section, in MeV. */
  long double tmin;

  /*! \brief Low-energy branch boundary in MeV. */
  long double tlow;

  /*! \brief Proxy cross section at tmin, in barns. */
  long double smin;

  /*! \brief Proxy cross section at 0.2 MeV, in barns. */
  long double s200;

  /*! \brief Dimensionless intermediate-branch curvature. */
  long double cmin;

  /*! \brief Proxy cross section at tlow, in barns. */
  long double slow;

  /*! \brief Dimensionless low-energy power exponent. */
  long double clow;

  /*! \brief Natural logarithm of tlow divided by 1 keV. */
  long double logtlow;

  /*! \brief High-energy logarithm coefficient in barns. */
  long double chigh;
};

// =============================================================================
// =============================================================================

/*!
 * \brief Precomputes the Z-dependent Gamma absorption-proxy coefficients.
 *
 * \param[in] constituent Element Z and isotope-derived atom density.
 * \return A value record of number density and piecewise-law coefficients.
 */
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

/*!
 * \brief Evaluates the piecewise microscopic Gamma absorption proxy.
 *
 * This approximate cut-converter law is independent of runtime Process tables.
 * The low branch freezes below 1 keV; later branches join at tlow, 0.2 MeV, and
 * tmin using the coefficients prepared from Z.
 *
 * \param[in] element Precomputed element coefficients.
 * \param[in] energy Positive working energy in MeV.
 * \return The microscopic absorption proxy in barns.
 */
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

/*!
 * \brief Inverts five material absorption lengths on the admitted monotone
 * branch.
 *
 * The lower energy is 990 eV. The upper energy is the smallest constituent
 * tmin, keeping all element contributions within the selected branch. Exact
 * equality at the lower range returns the lower energy.
 *
 * \pre constituents must be nonempty and describe valid matter.
 *
 * \param[in] constituents Nonempty elemental contributions with valid atom
 * densities.
 * \param[in] length_cm Target cut length in centimeters.
 * \return The unquantized threshold energy in MeV.
 * \throws GGEMSRecoverable If length_cm lies below the lower range or above the
 * monotone upper range.
 */
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
    std::ranges::min_element(elements, {}, &GammaAbsorptionElement::tmin)->tmin;

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

/*!
 * \brief Evaluates converter-local Electron or Positron surrogate stopping
 * power.
 *
 * The borrowed elemental span must remain alive while the evaluator is used.
 * Collision formulas differ for electrons and positrons. These surrogates are
 * used only for production-cut conversion, not restricted transport stopping.
 */
class LeptonSurrogateStopping {
public:
  /*!
   * \brief Prepares ionization logarithms and the analytic low-energy
   * normalization.
   *
   * \param[in] constituents Borrowed elemental EM rows that outlive this
   * evaluator.
   * \param[in] positron True for Positron formulas; false for Electron
   * formulas.
   */
  LeptonSurrogateStopping(Constituents constituents, bool positron)
      : constituents_{constituents}, positron_{positron} {
    ionisation_log_.reserve(constituents.size());

    for (auto const &constituent : constituents) {
      auto const z = static_cast<long double>(constituent.atomic_number);
      ionisation_log_.push_back(
        std::log(1.6e-5L * std::exp(0.9L * std::log(z)) / k_electron_mass_mev));
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

  /*!
   * \brief Evaluates collision and approximate radiative stopping
   * contributions.
   *
   * \param[in] energy Positive working energy in MeV on the integrated branch.
   * \return Surrogate stopping power in MeV/cm.
   */
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

  /*!
   * \brief Evaluates the analytic low-energy surrogate range.
   *
   * \param[in] energy Working energy in MeV at or below the 10 keV branch
   * boundary.
   * \return Surrogate range in centimeters, proportional to energy to the power
   * 3/2.
   */
  [[nodiscard]] auto LowEnergyRange(long double energy) const -> long double {
    return 2.0L * energy * std::sqrt(energy / k_lepton_low_energy_mev) /
           (3.0L * low_branch_at_low_energy_);
  }

  /*!
   * \brief Inverts the analytic low-energy range relation.
   *
   * \param[in] length Nonnegative surrogate range in centimeters on the low
   * branch.
   * \return The corresponding uncorrected energy in MeV.
   */
  [[nodiscard]] auto InverseLowEnergyRange(long double length) const
    -> long double {
    return std::cbrt(std::pow(1.5L * length * low_branch_at_low_energy_, 2.0L) *
                     k_lepton_low_energy_mev);
  }

private:
  /*!
   * \brief Evaluates the dimensionless lepton-specific collision term.
   *
   * \param[in] tau Kinetic energy divided by the electron rest energy.
   * \param[in] ionisation_log Logarithm of the element excitation proxy over
   * electron rest energy.
   * \return The Electron or Positron collision factor, including the inverse
   * beta-squared term.
   */
  [[nodiscard]] auto CollisionTerm(long double tau,
                                   long double ionisation_log) const
    -> long double {
    auto const t1 = tau + 1.0L;
    auto const t2 = tau + 2.0L;
    auto const tsq = tau * tau;
    auto const beta2 = tau * t2 / (t1 * t1);
    auto const f =
      positron_
        ? (2.0L * std::log(tau)) -
            (((6.0L * tau) + (1.5L * tsq) - (tau * (1.0L - (tsq / 3.0L)) / t2) -
              (tsq * (0.5L - (tsq / 12.0L)) / (t2 * t2))) /
             (t1 * t1))
        : 1.0L - beta2 + std::log(tsq / 2.0L) +
            ((0.5L + (0.25L * tsq) + ((1.0L + (2.0L * tau)) * std::log(0.5L))) /
             (t1 * t1));
    return (std::log((2.0L * tau) + 4.0L) - (2.0L * ionisation_log) + f) /
           beta2;
  }

  /*! \brief Borrowed elemental EM rows. */
  Constituents constituents_;

  /*! \brief Selects Positron instead of Electron collision formulas. */
  bool positron_;

  /*! \brief Per-element dimensionless excitation logs. */
  std::vector<long double> ionisation_log_;

  /*! \brief 10 keV collision stopping in MeV/cm. */
  long double low_branch_at_low_energy_{0.0L};
};

// =============================================================================
// =============================================================================

/*!
 * \brief Integrates inverse stopping over one logarithmic energy interval.
 *
 * Five-point Gauss-Legendre quadrature integrates exp(logE)/S(exp(logE))
 * against logE, giving a range increment.
 *
 * \param[in] stopping Lepton surrogate returning MeV/cm.
 * \param[in] lower_log Natural logarithm of the lower energy expressed in MeV.
 * \param[in] upper_log Natural logarithm of the upper energy expressed in MeV.
 * \return The surrogate range increment in centimeters.
 */
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

/*!
 * \brief Inverts a lepton surrogate range and applies its low-energy density
 * correction.
 *
 * Uses the analytic range through 10 keV, then logarithmic quadrature cells and
 * bisection up to 10 GeV. Below 30 keV the provisional energy receives the
 * converter density correction. Admission checks 990 eV both before and after
 * that correction; no endpoint clamp is used.
 *
 * \param[in] constituents Nonempty elemental EM contributions.
 * \param[in] density_g_cm3 Positive bulk material density in g/cm3.
 * \param[in] length_cm Target cut length in centimeters.
 * \param[in] positron True for Positron conversion; false for Electron
 * conversion.
 * \return The corrected unquantized threshold energy in MeV.
 * \throws GGEMSRecoverable If the provisional or corrected energy is below 990
 * eV, or the range exceeds the 10 GeV endpoint.
 */
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

  if (provisional < min_energy) {
    throw GGEMSRecoverable{std::format(
      "{} Production Cut resolves below the 0.99 keV converter domain.",
      particle)};
  }

  auto energy = provisional;
  if (provisional < k_density_correction_energy_mev) {
    energy =
      provisional /
      (1.0L + ((1.0L - (provisional / k_density_correction_energy_mev)) *
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

/*!
 * \brief Requires nonempty elemental matter for Gamma or lepton conversion.
 *
 * \param[in] constituents Borrowed elemental contribution span.
 * \return The same borrowed span.
 * \throws GGEMSRecoverable If constituents is empty.
 */
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

/*!
 * \brief Quantizes an admitted MeV result once through the central Units
 * conversion.
 *
 * \pre Units conversion must succeed; its result is dereferenced without a
 * check.
 *
 * \param[in] energy_mev Finite nonnegative threshold in MeV within canonical
 * Energy range.
 * \return Nearest canonical integer micro-eV, with halfway values rounded away
 * from zero.
 */
[[nodiscard]] auto QuantizeConvertedEnergy(long double energy_mev)
  -> units::Energy {
  return *units::MakeQuantity<units::Energy>(energy_mev, "MeV");
}

// =============================================================================
// =============================================================================

/*!
 * \brief Applies the material-independent 100 keV per mm recoil-cut convention.
 *
 * \param[in] length Cut length in canonical integer pm; zero is accepted.
 * \return The converted canonical integer micro-eV threshold.
 * \throws GGEMSRecoverable If Units cannot represent the resulting Energy.
 */
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
  materials::GGEMSEMMaterialPackage const &materials, std::uint32_t material_id)
  -> units::Energy {
  auto const descriptors = materials.GetDescriptors();

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
