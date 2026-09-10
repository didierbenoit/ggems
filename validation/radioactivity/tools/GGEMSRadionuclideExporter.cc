#include <algorithm>
#include <array>
#include <charconv>
#include <cstddef>
#include <cstdint>
#include <filesystem>
#include <format>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <limits>
#include <locale>
#include <memory>
#include <span>
#include <stdexcept>
#include <string>
#include <string_view>
#include <system_error>
#include <vector>

#include "GGEMS/GGEMSRun.hh"
#include "GGEMS/GGEMSTimeWindow.hh"
#include "GGEMS/logging/GGEMSLogger.hh"
#include "GGEMS/observer/GGEMSObserverRecord.hh"
#include "GGEMS/observer/GGEMSObserverTypes.hh"
#include "GGEMS/observer/GGEMSTransportObserver.hh"
#include "GGEMS/opencl/GGEMSOpenCL.hh"
#include "GGEMS/opencl/GGEMSOpenCLContext.hh"
#include "GGEMS/opencl/GGEMSOpenCLDevice.hh"
#include "GGEMS/particles/GGEMSParticleTypes.hh"
#include "GGEMS/radioactivity/GGEMSRadionuclideDefinition.hh"
#include "GGEMS/radioactivity/GGEMSRadionuclideEmission.hh"
#include "GGEMS/radioactivity/builtins/GGEMSBuiltInRadionuclides.hh"
#include "GGEMS/random/GGEMSRandom.hh"
#include "GGEMS/random/GGEMSRandomEngine.hh"
#include "GGEMS/sources/GGEMSEnergyDistribution.hh"
#include "GGEMS/sources/GGEMSSource.hh"
#include "GGEMS/sources/GGEMSSourceEmissionRange.hh"
#include "GGEMS/sources/GGEMSSourcePopulationPlan.hh"
#include "GGEMS/sources/GGEMSSourceRunSnapshot.hh"
#include "GGEMS/sources/GGEMSSourceTypes.hh"
#include "GGEMS/units/GGEMSActivityUnits.hh"
#include "GGEMS/units/GGEMSEnergyUnits.hh"
#include "GGEMS/units/GGEMSTimeUnits.hh"
#include "GGEMS/units/GGEMSUnitConversion.hh"

namespace {
namespace sources = ggems::core::sources;
namespace radioactivity = ggems::core::radioactivity;
namespace random = ggems::core::random;
namespace observer = ggems::core::observer;

struct Options {
  std::string nuclide;
  std::string device{"cpu"};
  std::filesystem::path output;
  std::uint64_t seed{77'777ULL};
  std::uint64_t step_ps{0ULL};
  std::uint32_t windows{32U};
  std::uint32_t workers{256U};
  std::uint32_t capacity{50'000U};
  std::uint32_t population_replicates{128U};
  ggems::units::Activity activity{0.0L};
  bool describe{false};
  bool help{false};
};

auto PrintHelp() -> void {
  std::cout
      << R"(Export compiled radionuclide data or run an ActivityDriven campaign.
Use run_campaign.py for automatic half-life windows and activity selection.

Required:
  --nuclide NAME             Exact GGEMS name, for example O-15.
  --output DIRECTORY        Destination for definition, populations and samples.

Operation:
  --describe                Export the definition only, without OpenCL.
  --help                    Show this help.

Campaign:
  --activity-bq VALUE        Activity at time zero in Bq (required for a run).
  --step-ps INTEGER          Duration of each half-open window in ps (required).
  --windows INTEGER         Number of consecutive windows (default: 32).
  --device SELECTOR         GGEMS OpenCL selector, e.g. cpu, gpu, all (default: cpu).
  --workers INTEGER         Random-stream workers per device (default: 256).
  --seed INTEGER            Philox campaign seed (default: 77777).
  --capacity INTEGER        Captured primaries per window (default: 50000).
  --population-replicates N  Extra host-only Poisson experiments (default: 128).
                            Their seeds are seed+1 through seed+N.
)";
}

template <typename T> auto ParseNumber(std::string_view text) -> T {
  T value{};
  auto const result =
      std::from_chars(text.data(), text.data() + text.size(), value);
  if (result.ec != std::errc{} || result.ptr != text.data() + text.size()) {
    throw std::runtime_error(
        std::format("Invalid numeric argument '{}'.", text));
  }
  return value;
}

auto ParseOptions(int argc, char const *const *argv) -> Options {
  Options options;
  for (int index = 1; index < argc; ++index) {
    std::string_view const key{argv[index]};
    if (key == "--help") {
      options.help = true;
      return options;
    }
    if (key == "--describe") {
      options.describe = true;
      continue;
    }
    if (++index == argc) {
      throw std::runtime_error("Missing option value.");
    }
    std::string_view const value{argv[index]};
    if (key == "--nuclide") {
      options.nuclide = value;
    } else if (key == "--output") {
      options.output = value;
    } else if (key == "--device") {
      options.device = value;
    } else if (key == "--seed") {
      options.seed = ParseNumber<std::uint64_t>(value);
    } else if (key == "--step-ps") {
      options.step_ps = ParseNumber<std::uint64_t>(value);
    } else if (key == "--windows") {
      options.windows = ParseNumber<std::uint32_t>(value);
    } else if (key == "--workers") {
      options.workers = ParseNumber<std::uint32_t>(value);
    } else if (key == "--capacity") {
      options.capacity = ParseNumber<std::uint32_t>(value);
    } else if (key == "--population-replicates") {
      options.population_replicates = ParseNumber<std::uint32_t>(value);
    } else if (key == "--activity-bq") {
      auto const converted = ggems::units::MakeQuantity<ggems::units::Activity>(
          ParseNumber<long double>(value), "Bq");
      if (!converted) {
        throw std::runtime_error(
            "Activity is not representable in central Units.");
      }
      options.activity = *converted;
    } else {
      throw std::runtime_error(std::format("Unknown option '{}'.", key));
    }
  }
  return options;
}

// ----------------------------------------------------------------------------
// Scientific output

auto OpenOutput(std::filesystem::path const &path) -> std::ofstream {
  std::ofstream output;
  output.exceptions(std::ios::badbit | std::ios::failbit);
  output.open(path, std::ios::binary);
  output.imbue(std::locale::classic());
  output << std::setprecision(std::numeric_limits<long double>::max_digits10);
  return output;
}

auto JsonString(std::string_view value) -> std::string {
  constexpr unsigned char k_first_printable_ascii{0x20U};
  std::string result{R"json(")json"};
  for (char character : value) {
    auto const byte = static_cast<unsigned char>(character);
    if (character == '"' || character == '\\') {
      result += '\\';
      result += character;
    } else if (byte < k_first_printable_ascii) {
      result += std::format("\\u{:04x}", byte);
    } else {
      result += character;
    }
  }
  return result + '"';
}

auto WriteEnergyTable(std::filesystem::path const &directory, std::size_t index,
                      sources::GGEMSEnergyDistribution const &energy) -> void {
  auto table = OpenOutput(directory / std::format("group_{}.csv", index));
  table << "energy_micro_eV,relative_weight,cumulative_ticket_upper\n";
  auto const values = energy.GetEnergyValuesMicroElectronVolt();
  auto const weights = energy.GetRelativeWeights();
  auto const tickets = energy.GetCumulativeTicketUpperBounds();
  for (std::size_t row = 0U; row < values.size(); ++row) {
    table << values[row] << ',' << weights[row] << ',' << tickets[row] << '\n';
  }
}

auto WriteDefinition(
    std::filesystem::path const &directory,
    radioactivity::GGEMSRadionuclideDefinition const &definition) -> void {
  auto output = OpenOutput(directory / "definition.json");
  auto const energy_scale =
      ggems::units::MakeQuantity<ggems::units::Energy>(1ULL, "keV")
          .value()
          .value;
  auto const time_scale =
      ggems::units::MakeQuantity<ggems::units::Duration>(1ULL, "s")
          .value()
          .value;
  output << R"json({
"name":)json"
         << JsonString(definition.GetCanonicalName())
         << R"json(,"half_life_seconds":")json"
         << definition.GetHalfLifeSeconds()
         << R"json(","total_yield_per_decay":")json"
         << definition.GetTotalYieldPerDecay()
         << R"json(","energy_micro_eV_per_keV":)json" << energy_scale
         << R"json(,"time_ps_per_s":)json" << time_scale
         << R"json(,"time_max_ps":)json"
         << std::numeric_limits<std::uint64_t>::max()
         << R"json(,"ticket_space":)json" << sources::k_energy_ticket_space_size
         << R"json(,"groups":[
)json";
  std::size_t index = 0U;
  for (auto const &emission : definition.GetEmissions()) {
    if (index != 0U) {
      output << ",\n";
    }
    auto const &energy = emission.GetEnergyDistribution();
    WriteEnergyTable(directory, index, energy);
    output << R"json({"index":)json" << index << R"json(,"particle":)json"
           << JsonString(ggems::core::particles::ToLongName(
                  emission.GetParticleType()))
           << R"json(,"particle_type":)json"
           << ggems::core::particles::ToKernelParticleType(
                  emission.GetParticleType())
           << R"json(,"yield_per_decay":")json" << emission.GetYieldPerDecay()
           << R"json(","distribution_type":)json"
           << sources::ToKernelEnergyDistributionType(energy.GetType())
           << R"json(,"distribution_name":)json"
           << JsonString(sources::ToLongName(energy.GetType()))
           << R"json(,"mono_energy_micro_eV":)json"
           << energy.GetMonoEnergyMicroElectronVolt()
           << R"json(,"bin_width_micro_eV":)json"
           << energy.GetRegularBinWidthMicroElectronVolt()
           << R"json(,"table_count":)json" << energy.GetTableCount()
           << R"json(,"table_file":)json"
           << JsonString(std::format("group_{}.csv", index)) << '}';
    ++index;
  }
  output << "\n]}\n";
}

// ----------------------------------------------------------------------------
// Source samples and population observations

auto WritePopulation(std::ofstream &output, std::uint32_t replicate,
                     std::uint64_t seed, std::uint32_t window,
                     sources::GGEMSSourcePopulationPlan const &plan) -> void {
  for (auto const &group : plan.GetGroups()) {
    output << replicate << ',' << seed << ',' << window << ','
           << group.emission_index << ','
           << plan.GetSources().front().expected_parent_decay_count << ','
           << group.expected_emission_count << ','
           << group.sampled_primary_count << '\n';
  }
}

auto WriteSamples(std::ofstream &output, std::uint32_t window,
                  observer::GGEMSTransportObserver const &capture,
                  sources::GGEMSSourceRunSnapshot const &snapshot) -> void {
  auto const &ranges = snapshot.GetGroupRanges();
  std::vector<observer::GGEMSObserverRecord const *> records;
  for (auto const &record : capture.GetRecords()) {
    if (record.record_kind == observer::ToKernelObserverRecordKind(
                                  observer::GGEMSObserverRecordKind::Source)) {
      records.push_back(&record);
    }
  }
  std::ranges::sort(records, {},
                    &observer::GGEMSObserverRecord::source_local_primary_id);
  std::size_t group = 0U;
  for (std::size_t index = 0U; index < records.size(); ++index) {
    auto const &record = *records[index];
    while (index >= ranges[group].source_local_primary_begin +
                        ranges[group].primary_count) {
      ++group;
    }
    output << window << ',' << group << ',' << record.time_ps << ','
           << record.energy_micro_eV << '\n';
  }
}

// ----------------------------------------------------------------------------
// Campaign execution

auto WriteRunHeader(std::ofstream &metadata, Options const &options,
                    ggems::ocl::GGEMSOpenCL &opencl) -> void {
  metadata
      << R"json({"seed":)json" << options.seed
      << R"json(,"workers_per_device":)json" << options.workers
      << R"json(,"rng_engine":"Philox","geometry":"Point","direction":"Fixed")json"
      << R"json(,"population_mode":"ActivityDriven","activity_bq":")json"
      << options.activity.value
      << R"json(","reference_time_ps":0,"start_ps":0,"step_ps":)json"
      << options.step_ps << R"json(,"windows":)json" << options.windows
      << R"json(,"capacity_primaries":)json" << options.capacity
      << R"json(,"population_replicates":)json" << options.population_replicates

#ifdef GGEMS_DEBUG_MODE
      << R"json(,"build_mode":"Debug")json"
#else
      << R"json(,"build_mode":"Release")json"
#endif
      << R"json(,"compiler":)json" << JsonString(GGEMS_VALIDATION_COMPILER)
      << R"json(,"device_selector":)json" << JsonString(options.device)
      << R"json(,"devices":[)json";
  bool first = true;
  for (auto const &context : opencl.GetContext()) {
    if (!first) {
      metadata << ',';
    }
    first = false;
    auto const &device = context.GetDevice();
    metadata << R"json({"name":)json" << JsonString(device.GetName())
             << R"json(,"vendor":)json" << JsonString(device.GetVendor())
             << R"json(,"version":)json" << JsonString(device.GetVersion())
             << R"json(,"driver":)json" << JsonString(device.GetDriverVersion())
             << '}';
  }
  metadata << R"json(],"window_results":[
)json";
}

auto WriteWindow(std::ofstream &metadata, std::uint32_t window,
                 sources::GGEMSSourceRunSnapshot const &snapshot,
                 observer::GGEMSTransportObserver const &capture) -> void {
  auto const time = snapshot.GetTimeWindow();
  if (window != 0U) {
    metadata << ",\n";
  }
  metadata << R"json({"index":)json" << window << R"json(,"start_ps":)json"
           << time.start_ps << R"json(,"stop_ps":)json" << time.stop_ps
           << R"json(,"primary_count":)json" << snapshot.GetTotalPrimaryCount()
           << R"json(,"captured_primary_count":)json"
           << capture.GetCapturedPrimaryCount()
           << R"json(,"overflow_count":)json" << capture.GetOverflowCount()
           << R"json(,"scaled_decay":)json"
           << snapshot.GetPopulationRecords().front().scaled_decay << '}';
}

auto WritePopulationReplicas(
    std::ofstream &populations, Options const &options,
    std::span<std::shared_ptr<sources::GGEMSSource> const> source_list)
    -> void {
  // Extra population experiments have distinct seeds and no device execution.
  for (std::uint32_t index = 0U; index < options.population_replicates;
       ++index) {
    auto const replicate = index + 1ULL;
    random::GGEMSRandom replicate_random;
    auto const seed = options.seed + replicate;
    replicate_random.SetEngine(random::GGEMSRandomEngine::Philox).SetSeed(seed);
    sources::GGEMSSourcePopulationPlanner planner(source_list,
                                                  replicate_random);
    for (std::uint32_t window = 0U; window < options.windows; ++window) {
      ggems::core::GGEMSTimeWindow const time{
          .start_ps = options.step_ps * window,
          .stop_ps = options.step_ps * (window + 1ULL)};
      auto candidate = planner.BuildCandidate(time);
      WritePopulation(populations, static_cast<std::uint32_t>(replicate), seed,
                      window, candidate.GetPlan());
      planner.CommitCandidate(candidate);
    }
  }
}

auto RunCampaign(
    Options const &options,
    std::shared_ptr<radioactivity::GGEMSRadionuclideDefinition const> const
        &definition) -> void {
  auto source = std::make_shared<sources::GGEMSSource>();
  source->SetPointEmission().SetFixedAngularDistribution().SetRadionuclide(
      definition, options.activity, 0ULL);
  auto engine = std::make_shared<random::GGEMSRandom>();
  engine->SetEngine(random::GGEMSRandomEngine::Philox).SetSeed(options.seed);
  std::array const source_list{source};
  // A separate planner exposes expected counts without consuming Run's RNG.
  sources::GGEMSSourcePopulationPlanner population_planner(source_list,
                                                           *engine);
  auto capture = std::make_shared<observer::GGEMSTransportObserver>();
  capture->SetRecordCapacity(2U * options.capacity)
      .SetMaxStoredRecordCount(2U * options.capacity)
      .CaptureFirstPrimaries(options.capacity);
  auto &opencl = ggems::ocl::GGEMSOpenCL::GetInstance();
  opencl.SelectDevices({options.device});
  opencl.Initialize();
  auto metadata = OpenOutput(options.output / "run.json");
  WriteRunHeader(metadata, options, opencl);

  // ----------------------------------------------------------------------------
  // Initialize once; each Run advances the same source and random streams.

  ggems::core::GGEMSRun run;
  run.SetTimePicoSecond(0ULL, options.step_ps * options.windows,
                        options.step_ps);
  run.SetSource(source);
  run.SetRandom(engine);
  run.SetWorkerCount(options.workers);
  run.SetObserver(capture);
  run.Initialize();
  auto samples = OpenOutput(options.output / "samples.csv");
  samples << "window,group,time_ps,energy_micro_eV\n";
  auto populations = OpenOutput(options.output / "populations.csv");
  populations << "replicate,seed,window,group,expected_parent_decays,expected_"
                 "emissions,observed_count\n";
  for (std::uint32_t window = 0U; window < options.windows; ++window) {
    auto const time = run.GetCurrentTimeWindowPicoSecond();
    auto population = population_planner.BuildCandidate(time);
    auto const &plan = population.GetPlan();
    if (plan.GetTotalPrimaryCount() > options.capacity) {
      throw std::runtime_error("Planned population exceeds requested capture "
                               "capacity; no truncated campaign is accepted.");
    }
    run.Run();
    auto const snapshot = run.GetLastSourceRunSnapshot();
    if (capture->GetOverflowCount() != 0U) {
      throw std::runtime_error("Observer capture overflowed.");
    }
    WriteSamples(samples, window, *capture, *snapshot);
    WritePopulation(populations, 0U, options.seed, window, plan);
    WriteWindow(metadata, window, *snapshot, *capture);
    population_planner.CommitCandidate(population);
  }
  WritePopulationReplicas(populations, options, source_list);
  metadata << "\n]}\n";
}
} // namespace

auto main(int argc, char const *const *argv) -> int {
  auto const options = ParseOptions(argc, argv);
  if (options.help) {
    PrintHelp();
    return 0;
  }

  ggems::core::GGEMSLogger::GetInstance().SetDetailLevel(-1);
  auto definition =
      std::make_shared<radioactivity::GGEMSRadionuclideDefinition const>(
          radioactivity::builtins::BuildBuiltInRadionuclide(options.nuclide)
              .value());
  std::filesystem::create_directories(options.output);
  WriteDefinition(options.output, *definition);
  if (!options.describe) {
    RunCampaign(options, definition);
  }
  return 0;
}
