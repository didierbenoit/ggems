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

#include "GGEMS/GGEMSRun.hh"
#include "GGEMS/opencl/GGEMSOpenCL.hh"
#include "GGEMS/opencl/GGEMSOpenCLContext.hh"
#include "GGEMS/opencl/GGEMSOpenCLDevice.hh"
#include "GGEMS/logging/GGEMSLogger.hh"
#include "GGEMS/sources/GGEMSSource.hh"
#include "GGEMS/sources/GGEMSSourceRecord.hh"
#include "GGEMS/particles/GGEMSParticleTypes.hh"
#include "GGEMS/units/GGEMSEnergyUnits.hh"
#include "GGEMS/units/GGEMSUnitConversion.hh"
#include "GGEMS/units/GGEMSLengthUnits.hh"
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
  std::filesystem::path output_path;
  std::filesystem::path metadata_path;
  bool help{false};
};

// =============================================================================
// =============================================================================

auto PrintUsage() -> void {
  std::cout
      << "GGEMS Source / Geometry G1 sample exporter\n"
      << "Required: --device <GGEMS selector> --geometry <name>\n"
      << "          --output <samples.csv> --metadata <metadata.json>\n"
      << "          --primaries <uint64> --workers <uint32> --seed <uint64>\n"
      << "Geometry: point|rectangle|ellipse|circle|box|sphere|cylinder\n"
      << "Options:  --case-name <name> (default G1_<geometry>)\n"
      << "          --size-x-mm <value> --size-y-mm <value>\n"
      << "          --size-z-mm <value> (default sizes are zero)\n"
      << "Supply complete widths/diameters/heights: circle X=Y, sphere\n"
      << "X=Y=Z, cylinder X=Y and Z=height; planar Z and Point sizes=0.\n"
      << "Capture requires 2*N <= UINT32_MAX with the current Observer.\n"
      << "One fresh CountDriven Gamma Source, identity frame, origin,\n"
      << "Fixed +Z, Mono 511 keV, static 0 ps, weight 1, Philox only.\n";
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

auto ParseLength(std::string_view text, std::string_view option)
    -> std::uint64_t {
  long double value{};

  auto const result =
      std::from_chars(text.data(), text.data() + text.size(), value);

  if (result.ec != std::errc{} || result.ptr != text.data() + text.size()) {
    throw std::runtime_error(
        std::format("Invalid number for {}: '{}'.", option, text));
  }

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
    } else if (option == "--output") {
      options.output_path = value;
    } else if (option == "--metadata") {
      options.metadata_path = value;
    } else {
      throw std::runtime_error(std::format("Unknown option '{}'.", option));
    }
  }

  if (options.device_selector.empty() || options.geometry.empty() ||
      options.output_path.empty() || options.metadata_path.empty() ||
      !seen.contains("--primaries") || !seen.contains("--workers") ||
      !seen.contains("--seed")) {
    throw std::runtime_error(
        "--device, --geometry, --output, --metadata, --primaries, --workers "
        "and --seed are required.");
  }

  if (options.primary_count == 0ULL ||
      options.primary_count >
          std::numeric_limits<std::uint32_t>::max() / 2ULL) {
    throw std::runtime_error(
        "--primaries must be in [1, 2147483647]: the current Observer needs "
        "uint32 capacity for Source and Terminal records.");
  }

  if (std::filesystem::absolute(options.output_path).lexically_normal() ==
      std::filesystem::absolute(options.metadata_path).lexically_normal()) {
    throw std::runtime_error("Sample and metadata paths must differ.");
  }

  if (options.case_name.empty()) {
    options.case_name = "G1_" + options.geometry;
  }

  return options;
}

// =============================================================================
// =============================================================================

auto ConfigureSource(Options const &options) -> std::shared_ptr<GGEMSSource> {
  auto source = std::make_shared<GGEMSSource>();
  auto const energy =
      ggems::units::MakeQuantity<ggems::units::Energy>(511U, "keV").value();

  source->SetAnalytic()
      .SetCountDrivenPopulation(options.primary_count)
      .SetEmittedParticleType(ggems::core::particles::GGEMSParticleType::Gamma)
      .SetFixedAngularDistribution()
      .SetEnergyMilliElectronVolt(energy.value)
      .SetPositionPicoMeter(0LL, 0LL, 0LL)
      .SetWeight(1.0F);

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
        std::format("Unsupported G1 geometry '{}'.", options.geometry));
  }

  auto const record = source->BuildRecord();

  if (record.geometry_size_x_pm != size_x ||
      record.geometry_size_y_pm != size_y ||
      record.geometry_size_z_pm != size_z) {
    throw std::runtime_error(
        "Supplied XYZ dimensions do not match the geometry's complete "
        "widths/diameters/heights; see --help.");
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
          "Unexpected record kind in G1 diagnostic transport.");
    }
  }

  if (records.size() != options.primary_count) {
    throw std::runtime_error("Missing or extra Source records.");
  }

  std::ranges::sort(records, [](auto const *left, auto const *right) -> bool {
    return std::tie(left->source_index, left->source_local_primary_id) <
           std::tie(right->source_index, right->source_local_primary_id);
  });

  std::uint64_t expected_id{};
  for (auto const *record : records) {
    if (record->source_index != 0U ||
        record->source_local_primary_id != expected_id ||
        record->global_primary_id != expected_id || record->run_id != 0ULL ||
        record->global_particle_id != expected_id || record->track_id != 0ULL) {
      throw std::runtime_error(
          "Missing, duplicate, or impossible Source primary provenance.");
    }

    if (record->particle_type != source.emitted_particle_type ||
        record->direction_x != source.axis_z_x ||
        record->direction_y != source.axis_z_y ||
        record->direction_z != source.axis_z_z ||
        record->energy_milli_eV != source.energy_milli_eV ||
        record->time_ps != 0ULL || record->weight != source.weight) {
      throw std::runtime_error(
          "Source record violates the fixed G1 configuration.");
    }

    if (options.geometry == "point" &&
        (record->position_x_pm != 0LL || record->position_y_pm != 0LL ||
         record->position_z_pm != 0LL)) {
      throw std::runtime_error("Point Source produced a nonzero coordinate.");
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
  output << "source_index,source_local_primary_id,global_primary_id,x_pm,y_pm,"
            "z_pm,"
            "direction_x,direction_y,direction_z,energy_meV,time_ps,weight,"
            "record_kind\n";

  for (auto const *record : records) {
    output << record->source_index << ',' << record->source_local_primary_id
           << ',' << record->global_primary_id << ',' << record->position_x_pm
           << ',' << record->position_y_pm << ',' << record->position_z_pm
           << ',' << record->direction_x << ',' << record->direction_y << ','
           << record->direction_z << ',' << record->energy_milli_eV << ','
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

auto WriteMetadata(Options const &options, GGEMSSourceRecord const &source,
                   GGEMSTransportObserver const &observer,
                   std::span<std::string const> device_names) -> void {
  std::ofstream output{options.metadata_path};
  output.exceptions(std::ios::badbit | std::ios::failbit);
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

  output << "]"
         << ",\n  \"source_index\":0,\n  \"global_primary_begin\":0"
         << ",\n  \"source_center_pm\":[" << source.position_x_pm << ','
         << source.position_y_pm << ',' << source.position_z_pm << ']'
         << ",\n  \"frame_axes\":[[" << source.axis_x_x << ','
         << source.axis_x_y << ',' << source.axis_x_z << "],["
         << source.axis_y_x << ',' << source.axis_y_y << ',' << source.axis_y_z
         << "],[" << source.axis_z_x << ',' << source.axis_z_y << ','
         << source.axis_z_z << "]]"
         << ",\n  \"fixed_direction\":[" << source.axis_z_x << ','
         << source.axis_z_y << ',' << source.axis_z_z << ']'
         << ",\n  \"energy_meV\":" << source.energy_milli_eV
         << ",\n  \"time_ps\":0,\n  \"weight\":" << source.weight
         << ",\n  \"population_mode\":\"CountDriven\",\n  "
            "\"rng_engine\":\"Philox\""
         << ",\n  \"particle\":\"Gamma\",\n  \"angular_mode\":\"Fixed\""
         << ",\n  \"energy_mode\":\"Mono\",\n  \"chronology\":\"static\""
         << ",\n  \"observer\":{\"overflow_count\":"
         << observer.GetOverflowCount()
         << ",\"record_count\":" << observer.GetRecordCount()
         << ",\"captured_primary_count\":" << observer.GetCapturedPrimaryCount()
         << ",\"source_record_count\":" << options.primary_count
         << ",\"capacity_per_device\":" << observer.GetRecordCapacity()
         << ",\"host_capacity\":" << 2ULL * options.primary_count << "}\n}\n";

  output.close();
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
    auto const source_record = source->BuildRecord();

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

    ggems::core::GGEMSRun run;
    run.SetRandom(random);
    run.SetSource(source);
    run.SetObserver(observer);
    run.SetWorkerCount(options.worker_count);
    run.Initialize();
    run.Run();

    auto const records =
        CollectSourceRecords(*observer, options, source_record);
    WriteSamples(options.output_path, records);
    WriteMetadata(options, source_record, *observer, device_names);
    std::cout << options.case_name << ": " << records.size()
              << " Source records; Observer overflow=0; " << options.output_path
              << '\n';

    return 0;
  } catch (std::exception const &error) {
    std::cerr << "GGEMS Source exporter:" << error.what() << '\n';
    return 1;
  }
}
