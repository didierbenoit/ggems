#include <algorithm>
#include <cmath>
#include <cstddef>
#include <cstdint>
#include <functional>
#include <limits>
#include <numbers>
#include <string>
#include <string_view>
#include <type_traits>
#include <utility>
#include <vector>

#include <gtest/gtest.h>

#include "GGEMS/GGEMSException.hh"
#include "GGEMS/materials/GGEMSEMMaterialPackage.hh"
#include "GGEMS/materials/GGEMSMaterial.hh"
#include "GGEMS/materials/builtins/GGEMSBuiltInMaterials.hh"
#include "GGEMS/processes/GGEMSProductionCutConverter.hh"
#include "GGEMS/processes/GGEMSProductionCutPolicy.hh"
#include "GGEMS/units/GGEMSEnergyUnits.hh"
#include "GGEMS/units/GGEMSLengthUnits.hh"
#include "GGEMS/units/GGEMSUnitConversion.hh"

namespace {

// =============================================================================
// =============================================================================

namespace materials = ggems::core::materials;
namespace processes = ggems::core::processes;
namespace units = ggems::units;

using namespace ggems::units;
using Channel = processes::GGEMSProductionCutChannel;

// Material IDs of the package built by MakeC01Package().
constexpr std::uint32_t k_water{0U};
constexpr std::uint32_t k_aluminum{1U};
constexpr std::uint32_t k_tungsten{2U};
constexpr std::uint32_t k_vacuum{3U};

constexpr long double k_min_mev{0.99e-3L};
constexpr long double k_max_mev{1.0e4L};
constexpr long double k_electron_mass_mev{0.510998910L};

// Numeric acceptance: 1 µeV (final quantization) plus a relative range
// allowance of 1e-12 amplified by the inversion conditioning d ln E / d ln R.
constexpr long double k_range_relative_allowance{1.0e-12L};

auto TwoPiMc2Rcl2() -> long double {
  constexpr long double e_si{1.602176634e-19L};
  constexpr long double c_light{299792458.0L};
  // r_e = e^2 / (4 pi eps0 m c^2), eps0 = 1 / (c^2 mu0), mu0 = 4 pi 1e-7.
  long double const radius_cm = e_si * 1.0e-7L * c_light * c_light /
                                (k_electron_mass_mev * 1.0e6L) * 100.0L;
  return 2.0L * std::numbers::pi_v<long double> * k_electron_mass_mev *
         radius_cm * radius_cm;
}

struct OracleElement {
  long double z;
  long double density;
};

struct OracleMaterial {
  std::vector<OracleElement> elements;
  long double density_g_cm3;
};

auto ReadMaterial(materials::GGEMSEMMaterialPackage const &package,
                  std::uint32_t material_id) -> OracleMaterial {
  auto const &descriptor = package.GetDescriptors()[material_id];
  OracleMaterial material{
    .elements = {},
    .density_g_cm3 = *units::ConvertTo(descriptor.density, "g/cm3"),
  };
  for (auto const &constituent : package.GetElementalConstituents().subspan(
         descriptor.first_constituent, descriptor.constituent_count)) {
    material.elements.push_back({
      .z = static_cast<long double>(constituent.atomic_number),
      .density = constituent.number_density_per_cubic_centimeter,
    });
  }
  return material;
}

auto GammaSigmaBarn(long double z, long double energy) -> long double {
  long double const lz = std::log(z);
  long double const tmin = 0.552L + 218.5L / z + 557.17L / (z * z);
  long double const tlow = 0.2L * std::exp(-7.355L / std::sqrt(z));
  long double const smin =
    (0.01239L + 0.005585L * lz - 0.000923L * lz * lz) * std::exp(1.5L * lz);
  long double const s200 =
    (0.2651L - 0.1501L * lz + 0.02283L * lz * lz) * z * z;
  long double const cmin =
    std::log(s200 / smin) / std::pow(std::log(tmin / 0.2L), 2.0L);
  long double const slow =
    s200 * std::exp(0.042L * z * std::pow(std::log(0.2L / tlow), 2.0L));
  long double const clow =
    std::log(300.0L * z * z / slow) / std::log(tlow / 1e-3L);
  long double const chigh =
    (7.55e-5L - 0.0542e-5L * z) * z * z * z / std::log(100.0L / tmin);

  if (energy < tlow) {
    return slow * std::pow(tlow / std::max(energy, 1e-3L), clow);
  }
  if (energy < 0.2L) {
    return s200 *
           std::exp(0.042L * z * std::pow(std::log(0.2L / energy), 2.0L));
  }
  if (energy < tmin) {
    return smin * std::exp(cmin * std::pow(std::log(tmin / energy), 2.0L));
  }
  return smin + (chigh * std::log(energy / tmin));
}

auto GammaRange(OracleMaterial const &material, long double energy)
  -> long double {
  long double sigma{0.0L};
  for (auto const &element : material.elements) {
    sigma += element.density * GammaSigmaBarn(element.z, energy) * 1.0e-24L;
  }
  return 5.0L / sigma;
}

auto LeptonStopping(OracleMaterial const &material, long double energy,
                    bool positron, bool low_branch = false) -> long double {
  long double const tau_low = 0.01L / k_electron_mass_mev;
  long double const tau = energy / k_electron_mass_mev;
  bool const low = low_branch || tau < tau_low;
  long double const t = low ? tau_low : tau;
  long double const beta2 = t * (t + 2.0L) / ((t + 1.0L) * (t + 1.0L));
  long double const f =
    positron
      ? 2.0L * std::log(t) -
          (6.0L * t + 1.5L * t * t - t * (1.0L - t * t / 3.0L) / (t + 2.0L) -
           t * t * (0.5L - t * t / 12.0L) / ((t + 2.0L) * (t + 2.0L))) /
            ((t + 1.0L) * (t + 1.0L))
      : 1.0L - beta2 + std::log(t * t / 2.0L) +
          (0.5L + 0.25L * t * t + (1.0L + 2.0L * t) * std::log(0.5L)) /
            ((t + 1.0L) * (t + 1.0L));

  long double sum{0.0L};
  for (auto const &element : material.elements) {
    long double const z = element.z;
    long double const ionisation =
      1.6e-5L * std::pow(z, 0.9L) / k_electron_mass_mev;
    long double dedx =
      z * (std::log(2.0L * t + 4.0L) - 2.0L * std::log(ionisation) + f) / beta2;
    if (low) {
      dedx *= std::sqrt(tau_low / tau);
    } else {
      long double const cbrem =
        (0.02L - 5.7e-5L * z) * (1.0L + 0.072L * std::log(energy / 1000.0L));
      dedx += z * (z + 1.0L) * cbrem * 0.1L * tau / beta2;
    }
    sum += element.density * dedx;
  }
  return sum * TwoPiMc2Rcl2();
}

auto AdaptiveSimpson(std::function<long double(long double)> const &function,
                     long double lower, long double upper, long double fa,
                     long double fm, long double fb, long double whole,
                     long double tolerance, int depth) -> long double {
  long double const middle = (lower + upper) / 2.0L;
  long double const left_middle = (lower + middle) / 2.0L;
  long double const right_middle = (middle + upper) / 2.0L;
  long double const flm = function(left_middle);
  long double const frm = function(right_middle);
  long double const left = (middle - lower) / 6.0L * (fa + 4.0L * flm + fm);
  long double const right = (upper - middle) / 6.0L * (fm + 4.0L * frm + fb);
  if (depth <= 0 || std::fabs(left + right - whole) <= 15.0L * tolerance) {
    return left + right + (left + right - whole) / 15.0L;
  }
  return AdaptiveSimpson(function, lower, middle, fa, flm, fm, left,
                         tolerance / 2.0L, depth - 1) +
         AdaptiveSimpson(function, middle, upper, fm, frm, fb, right,
                         tolerance / 2.0L, depth - 1);
}

auto Integrate(std::function<long double(long double)> const &function,
               long double lower, long double upper, long double tolerance)
  -> long double {
  long double const fa = function(lower);
  long double const fb = function(upper);
  long double const fm = function((lower + upper) / 2.0L);
  long double const whole = (upper - lower) / 6.0L * (fa + 4.0L * fm + fb);
  return AdaptiveSimpson(function, lower, upper, fa, fm, fb, whole,
                         tolerance * std::fabs(whole), 30);
}

auto LeptonRange(OracleMaterial const &material, long double energy,
                 bool positron) -> long double {
  // Below 10 keV, E = x^2 removes the E^-1/2 stopping singularity.
  long double const low_upper = std::sqrt(std::min(energy, 0.01L));
  long double range = Integrate(
    [&](long double x) -> long double {
      return x == 0.0L
               ? 0.0L
               : 2.0L * x / LeptonStopping(material, x * x, positron, true);
    },
    0.0L, low_upper, 1e-14L);
  if (energy <= 0.01L) {
    return range;
  }
  // Decade-split adaptive Simpson in u = ln E above 10 keV.
  long double lower = 0.01L;
  while (lower < energy) {
    long double const upper = std::min(lower * 10.0L, energy);
    range += Integrate(
      [&](long double u) -> long double {
        long double const e = std::exp(u);
        return e / LeptonStopping(material, e, positron);
      },
      std::log(lower), std::log(upper), 1e-14L);
    lower = upper;
  }
  return range;
}

enum class OracleStatus : std::uint8_t { Admitted, BelowDomain, AboveDomain };

struct OracleResult {
  OracleStatus status;
  long double energy_mev;
  long double conditioning;
};

auto OracleGamma(OracleMaterial const &material, long double length_cm)
  -> OracleResult {
  long double const min_range = GammaRange(material, k_min_mev);
  if (min_range > length_cm) {
    return {.status = OracleStatus::BelowDomain,
            .energy_mev = 0.0L,
            .conditioning = 0.0L};
  }
  // Every element cross section is non-increasing on [0.99 keV, 2.99 MeV]
  // (strictly decreasing from 1 keV), so every mixture proxy is non-decreasing.
  long double lower = std::log(k_min_mev);
  long double upper = std::log(2.99L);
  if (GammaRange(material, std::exp(upper)) < length_cm) {
    ADD_FAILURE() << "C01 Gamma oracle point outside its monotone interval.";
    return {.status = OracleStatus::AboveDomain,
            .energy_mev = 0.0L,
            .conditioning = 0.0L};
  }
  for (int iteration = 0; iteration < 200; ++iteration) {
    long double const middle = (lower + upper) / 2.0L;
    if (GammaRange(material, std::exp(middle)) >= length_cm) {
      upper = middle;
    } else {
      lower = middle;
    }
  }
  long double const energy = std::exp(upper);
  long double const step = 1.0e-6L;
  long double const slope =
    (std::log(GammaRange(material, energy * std::exp(step))) -
     std::log(GammaRange(material, energy * std::exp(-step)))) /
    (2.0L * step);
  return {.status = OracleStatus::Admitted,
          .energy_mev = energy,
          .conditioning = 1.0L / slope};
}

auto OracleLepton(OracleMaterial const &material, long double length_cm,
                  bool positron) -> OracleResult {
  if (LeptonRange(material, k_max_mev, positron) < length_cm) {
    return {.status = OracleStatus::AboveDomain,
            .energy_mev = 0.0L,
            .conditioning = 0.0L};
  }
  // Safeguarded Newton on R(E) - L with dR/dE = 1 / S(E).
  long double lower = 0.0L;
  long double upper = k_max_mev;
  long double energy = 0.01L;
  for (int iteration = 0; iteration < 100; ++iteration) {
    long double const residual =
      LeptonRange(material, energy, positron) - length_cm;
    if (residual >= 0.0L) {
      upper = energy;
    } else {
      lower = energy;
    }
    long double next =
      energy - residual * LeptonStopping(material, energy, positron);
    if (!(next > lower && next < upper)) {
      next = (lower + upper) / 2.0L;
    }
    if (std::fabs(next - energy) <=
        4.0L * std::numeric_limits<long double>::epsilon() * energy) {
      energy = next;
      break;
    }
    energy = next;
  }
  long double conditioning = LeptonRange(material, energy, positron) *
                             LeptonStopping(material, energy, positron) /
                             energy;

  long double corrected = energy;
  if (energy < 0.03L) {
    // E_f = E / D with D = 1 + a (1 - E / 30 keV): d ln E_f / d ln E = (1+a)/D.
    long double const tune = 0.0025L / (length_cm * material.density_g_cm3);
    long double const divisor = 1.0L + tune * (1.0L - energy / 0.03L);
    corrected = energy / divisor;
    conditioning *= (1.0L + tune) / divisor;
  }
  if (corrected < k_min_mev) {
    return {.status = OracleStatus::BelowDomain,
            .energy_mev = corrected,
            .conditioning = conditioning};
  }
  return {.status = OracleStatus::Admitted,
          .energy_mev = corrected,
          .conditioning = conditioning};
}

auto Oracle(Channel channel, OracleMaterial const &material,
            units::Length length) -> OracleResult {
  long double const length_cm = *units::ConvertTo(length, "cm");
  switch (channel) {
  case Channel::Gamma:
    return OracleGamma(material, length_cm);
  case Channel::Electron:
    return OracleLepton(material, length_cm, false);
  case Channel::Positron:
    return OracleLepton(material, length_cm, true);
  case Channel::Proton:
    return {.status = OracleStatus::Admitted,
            .energy_mev = 0.1L * length_cm * 10.0L,
            .conditioning = 1.0L};
  }
  return {};
}

// =============================================================================
// =============================================================================

auto MakeC01Package() -> materials::GGEMSEMMaterialPackage {
  std::vector<materials::GGEMSMaterial> const list{
    materials::builtins::BuildBuiltInMaterial("Water"),
    materials::builtins::BuildBuiltInMaterial("Aluminum"),
    materials::builtins::BuildBuiltInMaterial("Tungsten"),
    materials::builtins::BuildBuiltInMaterial("Vacuum"),
  };
  return materials::GGEMSEMMaterialPackage{list};
}

auto Convert(Channel channel, units::Length length,
             materials::GGEMSEMMaterialPackage const &package,
             std::uint32_t material_id) -> units::Energy {
  return processes::ConvertProductionCutLength(channel, length, package,
                                               material_id);
}

auto ChannelLabel(Channel channel) -> std::string_view {
  switch (channel) {
  case Channel::Gamma:
    return "Gamma";
  case Channel::Electron:
    return "Electron";
  case Channel::Positron:
    return "Positron";
  case Channel::Proton:
    return "Proton";
  }
  return "?";
}

} // namespace

// =============================================================================
// =============================================================================

TEST(GGEMSProductionCutConverterTest, ReturnsCanonicalEnergy) {
  static_assert(
    std::is_same_v<decltype(processes::ConvertProductionCutLength(
                     Channel::Proton, units::Length{},
                     std::declval<materials::GGEMSEMMaterialPackage const &>(),
                     0U)),
                   units::Energy>);
}

// =============================================================================
// =============================================================================

TEST(GGEMSProductionCutConverterTest, C01MatchesIndependentOracle) {
  auto const package = MakeC01Package();

  std::size_t admitted{0U};
  std::size_t rejected{0U};

  for (auto const material_id : {k_water, k_aluminum, k_tungsten}) {
    auto const material = ReadMaterial(package, material_id);
    for (auto const channel : processes::k_production_cut_channels) {
      for (auto const length : {1_um, 10_um, 100_um, 1_mm, 10_mm}) {
        SCOPED_TRACE(std::string{ChannelLabel(channel)} + " material " +
                     std::to_string(material_id) + " length_pm " +
                     std::to_string(length.value));

        auto const reference = Oracle(channel, material, length);

        if (reference.status != OracleStatus::Admitted) {
          ++rejected;
          EXPECT_THROW(
            static_cast<void>(Convert(channel, length, package, material_id)),
            ggems::core::GGEMSRecoverable);
          continue;
        }

        ++admitted;
        auto const energy = Convert(channel, length, package, material_id);
        long double const reference_micro_ev = reference.energy_mev * 1.0e12L;
        long double const tolerance = 1.0L + k_range_relative_allowance *
                                               reference.conditioning *
                                               reference_micro_ev;
        EXPECT_LE(std::fabs(static_cast<long double>(energy.value) -
                            reference_micro_ev),
                  tolerance)
          << "production " << energy.value << " µeV, reference "
          << static_cast<double>(reference_micro_ev) << " µeV";
      }
    }
  }

  // The C01 matrix exercises both admitted and rejected domain cases.
  EXPECT_GE(admitted, 50U);
  EXPECT_GE(rejected, 1U);
}

// =============================================================================
// =============================================================================

TEST(GGEMSProductionCutConverterTest,
     SameLengthIsMaterialDependentExceptProton) {
  auto const package = MakeC01Package();

  for (auto const channel : processes::k_production_cut_channels) {
    SCOPED_TRACE(ChannelLabel(channel));

    auto const water = Convert(channel, 1_mm, package, k_water);
    auto const aluminum = Convert(channel, 1_mm, package, k_aluminum);
    auto const tungsten = Convert(channel, 1_mm, package, k_tungsten);

    if (channel == Channel::Proton) {
      EXPECT_EQ(water, aluminum);
      EXPECT_EQ(water, tungsten);
    } else {
      EXPECT_LT(water, aluminum);
      EXPECT_LT(aluminum, tungsten);
    }
  }
}

// =============================================================================
// =============================================================================

TEST(GGEMSProductionCutConverterTest, ProtonIsExactLinearConvention) {
  auto const package = MakeC01Package();

  for (auto const material_id : {k_water, k_tungsten, k_vacuum}) {
    EXPECT_EQ(Convert(Channel::Proton, 1_mm, package, material_id), 100_keV);
    EXPECT_EQ(Convert(Channel::Proton, 0_pm, package, material_id), 0_eV);
    EXPECT_EQ(Convert(Channel::Proton, 1_pm, package, material_id).value, 100U);
    EXPECT_EQ(Convert(Channel::Proton, 25_mm, package, material_id), 2500_keV);
  }
}

// =============================================================================
// =============================================================================

TEST(GGEMSProductionCutConverterTest, ProtonOverflowIsRejected) {
  auto const package = MakeC01Package();

  EXPECT_THROW(
    static_cast<void>(Convert(Channel::Proton, 1000_km, package, k_water)),
    ggems::core::GGEMSRecoverable);
}

// =============================================================================
// =============================================================================

TEST(GGEMSProductionCutConverterTest, OutOfDomainLengthsAreRejected) {
  auto const package = MakeC01Package();

  for (auto const channel :
       {Channel::Gamma, Channel::Electron, Channel::Positron}) {
    for (auto const material_id : {k_water, k_aluminum, k_tungsten}) {
      SCOPED_TRACE(std::string{ChannelLabel(channel)} + " material " +
                   std::to_string(material_id));

      EXPECT_THROW(
        static_cast<void>(Convert(channel, 0_pm, package, material_id)),
        ggems::core::GGEMSRecoverable);
      EXPECT_THROW(
        static_cast<void>(Convert(channel, 1_nm, package, material_id)),
        ggems::core::GGEMSRecoverable);
      EXPECT_THROW(
        static_cast<void>(Convert(channel, 1000_km, package, material_id)),
        ggems::core::GGEMSRecoverable);
    }
  }
}

// =============================================================================
// =============================================================================

TEST(GGEMSProductionCutConverterTest, DomainEdgeIsNotClamped) {
  auto const package = MakeC01Package();
  auto const water = ReadMaterial(package, k_water);

  // Gamma: the lower converter edge is reached when 5 / Sigma_abs(0.99 keV)
  // equals the length. Just below it the crossing lies below the domain.
  long double const edge_cm = GammaRange(water, k_min_mev);
  auto const edge_pm =
    static_cast<std::uint64_t>(std::floor(edge_cm * 1.0e10L));

  EXPECT_THROW(
    static_cast<void>(
      Convert(Channel::Gamma, units::Length{edge_pm - 1U}, package, k_water)),
    ggems::core::GGEMSRecoverable);

  units::Length const above_length{edge_pm + 2U};
  auto const above = Convert(Channel::Gamma, above_length, package, k_water);
  auto const reference = Oracle(Channel::Gamma, water, above_length);
  ASSERT_EQ(reference.status, OracleStatus::Admitted);
  long double const reference_micro_ev = reference.energy_mev * 1.0e12L;
  EXPECT_GE(above, 990_eV);
  EXPECT_LE(
    std::fabs(static_cast<long double>(above.value) - reference_micro_ev),
    1.0L +
      k_range_relative_allowance * reference.conditioning * reference_micro_ev);

  // Electron: find the smallest admitted length by bisection on the public
  // API; below it every length is rejected rather than clamped to 0.99 keV.
  std::uint64_t rejected_pm{1U};
  std::uint64_t admitted_pm{1'000'000'000U};
  ASSERT_THROW(
    static_cast<void>(
      Convert(Channel::Electron, units::Length{rejected_pm}, package, k_water)),
    ggems::core::GGEMSRecoverable);
  ASSERT_NO_THROW(static_cast<void>(
    Convert(Channel::Electron, units::Length{admitted_pm}, package, k_water)));
  while (admitted_pm - rejected_pm > 1U) {
    auto const middle = rejected_pm + (admitted_pm - rejected_pm) / 2U;
    try {
      static_cast<void>(
        Convert(Channel::Electron, units::Length{middle}, package, k_water));
      admitted_pm = middle;
    } catch (ggems::core::GGEMSRecoverable const &) {
      rejected_pm = middle;
    }
  }

  auto const smallest =
    Convert(Channel::Electron, units::Length{admitted_pm}, package, k_water);
  EXPECT_GE(smallest, 990_eV);
  EXPECT_LE(smallest, 991_eV);
}

// =============================================================================
// =============================================================================

TEST(GGEMSProductionCutConverterTest, GammaCrossingNearProxyMaximumIsFound) {
  auto const package = MakeC01Package();
  auto const tungsten = ReadMaterial(package, k_tungsten);

  // For a pure element the proxy 5 / Sigma_abs peaks exactly at tmin, the end
  // of its monotone Gamma converter domain.
  long double const z = 74.0L;
  long double const tmin = 0.552L + 218.5L / z + 557.17L / (z * z);
  long double const peak_cm = GammaRange(tungsten, tmin);

  units::Length const below{static_cast<std::uint64_t>(
    std::floor(peak_cm * (1.0L - 1.0e-7L) * 1.0e10L))};
  units::Length const above{static_cast<std::uint64_t>(
    std::ceil(peak_cm * (1.0L + 1.0e-7L) * 1.0e10L))};

  EXPECT_THROW(
    static_cast<void>(Convert(Channel::Gamma, above, package, k_tungsten)),
    ggems::core::GGEMSRecoverable);

  long double const length_cm = *units::ConvertTo(below, "cm");
  long double lower = std::log(k_min_mev);
  long double upper = std::log(tmin);
  for (int iteration = 0; iteration < 200; ++iteration) {
    long double const middle = (lower + upper) / 2.0L;
    if (GammaRange(tungsten, std::exp(middle)) >= length_cm) {
      upper = middle;
    } else {
      lower = middle;
    }
  }
  long double const reference_micro_ev = std::exp(upper) * 1.0e12L;
  long double const step = 1.0e-6L;
  long double const slope =
    (std::log(GammaRange(tungsten, std::exp(upper + step))) -
     std::log(GammaRange(tungsten, std::exp(upper - step)))) /
    (2.0L * step);

  auto const energy = Convert(Channel::Gamma, below, package, k_tungsten);
  EXPECT_LE(
    std::fabs(static_cast<long double>(energy.value) - reference_micro_ev),
    1.0L + ((k_range_relative_allowance / slope) * reference_micro_ev))
    << "production " << energy.value << " µeV, reference "
    << static_cast<double>(reference_micro_ev) << " µeV";
}

// =============================================================================
// =============================================================================

TEST(GGEMSProductionCutConverterTest, MatterlessMaterialIsRejected) {
  auto const package = MakeC01Package();

  for (auto const channel :
       {Channel::Gamma, Channel::Electron, Channel::Positron}) {
    SCOPED_TRACE(ChannelLabel(channel));
    EXPECT_THROW(static_cast<void>(Convert(channel, 1_mm, package, k_vacuum)),
                 ggems::core::GGEMSRecoverable);
  }
}

// =============================================================================
// =============================================================================

TEST(GGEMSProductionCutConverterTest, UnknownMaterialIdIsRejected) {
  auto const package = MakeC01Package();

  for (auto const channel : processes::k_production_cut_channels) {
    SCOPED_TRACE(ChannelLabel(channel));
    EXPECT_THROW(static_cast<void>(Convert(channel, 1_mm, package, 4U)),
                 ggems::core::GGEMSRecoverable);
  }
}
