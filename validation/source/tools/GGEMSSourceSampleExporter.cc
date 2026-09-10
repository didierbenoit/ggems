#include <iostream>
#include <exception>
#include <string>
#include <cstdint>
#include <array>
#include <filesystem>
#include <charconv>
#include <string_view>
#include <format>
#include <stdexcept>
#include <system_error>
#include <set>
#include <limits>
#include <memory>
#include <vector>
#include <cstddef>
#include <algorithm>
#include <span>
#include <fstream>
#include <iomanip>
#include <locale>
#include <cmath>
#include <tuple>
#include <ostream>
#include <optional>

#include "GGEMS/GGEMSRun.hh"
#include "GGEMS/opencl/GGEMSOpenCL.hh"
#include "GGEMS/opencl/GGEMSOpenCLContext.hh"
#include "GGEMS/opencl/GGEMSOpenCLDevice.hh"
#include "GGEMS/logging/GGEMSLogger.hh"
#include "GGEMS/sources/GGEMSSource.hh"
#include "GGEMS/sources/GGEMSSourceRecord.hh"
#include "GGEMS/sources/GGEMSSourceRunSnapshot.hh"
#include "GGEMS/sources/GGEMSEnergyDistribution.hh"
#include "GGEMS/sources/GGEMSEnergyDistributionRecord.hh"
#include "GGEMS/sources/GGEMSSourceTypes.hh"
#include "GGEMS/particles/GGEMSParticleTypes.hh"
#include "GGEMS/units/GGEMSEnergyUnits.hh"
#include "GGEMS/units/GGEMSUnitConversion.hh"
#include "GGEMS/units/GGEMSLengthUnits.hh"
#include "GGEMS/units/GGEMSAngularUnits.hh"
#include "GGEMS/units/GGEMSTimeUnits.hh"
#include "GGEMS/random/GGEMSRandom.hh"
#include "GGEMS/random/GGEMSRandomEngine.hh"
#include "GGEMS/observer/GGEMSObserverRecord.hh"
#include "GGEMS/observer/GGEMSObserverTypes.hh"
#include "GGEMS/observer/GGEMSTransportObserver.hh"

namespace {

using ggems::core::observer::GGEMSObserverRecord;
using ggems::core::observer::GGEMSObserverRecordKind;
using ggems::core::observer::GGEMSTransportObserver;
using ggems::core::observer::ToKernelObserverRecordKind;
using ggems::core::sources::GGEMSSource;
using ggems::core::sources::GGEMSSourceRecord;
using ggems::core::sources::GGEMSSourceRunSnapshot;

// =============================================================================
// =============================================================================

struct Options {
  std::string device_selector;
  std::string geometry;
  std::string case_name;
  std::uint64_t primary_count{};
  std::uint32_t worker_count{};
  std::uint64_t seed{};
  std::array<std::uint64_t, 3U> dimensions_pm{};
  std::array<long double, 3U> requested_center_mm{};
  std::array<std::int64_t, 3U> center_pm{};
  std::array<double, 3U> frame_direction{};
  std::array<double, 3U> frame_up{};
  bool has_orientation{false};
  std::string angular{"fixed"};
  // Requested theta min/max, then phi min/max, before central Units conversion.
  std::array<long double, 4U> angular_bounds_deg{};
  std::array<std::int64_t, 3U> focus_pm{};
  std::string energy_mode{"mono"};
  long double mono_energy_kev{511.0L};
  std::vector<double> energy_values_kev;
  std::vector<double> energy_weights;
  long double energy_bin_width_kev{};
  std::string chronology{"static"};
  // Requested start/stop/step in ns and the central Units conversion in ps.
  std::array<long double, 3U> requested_time_ns{};
  std::array<std::uint64_t, 3U> configured_time_ps{};
  std::uint32_t sequence_runs{1U};
  std::optional<std::uint32_t> reset_before_run;
  std::filesystem::path sequence_path;
  std::filesystem::path output_path;
  std::filesystem::path metadata_path;
  bool help{false};
};

// =============================================================================
// =============================================================================

auto PrintUsage() -> void {
  std::cout
      << "GGEMS Source / G1, A1, E1, T1, Frame G2/A2 exporter\n"
      << "Required: --device <GGEMS selector> --geometry <name>\n"
      << "          --output <samples.csv> --metadata <metadata.json>\n"
      << "          --primaries <uint64> --workers <uint32> --seed <uint64>\n"
      << "Geometry: point|rectangle|ellipse|circle|box|sphere|cylinder\n"
      << "Options:  --case-name <name> (default G1_<geometry>)\n"
      << "          --size-x-mm <value> --size-y-mm <value>\n"
      << "          --size-z-mm <value> (default sizes are zero)\n"
      << "Supply complete widths/diameters/heights: circle X=Y, sphere\n"
      << "X=Y=Z, cylinder X=Y and Z=height; planar Z and Point sizes=0.\n"
      << "Center:   --center-x-mm <value> --center-y-mm <value>\n"
      << "          --center-z-mm <value> (all three, default origin)\n"
      << "Frame:    --frame-direction-x <value> --frame-direction-y <value>\n"
      << "          --frame-direction-z <value> --frame-up-x <value>\n"
      << "          --frame-up-y <value> --frame-up-z <value>\n"
      << "All six frame components are required together. They are passed\n"
      << "to Source::SetOrientation(direction, up_reference); no arguments\n"
      << "keep the default identity frame. Packed axes come from the "
         "snapshot.\n"
      << "          --angular fixed|isotropic|bounded-isotropic|focused\n"
      << "          (default fixed; isotropic uses the no-argument API)\n"
      << "Bounded:  --theta-min-deg <value> --theta-max-deg <value>\n"
      << "          --phi-min-deg <value> --phi-max-deg <value>\n"
      << "Focused:  --focus-x-mm <value> --focus-y-mm <value>\n"
      << "          --focus-z-mm <value> (global focus)\n"
      << "All four bounds or all three focus coordinates are required for\n"
      << "their mode; angular parameters in another mode are rejected.\n"
      << "Energy:   --energy-mode mono|discrete-lines|regular-spectrum\n"
      << "          (default mono, 511 keV)\n"
      << "Mono:     --mono-energy-kev <value>\n"
      << "Tables:   --energy-values-kev <ordered comma-separated values>\n"
      << "          --energy-weights <comma-separated relative weights>\n"
      << "Regular:  --energy-bin-width-kev <full bin width>\n"
      << "Regular values are centers; width must equal their canonical\n"
      << "spacing. Lists require matching counts, no empty fields/spaces.\n"
      << "Energy parameters belonging to another mode are rejected.\n"
      << "Time:     --chronology static|configured (default static)\n"
      << "          --time-start-ns <value> --time-stop-ns <value>\n"
      << "          --time-step-ns <value> (all required only if configured)\n"
      << "Sequence: --sequence-dir <new directory> instead of --output\n"
      << "          --sequence-runs <positive uint32> (default 1)\n"
      << "          --reset-before-run <zero-based index> (optional)\n"
      << "A sequence initializes one Run once, writes one CSV per Run,\n"
      << "and writes an ordered manifest to --metadata. Reset requires\n"
      << "configured chronology and an index in [1, sequence-runs).\n"
      << "CountDriven births equal the effective window start exactly.\n"
      << "Capture requires 2*N <= UINT32_MAX with the current Observer.\n"
      << "One fresh CountDriven Gamma Source, default identity frame/origin,\n"
      << "default Fixed +Z, Mono 511 keV, static 0 ps, weight 1, Philox "
         "only.\n";
}

// =============================================================================
// =============================================================================

auto ParseUnsigned(std::string_view text, std::string_view option)
    -> std::uint64_t {
  std::uint64_t value{};

  auto const result =
      std::from_chars(text.data(), text.data() + text.size(), value);

  if (result.ec != std::errc{} || result.ptr != text.data() + text.size()) {
    throw std::runtime_error(
        std::format("Invalid uint64 for {}: '{}'.", option, text));
  }

  return value;
}

// =============================================================================
// =============================================================================

auto ParseNumber(std::string_view text, std::string_view option)
    -> long double {
  long double value{};

  auto const result =
      std::from_chars(text.data(), text.data() + text.size(), value);

  if (result.ec != std::errc{} || result.ptr != text.data() + text.size() ||
      !std::isfinite(value)) {
    throw std::runtime_error(
        std::format("Invalid number for {}: '{}'.", option, text));
  }

  return value;
}

// =============================================================================
// =============================================================================

auto ParseLength(std::string_view text, std::string_view option)
    -> std::uint64_t {
  auto const value = ParseNumber(text, option);
  auto const converted =
      ggems::units::MakeQuantity<ggems::units::Length>(value, "mm");

  if (!converted) {
    throw std::runtime_error(std::format(
        "{} must be a finite nonnegative length representable in pm.", option));
  }

  return converted->value;
}

// =============================================================================
// =============================================================================

auto ParsePosition(std::string_view text, std::string_view option)
    -> std::int64_t {
  auto const converted =
      ggems::units::MakeQuantity<ggems::units::PositionCoordinate>(
          ParseNumber(text, option), "mm");

  if (!converted) {
    throw std::runtime_error(std::format(
        "{} must be a finite signed position representable in pm.", option));
  }

  return converted->value;
}

// =============================================================================
// =============================================================================

auto ParseEnergyList(std::string_view text, std::string_view option)
    -> std::vector<double> {
  std::vector<double> values;

  while (true) {
    auto const comma = text.find(',');
    auto const field = text.substr(0U, comma);
    double value{};
    auto const parsed =
        std::from_chars(field.data(), field.data() + field.size(), value);

    if (field.empty() || parsed.ec != std::errc{} ||
        parsed.ptr != field.data() + field.size() || !std::isfinite(value) ||
        value < 0.0) {
      throw std::runtime_error(std::format(
          "{} requires comma-separated finite nonnegative numbers.", option));
    }

    values.push_back(value);
    if (comma == std::string_view::npos) {
      return values;
    }
    text.remove_prefix(comma + 1U);
  }
}

// =============================================================================
// =============================================================================

auto CanonicalEnergy(long double value, std::string_view option)
    -> std::uint64_t {
  auto const converted =
      ggems::units::MakeQuantity<ggems::units::Energy>(value, "keV");

  if (!converted || converted->value == 0ULL) {
    throw std::runtime_error(std::format(
        "{} must convert to a positive representable energy in micro-eV.",
        option));
  }

  return converted->value;
}

// =============================================================================
// =============================================================================

auto ParseFrameComponent(std::string_view text, std::string_view option)
    -> double {
  auto const value = ParseNumber(text, option);
  if (std::abs(value) > std::numeric_limits<double>::max()) {
    throw std::runtime_error(
        std::format("{} must be representable in binary64.", option));
  }
  return static_cast<double>(value);
}

// =============================================================================
// =============================================================================

auto ValidatePoseOptions(Options &options,
                         std::set<std::string_view> const &seen) -> void {
  bool const has_center = seen.contains("--center-x-mm") ||
                          seen.contains("--center-y-mm") ||
                          seen.contains("--center-z-mm");
  for (auto const *option :
       {"--center-x-mm", "--center-y-mm", "--center-z-mm"}) {
    if (seen.contains(option) != has_center) {
      throw std::runtime_error("Supply all three center coordinates together.");
    }
  }

  for (std::size_t axis = 0U; axis < options.center_pm.size(); ++axis) {
    auto const converted =
        ggems::units::MakeQuantity<ggems::units::PositionCoordinate>(
            options.requested_center_mm[axis], "mm");
    if (!converted) {
      throw std::runtime_error(
          "Source center must be representable in int64 pm.");
    }
    options.center_pm[axis] = converted->value;
  }

  constexpr std::array<std::string_view, 6U> k_frame_options{
      "--frame-direction-x", "--frame-direction-y", "--frame-direction-z",
      "--frame-up-x",        "--frame-up-y",        "--frame-up-z"};
  options.has_orientation =
      std::ranges::any_of(k_frame_options, [&seen](auto option) -> bool {
        return seen.contains(option);
      });
  for (auto const option : k_frame_options) {
    if (seen.contains(option) != options.has_orientation) {
      throw std::runtime_error(
          "Supply all six direction/up components together.");
    }
  }
}

// =============================================================================
// =============================================================================

auto ParseArguments(int argc, char const *const *argv) -> Options {
  Options options;
  std::set<std::string_view> seen;

  for (int index = 1; index < argc; ++index) {
    std::string_view const option{argv[index]};

    if (option == "--help") {
      options.help = true;
      return options;
    }

    if (!seen.insert(option).second) {
      throw std::runtime_error(std::format("Repeated option '{}'.", option));
    }

    if (index + 1 >= argc) {
      throw std::runtime_error(
          std::format("Missing value after '{}'.", option));
    }

    std::string_view const value{argv[++index]};

    if (option == "--device") {
      options.device_selector = value;
    } else if (option == "--geometry") {
      options.geometry = value;
    } else if (option == "--case-name") {
      options.case_name = value;
    } else if (option == "--primaries") {
      options.primary_count = ParseUnsigned(value, option);
    } else if (option == "--seed") {
      options.seed = ParseUnsigned(value, option);
    } else if (option == "--workers") {
      auto const workers = ParseUnsigned(value, option);
      if (workers == 0ULL ||
          workers > std::numeric_limits<std::uint32_t>::max()) {
        throw std::runtime_error("--workers must be a positive uint32.");
      }
      options.worker_count = static_cast<std::uint32_t>(workers);
    } else if (option == "--size-x-mm") {
      options.dimensions_pm[0U] = ParseLength(value, option);
    } else if (option == "--size-y-mm") {
      options.dimensions_pm[1U] = ParseLength(value, option);
    } else if (option == "--size-z-mm") {
      options.dimensions_pm[2U] = ParseLength(value, option);
    } else if (option == "--center-x-mm") {
      options.requested_center_mm[0U] = ParseNumber(value, option);
    } else if (option == "--center-y-mm") {
      options.requested_center_mm[1U] = ParseNumber(value, option);
    } else if (option == "--center-z-mm") {
      options.requested_center_mm[2U] = ParseNumber(value, option);
    } else if (option == "--frame-direction-x") {
      options.frame_direction[0U] = ParseFrameComponent(value, option);
    } else if (option == "--frame-direction-y") {
      options.frame_direction[1U] = ParseFrameComponent(value, option);
    } else if (option == "--frame-direction-z") {
      options.frame_direction[2U] = ParseFrameComponent(value, option);
    } else if (option == "--frame-up-x") {
      options.frame_up[0U] = ParseFrameComponent(value, option);
    } else if (option == "--frame-up-y") {
      options.frame_up[1U] = ParseFrameComponent(value, option);
    } else if (option == "--frame-up-z") {
      options.frame_up[2U] = ParseFrameComponent(value, option);
    } else if (option == "--angular") {
      options.angular = value;
    } else if (option == "--theta-min-deg") {
      options.angular_bounds_deg[0U] = ParseNumber(value, option);
    } else if (option == "--theta-max-deg") {
      options.angular_bounds_deg[1U] = ParseNumber(value, option);
    } else if (option == "--phi-min-deg") {
      options.angular_bounds_deg[2U] = ParseNumber(value, option);
    } else if (option == "--phi-max-deg") {
      options.angular_bounds_deg[3U] = ParseNumber(value, option);
    } else if (option == "--focus-x-mm") {
      options.focus_pm[0U] = ParsePosition(value, option);
    } else if (option == "--focus-y-mm") {
      options.focus_pm[1U] = ParsePosition(value, option);
    } else if (option == "--focus-z-mm") {
      options.focus_pm[2U] = ParsePosition(value, option);
    } else if (option == "--energy-mode") {
      options.energy_mode = value;
    } else if (option == "--mono-energy-kev") {
      options.mono_energy_kev = ParseNumber(value, option);
    } else if (option == "--energy-values-kev") {
      options.energy_values_kev = ParseEnergyList(value, option);
    } else if (option == "--energy-weights") {
      options.energy_weights = ParseEnergyList(value, option);
    } else if (option == "--energy-bin-width-kev") {
      options.energy_bin_width_kev = ParseNumber(value, option);
    } else if (option == "--chronology") {
      options.chronology = value;
    } else if (option == "--time-start-ns") {
      options.requested_time_ns[0U] = ParseNumber(value, option);
    } else if (option == "--time-stop-ns") {
      options.requested_time_ns[1U] = ParseNumber(value, option);
    } else if (option == "--time-step-ns") {
      options.requested_time_ns[2U] = ParseNumber(value, option);
    } else if (option == "--sequence-dir") {
      options.sequence_path = value;
    } else if (option == "--sequence-runs" || option == "--reset-before-run") {
      auto const sequence_value = ParseUnsigned(value, option);
      if (sequence_value > std::numeric_limits<std::uint32_t>::max()) {
        throw std::runtime_error(std::format("{} requires a uint32.", option));
      }
      if (option == "--sequence-runs") {
        options.sequence_runs = static_cast<std::uint32_t>(sequence_value);
      } else {
        options.reset_before_run = static_cast<std::uint32_t>(sequence_value);
      }
    } else if (option == "--output") {
      options.output_path = value;
    } else if (option == "--metadata") {
      options.metadata_path = value;
    } else {
      throw std::runtime_error(std::format("Unknown option '{}'.", option));
    }
  }

  if (options.device_selector.empty() || options.geometry.empty() ||
      options.metadata_path.empty() || !seen.contains("--primaries") ||
      !seen.contains("--workers") || !seen.contains("--seed")) {
    throw std::runtime_error(
        "--device, --geometry, --metadata, --primaries, --workers "
        "and --seed are required.");
  }

  if (options.primary_count == 0ULL ||
      options.primary_count >
          std::numeric_limits<std::uint32_t>::max() / 2ULL) {
    throw std::runtime_error(
        "--primaries must be in [1, 2147483647]: the current Observer needs "
        "uint32 capacity for Source and Terminal records.");
  }

  if (options.output_path.empty() == options.sequence_path.empty() ||
      (seen.contains("--output") && seen.contains("--sequence-dir"))) {
    throw std::runtime_error(
        "Supply exactly one of --output and --sequence-dir.");
  }

  if (!options.output_path.empty() &&
      std::filesystem::absolute(options.output_path).lexically_normal() ==
          std::filesystem::absolute(options.metadata_path).lexically_normal()) {
    throw std::runtime_error("Sample and metadata paths must differ.");
  }

  if (options.case_name.empty()) {
    options.case_name = "G1_" + options.geometry;
  }

  ValidatePoseOptions(options, seen);

  if (options.angular != "fixed" && options.angular != "isotropic" &&
      options.angular != "bounded-isotropic" && options.angular != "focused") {
    throw std::runtime_error(std::format(
        "Unsupported angular configuration '{}'.", options.angular));
  }

  for (auto const *option : {"--theta-min-deg", "--theta-max-deg",
                             "--phi-min-deg", "--phi-max-deg"}) {
    if (seen.contains(option) != (options.angular == "bounded-isotropic")) {
      throw std::runtime_error(std::format(
          "{} is required only with --angular bounded-isotropic.", option));
    }
  }

  for (auto const *option : {"--focus-x-mm", "--focus-y-mm", "--focus-z-mm"}) {
    if (seen.contains(option) != (options.angular == "focused")) {
      throw std::runtime_error(
          std::format("{} is required only with --angular focused.", option));
    }
  }

  if (options.energy_mode != "mono" &&
      options.energy_mode != "discrete-lines" &&
      options.energy_mode != "regular-spectrum") {
    throw std::runtime_error(
        std::format("Unsupported energy mode '{}'.", options.energy_mode));
  }

  bool const tabulated = options.energy_mode != "mono";
  for (auto const *option : {"--energy-values-kev", "--energy-weights"}) {
    if (seen.contains(option) != tabulated) {
      throw std::runtime_error(std::format(
          "{} is required only for a tabulated energy mode.", option));
    }
  }

  if (tabulated && seen.contains("--mono-energy-kev")) {
    throw std::runtime_error("--mono-energy-kev is only valid in Mono mode.");
  }

  if (seen.contains("--energy-bin-width-kev") !=
      (options.energy_mode == "regular-spectrum")) {
    throw std::runtime_error(
        "--energy-bin-width-kev is required only for regular-spectrum.");
  }

  if (tabulated &&
      (options.energy_values_kev.size() < 2U ||
       options.energy_values_kev.size() != options.energy_weights.size())) {
    throw std::runtime_error(
        "Energy values and weights require matching counts of at least two.");
  }

  if (options.chronology != "static" && options.chronology != "configured") {
    throw std::runtime_error("--chronology must be static or configured.");
  }

  for (auto const *option :
       {"--time-start-ns", "--time-stop-ns", "--time-step-ns"}) {
    if (seen.contains(option) != (options.chronology == "configured")) {
      throw std::runtime_error(std::format(
          "{} is required only with --chronology configured.", option));
    }
  }

  if (options.chronology == "configured") {
    auto const start = ggems::units::MakeQuantity<ggems::units::TimePoint>(
        options.requested_time_ns[0U], "ns");
    auto const stop = ggems::units::MakeQuantity<ggems::units::TimePoint>(
        options.requested_time_ns[1U], "ns");
    auto const step = ggems::units::MakeQuantity<ggems::units::Duration>(
        options.requested_time_ns[2U], "ns");
    if (!start || !stop || !step) {
      throw std::runtime_error(
          "Time values must be representable in uint64 ps.");
    }
    options.configured_time_ps = {start->value, stop->value, step->value};
  }

  if (options.sequence_runs == 0U ||
      (options.sequence_path.empty() &&
       (seen.contains("--sequence-runs") || options.reset_before_run))) {
    throw std::runtime_error(
        "A positive --sequence-runs requires --sequence-dir.");
  }

  if (options.reset_before_run &&
      (options.chronology != "configured" || *options.reset_before_run == 0U ||
       *options.reset_before_run >= options.sequence_runs)) {
    throw std::runtime_error("Reset requires configured chronology and an "
                             "index in [1, sequence-runs).");
  }

  if (!options.sequence_path.empty() &&
      (std::filesystem::exists(options.sequence_path) ||
       std::filesystem::exists(options.metadata_path))) {
    throw std::runtime_error(
        "Sequence directory and manifest must be new outputs.");
  }

  if (!options.sequence_path.empty()) {
    auto const relative_metadata =
        std::filesystem::absolute(options.metadata_path)
            .lexically_normal()
            .lexically_relative(std::filesystem::absolute(options.sequence_path)
                                    .lexically_normal());
    if (!relative_metadata.empty() && *relative_metadata.begin() != "..") {
      throw std::runtime_error(
          "Keep the manifest outside the sequence CSV directory.");
    }
  }

  return options;
}

// =============================================================================
// =============================================================================

auto ConfigureSource(Options const &options) -> std::shared_ptr<GGEMSSource> {
  auto source = std::make_shared<GGEMSSource>();

  source->SetAnalytic()
      .SetCountDrivenPopulation(options.primary_count)
      .SetEmittedParticleType(ggems::core::particles::GGEMSParticleType::Gamma)
      .SetFixedAngularDistribution()
      .SetWeight(1.0F);

  source->SetPositionPicoMeter(options.center_pm[0U], options.center_pm[1U],
                               options.center_pm[2U]);

  // Production owns normalization, orthogonalization, validation and packing.
  if (options.has_orientation) {
    source->SetOrientation(options.frame_direction, options.frame_up);
  }

  // The public Source builders own conversion, ordering, grids, and tickets.
  // All configuration errors occur before OpenCL setup or output creation.
  if (options.energy_mode == "mono") {
    source->SetEnergyMicroElectronVolt(
        CanonicalEnergy(options.mono_energy_kev, "--mono-energy-kev"));
  } else if (options.energy_mode == "discrete-lines") {
    source->SetDiscreteEnergyLines(options.energy_values_kev,
                                   options.energy_weights, "keV");
  } else {
    source->SetRegularEnergySpectrum(options.energy_values_kev,
                                     options.energy_weights, "keV");
    auto const width =
        CanonicalEnergy(options.energy_bin_width_kev, "--energy-bin-width-kev");

    if (width !=
        source->GetEnergyDistribution().GetRegularBinWidthMicroElectronVolt()) {
      throw std::runtime_error(
          "Requested bin width must equal the canonical center spacing.");
    }
  }

  auto const &[size_x, size_y, size_z] = options.dimensions_pm;

  if (options.geometry == "point") {
    source->SetPointEmission();
  } else if (options.geometry == "rectangle") {
    source->SetRectangleEmissionPicoMeter(size_x, size_y);
  } else if (options.geometry == "ellipse") {
    source->SetEllipseEmissionPicoMeter(size_x, size_y);
  } else if (options.geometry == "circle") {
    source->SetCircleEmissionPicoMeter(size_x);
  } else if (options.geometry == "box") {
    source->SetBoxEmissionPicoMeter(size_x, size_y, size_z);
  } else if (options.geometry == "sphere") {
    source->SetSphereEmissionPicoMeter(size_x);
  } else if (options.geometry == "cylinder") {
    source->SetCylinderEmissionPicoMeter(size_x, size_z);
  } else {
    throw std::runtime_error(
        std::format("Unsupported Source geometry '{}'.", options.geometry));
  }

  auto const record = source->BuildRecord();

  if (record.geometry_size_x_pm != size_x ||
      record.geometry_size_y_pm != size_y ||
      record.geometry_size_z_pm != size_z) {
    throw std::runtime_error(
        "Supplied XYZ dimensions do not match the geometry's complete "
        "widths/diameters/heights; see --help.");
  }

  if (options.angular == "isotropic") {
    // This API selects the exact packed canonical full-sphere branch.
    source->SetIsotropicAngularDistribution();
  } else if (options.angular == "bounded-isotropic") {
    auto const &[theta_min, theta_max, phi_min, phi_max] =
        options.angular_bounds_deg;
    source->SetIsotropicAngularDistribution(
        ggems::units::MakeQuantity<ggems::units::Angle>(theta_min, "deg")
            .value(),
        ggems::units::MakeQuantity<ggems::units::Angle>(theta_max, "deg")
            .value(),
        ggems::units::MakeQuantity<ggems::units::Angle>(phi_min, "deg").value(),
        ggems::units::MakeQuantity<ggems::units::Angle>(phi_max, "deg")
            .value());
  } else if (options.angular == "focused") {
    auto const &[focus_x, focus_y, focus_z] = options.focus_pm;
    source->SetFocusedAngularDistributionPicoMeter(focus_x, focus_y, focus_z);
  }

  return source;
}

// =============================================================================
// =============================================================================

auto CollectSourceRecords(GGEMSTransportObserver const &observer,
                          Options const &options,
                          GGEMSSourceRecord const &source)
    -> std::vector<GGEMSObserverRecord const *> {
  if (observer.GetOverflowCount() != 0U) {
    throw std::runtime_error("Observer overflow: samples are incomplete.");
  }

  auto const expected_records = 2ULL * options.primary_count;

  if (observer.GetRecords().size() != expected_records ||
      observer.GetRecordCount() != expected_records ||
      observer.GetCapturedPrimaryCount() != options.primary_count) {
    throw std::runtime_error(
        "Observer count mismatch for complete Source/Terminal capture.");
  }

  std::vector<GGEMSObserverRecord const *> records;
  records.reserve(static_cast<std::size_t>(options.primary_count));

  for (auto const &record : observer.GetRecords()) {
    if (record.record_kind ==
        ToKernelObserverRecordKind(GGEMSObserverRecordKind::Source)) {
      records.push_back(&record);
    } else if (record.record_kind !=
               ToKernelObserverRecordKind(GGEMSObserverRecordKind::Terminal)) {
      throw std::runtime_error(
          "Unexpected record kind in Source diagnostic transport.");
    }
  }

  if (records.size() != options.primary_count) {
    throw std::runtime_error("Missing or extra Source records.");
  }

  std::ranges::sort(records, [](auto const *left, auto const *right) -> bool {
    return std::tie(left->source_index, left->source_local_primary_id) <
           std::tie(right->source_index, right->source_local_primary_id);
  });

  auto const run_id = records.front()->run_id;
  auto const global_begin = records.front()->global_primary_id;
  if (global_begin > std::numeric_limits<std::uint64_t>::max() -
                         (options.primary_count - 1ULL)) {
    throw std::runtime_error("Source global primary range overflows uint64.");
  }

  // Preserve the fresh single-Run contract; sequences validate relative
  // progression after harvesting each actual Observer result.
  if (options.sequence_path.empty() &&
      (run_id != 0ULL || global_begin != 0ULL)) {
    throw std::runtime_error("Unexpected fresh single-Run provenance.");
  }

  std::uint64_t expected_id{};
  for (auto const *record : records) {
    if (record->source_index != 0U ||
        record->source_local_primary_id != expected_id ||
        record->global_primary_id != global_begin + expected_id ||
        record->run_id != run_id ||
        record->global_particle_id != record->global_primary_id ||
        record->track_id != 0ULL) {
      throw std::runtime_error(
          "Missing, duplicate, or impossible Source primary provenance.");
    }

    if (record->particle_type != source.emitted_particle_type ||
        !std::isfinite(record->direction_x) ||
        !std::isfinite(record->direction_y) ||
        !std::isfinite(record->direction_z) ||
        (options.energy_mode == "mono" &&
         record->energy_micro_eV != source.energy_micro_eV) ||
        record->time_ps != source.time_start_ps ||
        record->weight != source.weight) {
      throw std::runtime_error(
          "Source record violates the configured initialization contract.");
    }

    if (options.angular == "fixed" &&
        (record->direction_x != source.axis_z_x ||
         record->direction_y != source.axis_z_y ||
         record->direction_z != source.axis_z_z)) {
      throw std::runtime_error(
          "Source record violates the stored Fixed direction.");
    }

    if (options.geometry == "point" &&
        (record->position_x_pm != source.position_x_pm ||
         record->position_y_pm != source.position_y_pm ||
         record->position_z_pm != source.position_z_pm)) {
      throw std::runtime_error("Point Source differs from its exact center.");
    }

    ++expected_id;
  }

  return records;
}

// =============================================================================
// =============================================================================

auto WriteSamples(std::filesystem::path const &path,
                  std::span<GGEMSObserverRecord const *const> records) -> void {
  std::ofstream output{path};
  output.exceptions(std::ios::badbit | std::ios::failbit);
  output.imbue(std::locale::classic());
  output << std::setprecision(std::numeric_limits<float>::max_digits10);
  output
      << "source_index,source_local_primary_id,global_primary_id,x_pm,y_pm,"
         "z_pm,"
         "direction_x,direction_y,direction_z,energy_micro_eV,time_ps,weight,"
         "record_kind\n";

  for (auto const *record : records) {
    output << record->source_index << ',' << record->source_local_primary_id
           << ',' << record->global_primary_id << ',' << record->position_x_pm
           << ',' << record->position_y_pm << ',' << record->position_z_pm
           << ',' << record->direction_x << ',' << record->direction_y << ','
           << record->direction_z << ',' << record->energy_micro_eV << ','
           << record->time_ps << ',' << record->weight << ",Source\n";
  }

  output.close();
}

// =============================================================================
// =============================================================================

auto JsonString(std::string_view text) -> std::string {
  std::string result{"\""};
  constexpr std::string_view k_hex{"0123456789abcdef"};

  for (char character : text) {
    auto const byte = static_cast<unsigned char>(character);

    if (character == '"' || character == '\\') {
      result += '\\';
      result += character;
    } else if (byte < 0x20U) {
      result += "\\u00";
      result += k_hex[byte >> 4U];
      result += k_hex[byte & 0x0FU];
    } else {
      result += character;
    }
  }

  result += '"';
  return result;
}

// =============================================================================
// =============================================================================

auto WriteJsonArray(std::ostream &output, auto const &values) -> void {
  output << '[';
  bool first = true;
  for (auto const value : values) {
    if (!first) {
      output << ',';
    }
    output << value;
    first = false;
  }
  output << ']';
}

// =============================================================================
// =============================================================================

auto WriteEnergyMetadata(std::ostream &output, Options const &options,
                         GGEMSSourceRunSnapshot const &snapshot) -> void {
  auto const &record = snapshot.GetEnergyDistributionRecords().at(0U);
  auto const &energies = snapshot.GetEnergyValuesMicroElectronVolt();
  auto const &weights = snapshot.GetRelativeWeights();
  auto const &bounds = snapshot.GetCumulativeTicketUpperBounds();

  // This exporter owns exactly one CountDriven source. Export the immutable
  // arrays used by this run, with no reconstruction of their ticket allocation.
  if (record.table_offset != 0ULL || record.table_count != energies.size() ||
      weights.size() != energies.size() || bounds.size() != energies.size()) {
    throw std::runtime_error("Unexpected single-source packed energy table.");
  }

  output << ",\n  \"energy_mode\":"
         << JsonString(ggems::core::sources::ToLongName(
                ggems::core::sources::FromKernelEnergyDistributionType(
                    record.distribution_type)))
         << ",\n  \"energy_configuration\":" << JsonString(options.energy_mode)
         << ",\n  \"energy\":{\"representation\":\"uint64 micro-eV\""
         << ",\"distribution_type\":" << record.distribution_type
         << ",\"table_offset\":" << record.table_offset
         << ",\"table_count\":" << record.table_count
         << ",\"mono_energy_micro_eV\":"
         << snapshot.GetRecords().at(0U).energy_micro_eV
         << ",\"regular_bin_width_micro_eV\":"
         << record.regular_bin_width_micro_eV << ",\"ticket_space_size\":"
         << ggems::core::sources::k_energy_ticket_space_size
         << ",\"energy_values_micro_eV\":";
  WriteJsonArray(output, energies);
  output << ",\"relative_weights\":";
  WriteJsonArray(output, weights);
  output << ",\"cumulative_ticket_upper_bounds\":";
  WriteJsonArray(output, bounds);

  output << ",\"display_unit\":\"keV\",\"display_unit_micro_eV\":"
         << ggems::units::MakeQuantity<ggems::units::Energy>(1U, "keV")
                .value()
                .value
         << "},\n  \"requested_energy\":{\"unit\":\"keV\",\"mono\":";
  if (options.energy_mode == "mono") {
    output << options.mono_energy_kev;
  } else {
    output << "null";
  }
  output << ",\"values\":";
  WriteJsonArray(output, options.energy_values_kev);
  output << ",\"relative_weights\":";
  WriteJsonArray(output, options.energy_weights);
  output << ",\"bin_width\":";
  if (options.energy_mode == "regular-spectrum") {
    output << options.energy_bin_width_kev;
  } else {
    output << "null";
  }
  output << '}';
}

// =============================================================================
// =============================================================================

auto WriteMetadata(std::ostream &output, Options const &options,
                   GGEMSSourceRunSnapshot const &snapshot,
                   GGEMSTransportObserver const &observer,
                   std::span<std::string const> device_names,
                   std::span<GGEMSObserverRecord const *const> records,
                   std::uint32_t sequence_index,
                   std::filesystem::path const &samples_path) -> void {
  auto const &source = snapshot.GetRecords().at(0U);
  output.imbue(std::locale::classic());
  output << std::setprecision(std::numeric_limits<long double>::max_digits10);
  output << "{\n  \"case_name\":" << JsonString(options.case_name)
         << ",\n  \"geometry\":" << JsonString(options.geometry)
         << ",\n  \"dimensions_pm\":[" << source.geometry_size_x_pm << ','
         << source.geometry_size_y_pm << ',' << source.geometry_size_z_pm << ']'
         << ",\n  \"dimensions_mm\":[";

  for (std::size_t index = 0U; index < options.dimensions_pm.size(); ++index) {
    if (index != 0U) {
      output << ',';
    }
    output << ggems::units::ConvertTo(
                  ggems::units::Length{options.dimensions_pm[index]}, "mm")
                  .value();
  }

  output << "]"
         << ",\n  \"primary_count\":" << options.primary_count
         << ",\n  \"worker_count\":" << options.worker_count
         << ",\n  \"seed\":" << options.seed
         << ",\n  \"device_selector\":" << JsonString(options.device_selector)
         << ",\n  \"device_names\":[";

  for (std::size_t index = 0U; index < device_names.size(); ++index) {
    if (index != 0U) {
      output << ',';
    }
    output << JsonString(device_names[index]);
  }

  output
      << "]"
      << ",\n  \"source_index\":0,\n  \"global_primary_begin\":"
      << records.front()->global_primary_id
      << ",\n  \"global_primary_last\":" << records.back()->global_primary_id
      << ",\n  \"run_id\":" << records.front()->run_id
      << ",\n  \"sequence_index\":" << sequence_index
      << ",\n  \"samples_file\":"
      << JsonString(
             std::filesystem::relative(
                 samples_path,
                 std::filesystem::absolute(options.metadata_path).parent_path())
                 .generic_string())
      << ",\n  \"effective_start_ps\":" << snapshot.GetTimeWindow().start_ps
      << ",\n  \"effective_stop_ps\":" << snapshot.GetTimeWindow().stop_ps
      << ",\n  \"snapshot_time_start_ps\":" << source.time_start_ps
      << ",\n  \"snapshot_time_stop_ps\":" << source.time_stop_ps
      << ",\n  \"source_center_pm\":[" << source.position_x_pm << ','
      << source.position_y_pm << ',' << source.position_z_pm << ']'
      << ",\n  \"frame_axes\":[[" << source.axis_x_x << ',' << source.axis_x_y
      << ',' << source.axis_x_z << "],[" << source.axis_y_x << ','
      << source.axis_y_y << ',' << source.axis_y_z << "],[" << source.axis_z_x
      << ',' << source.axis_z_y << ',' << source.axis_z_z << "]]"
      << ",\n  \"fixed_direction\":[" << source.axis_z_x << ','
      << source.axis_z_y << ',' << source.axis_z_z << ']'
      << ",\n  \"energy_micro_eV\":" << source.energy_micro_eV
      << ",\n  \"time_ps\":" << source.time_start_ps
      << ",\n  \"weight\":" << source.weight
      << ",\n  \"population_mode\":\"CountDriven\",\n  "
         "\"rng_engine\":\"Philox\""
      << ",\n  \"particle\":\"Gamma\",\n  \"angular_mode\":"
      << JsonString(ggems::core::sources::ToLongName(
             ggems::core::sources::FromKernelAngularDistributionType(
                 source.angular_distribution_type)))
      << ",\n  \"angular_configuration\":" << JsonString(options.angular)
      << ",\n  \"isotropic_cos_theta_lower\":"
      << source.isotropic_cos_theta_lower
      << ",\n  \"isotropic_cos_theta_upper\":"
      << source.isotropic_cos_theta_upper
      << ",\n  \"isotropic_phi_min_rad\":" << source.isotropic_phi_min_rad
      << ",\n  \"isotropic_phi_max_rad\":" << source.isotropic_phi_max_rad
      << ",\n  \"requested_bounded_degrees\":";

  if (options.angular == "bounded-isotropic") {
    auto const &[theta_min, theta_max, phi_min, phi_max] =
        options.angular_bounds_deg;
    output << "{\"theta_min\":" << theta_min << ",\"theta_max\":" << theta_max
           << ",\"phi_min\":" << phi_min << ",\"phi_max\":" << phi_max << '}';
  } else {
    output << "null";
  }

  output << ",\n  \"focus_position_pm\":[" << source.focus_position_x_pm << ','
         << source.focus_position_y_pm << ',' << source.focus_position_z_pm
         << "],\n  \"focus_position_mm\":[";

  for (std::size_t index = 0U; index < options.focus_pm.size(); ++index) {
    if (index != 0U) {
      output << ',';
    }
    output << ggems::units::ConvertTo(
                  ggems::units::PositionCoordinate{options.focus_pm[index]},
                  "mm")
                  .value();
  }

  output << ']';
  WriteEnergyMetadata(output, options, snapshot);

  output << ",\n  \"requested_center_mm\":";
  WriteJsonArray(output, options.requested_center_mm);
  output << ",\n  \"position_display_unit_pm\":"
         << ggems::units::MakeQuantity<ggems::units::Length>(1U, "mm")
                .value()
                .value
         << ",\n  \"frame_matrix_convention\":\"axes_as_columns\""
         << ",\n  \"requested_frame\":";
  if (options.has_orientation) {
    output << R"({"api":"SetOrientation","direction":)";
    WriteJsonArray(output, options.frame_direction);
    output << ",\"up_reference\":";
    WriteJsonArray(output, options.frame_up);
    output << '}';
  } else {
    output << R"({"api":"default_identity"})";
  }

  output << ",\n  \"chronology\":" << JsonString(options.chronology)
         << ",\n  \"observer\":{\"overflow_count\":"
         << observer.GetOverflowCount()
         << ",\"record_count\":" << observer.GetRecordCount()
         << ",\"captured_primary_count\":" << observer.GetCapturedPrimaryCount()
         << ",\"source_record_count\":" << options.primary_count
         << ",\"capacity_per_device\":" << observer.GetRecordCapacity()
         << ",\"host_capacity\":" << 2ULL * options.primary_count << "}\n}\n";
}

// =============================================================================
// =============================================================================

auto WriteSequenceHeader(std::ostream &output, Options const &options) -> void {
  output.imbue(std::locale::classic());
  output << std::setprecision(std::numeric_limits<long double>::max_digits10);
  output << "{\n  \"case_name\":" << JsonString(options.case_name)
         << ",\n  \"chronology_mode\":" << JsonString(options.chronology)
         << ",\n  \"primary_count_per_run\":" << options.primary_count
         << ",\n  \"sequence_length\":" << options.sequence_runs
         << ",\n  \"time_representation\":\"uint64 ps\""
         << ",\n  \"display_unit_ps\":"
         << ggems::units::MakeQuantity<ggems::units::Duration>(1U, "ns")
                .value()
                .value
         << ",\n  \"requested_time_ns\":";

  if (options.chronology == "configured") {
    WriteJsonArray(output, options.requested_time_ns);
    output << ",\n  \"configured_time_ps\":";
    WriteJsonArray(output, options.configured_time_ps);
  } else {
    output << "null,\n  \"configured_time_ps\":null";
  }

  output << ",\n  \"reset_before_sequence_index\":";
  if (options.reset_before_run) {
    output << *options.reset_before_run;
  } else {
    output << "null";
  }
  output << ",\n  \"runs\":[\n";
}

} // namespace

// =============================================================================
// =============================================================================

auto main(int argc, char const *const *argv) -> int {
  try {
    auto const options = ParseArguments(argc, argv);

    if (options.help) {
      PrintUsage();
      return 0;
    }

    ggems::core::GGEMSLogger::GetInstance().SetDetailLevel(-1);

    auto source = ConfigureSource(options);

    // GGEMSRun owns chronology validation and effective window calculation.
    ggems::core::GGEMSRun run;
    if (options.chronology == "configured") {
      auto const &[start, stop, step] = options.configured_time_ps;
      run.SetTimePicoSecond(start, stop, step);
    }

    auto random = std::make_shared<ggems::core::random::GGEMSRandom>();
    random->SetEngine(ggems::core::random::GGEMSRandomEngine::Philox)
        .SetSeed(options.seed);

    auto observer = std::make_shared<GGEMSTransportObserver>();
    auto const capacity =
        static_cast<std::uint32_t>(2ULL * options.primary_count);
    observer->SetRecordCapacity(capacity)
        .SetMaxStoredRecordCount(capacity)
        .CaptureFirstPrimaries(
            static_cast<std::uint32_t>(options.primary_count));

    auto &opencl = ggems::ocl::GGEMSOpenCL::GetInstance();
    opencl.SelectDevices({options.device_selector});
    opencl.Initialize();
    std::vector<std::string> device_names;
    for (auto const &context : opencl.GetContext()) {
      device_names.push_back(context.GetDevice().GetName());
    }

    run.SetRandom(random);
    run.SetSource(source);
    run.SetObserver(observer);
    run.SetWorkerCount(options.worker_count);
    run.Initialize();

    bool const sequence = !options.sequence_path.empty();
    if (sequence) {
      std::filesystem::create_directories(options.sequence_path);
    }

    std::ofstream metadata;
    metadata.exceptions(std::ios::badbit | std::ios::failbit);

    std::optional<std::uint64_t> previous_run_id;
    std::optional<std::uint64_t> previous_global_last;

    for (std::uint32_t index = 0U; index < options.sequence_runs; ++index) {
      if (options.reset_before_run == index) {
        run.ResetTime();
      }
      run.Run();

      // Copy the committed snapshot and harvest this Observer result before
      // the next successful Run replaces it. Never read live Source time.
      auto const snapshot = run.GetLastSourceRunSnapshot();
      if (!snapshot || snapshot->GetRecords().size() != 1U ||
          snapshot->GetEnergyDistributionRecords().size() != 1U ||
          snapshot->GetTotalPrimaryCount() != options.primary_count) {
        throw std::runtime_error(
            "Missing or inconsistent single-source run snapshot.");
      }

      auto const &source_record = snapshot->GetRecords().at(0U);
      auto const window = snapshot->GetTimeWindow();
      if (source_record.time_start_ps != window.start_ps ||
          source_record.time_stop_ps != window.stop_ps) {
        throw std::runtime_error(
            "Source snapshot differs from its Run window.");
      }

      auto const records =
          CollectSourceRecords(*observer, options, source_record);
      auto const run_id = records.front()->run_id;
      auto const global_begin = records.front()->global_primary_id;

      if (previous_run_id &&
          (*previous_run_id == std::numeric_limits<std::uint64_t>::max() ||
           run_id != *previous_run_id + 1ULL)) {
        throw std::runtime_error("Successful Run identities did not continue.");
      }
      if (previous_global_last &&
          (*previous_global_last == std::numeric_limits<std::uint64_t>::max() ||
           global_begin != *previous_global_last + 1ULL)) {
        throw std::runtime_error("Successful primary ranges did not continue.");
      }

      auto const samples_path =
          sequence ? options.sequence_path / std::format("run_{:03}.csv", index)
                   : options.output_path;
      WriteSamples(samples_path, records);

      if (index == 0U) {
        metadata.open(options.metadata_path);
        if (sequence) {
          WriteSequenceHeader(metadata, options);
        }
      }

      if (sequence && index != 0U) {
        metadata << ",\n";
      }
      WriteMetadata(metadata, options, *snapshot, *observer, device_names,
                    records, index, samples_path);
      metadata.flush();

      previous_run_id = run_id;
      previous_global_last = records.back()->global_primary_id;
      if (sequence) {
        std::cout << options.case_name << ": run " << index << " (id " << run_id
                  << "); [" << window.start_ps << ',' << window.stop_ps
                  << (options.chronology == "static" ? "]" : ")") << " ps; "
                  << records.size() << " Source records; Observer overflow=0; "
                  << samples_path << '\n';
      } else {
        std::cout << options.case_name << ": " << records.size()
                  << " Source records; Observer overflow=0; " << samples_path
                  << '\n';
      }
    }

    if (sequence) {
      metadata << "]\n}\n";
    }
    metadata.close();

    return 0;
  } catch (std::exception const &error) {
    std::cerr << "GGEMS Source exporter:" << error.what() << '\n';
    return 1;
  }
}
