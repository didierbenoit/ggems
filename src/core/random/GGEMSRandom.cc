#include <algorithm>
#include <cctype>
#include <format>

#include "GGEMS/core/GGEMSException.hh"
#include "GGEMS/core/GGEMSMacros.hh"
#include "GGEMS/core/random/GGEMSRandom.hh"
#include "GGEMS/core/random/GGEMSRandomState.hh"

namespace ggems::core::random {
namespace {

std::string NormaliseEngineName(std::string_view engine_name) {
  std::string normalised;
  normalised.reserve(engine_name.size());

  for (char character : engine_name) {
    if (character == '_' || character == '-' || character == ' ') {
      continue;
    }

    normalised.push_back(
        static_cast<char>(std::tolower(static_cast<unsigned char>(character))));
  }

  return normalised;
}

} // namespace

// =============================================================================
// =============================================================================

std::string ToString(GGEMSRandomEngine engine) {
  switch (engine) {
  case GGEMSRandomEngine::JKISS:
    return "JKISS";
  case GGEMSRandomEngine::PCG32:
    return "PCG32";
  case GGEMSRandomEngine::Philox:
    return "Philox";
  }

  GGEMS_INTERNAL("Unsupported GGEMS random engine.");
  return "Unknown";
}

// =============================================================================
// =============================================================================

GGEMSRandomEngine ParseRandomEngine(std::string_view engine_name) {
  std::string normalised = NormaliseEngineName(engine_name);

  if (normalised == "jkiss" || normalised == "kiss") {
    return GGEMSRandomEngine::JKISS;
  }

  if (normalised == "pcg32" || normalised == "pcg") {
    return GGEMSRandomEngine::PCG32;
  }

  if (normalised == "philox") {
    return GGEMSRandomEngine::Philox;
  }

  GGEMS_RECOVERABLE(
      std::format("Unsupported GGEMS random engine '{}'.", engine_name));

  return GGEMSRandomEngine::JKISS;
}

// =============================================================================
// =============================================================================

std::uint32_t ToKernelEngineId(GGEMSRandomEngine engine) noexcept {
  return static_cast<std::uint32_t>(engine);
}

// =============================================================================
// =============================================================================

GGEMSRandom::GGEMSRandom() {
  GGEMS_INFOEX("Random", 3, "GGEMSRandom instance created.");
}

// -----------------------------------------------------------------------------

GGEMSRandom &GGEMSRandom::SetEngine(GGEMSRandomEngine engine) noexcept {
  engine_ = engine;
  return *this;
}

// -----------------------------------------------------------------------------

GGEMSRandom &GGEMSRandom::SetEngine(std::string_view engine_name) {
  engine_ = ParseRandomEngine(engine_name);
  return *this;
}

// -----------------------------------------------------------------------------

GGEMSRandomEngine GGEMSRandom::GetEngine() const noexcept { return engine_; }

// -----------------------------------------------------------------------------

std::string GGEMSRandom::GetEngineName() const { return ToString(engine_); }

// -----------------------------------------------------------------------------

GGEMSRandom &GGEMSRandom::SetSeed(std::uint64_t seed) noexcept {
  seed_ = seed;
  return *this;
}

// -----------------------------------------------------------------------------

std::uint64_t GGEMSRandom::GetSeed() const noexcept { return seed_; }

// -----------------------------------------------------------------------------

std::uint32_t GGEMSRandom::GetKernelEngineId() const noexcept {
  return ToKernelEngineId(engine_);
}

// -----------------------------------------------------------------------------

std::string GGEMSRandom::GetKernelBuildDefinition() const {
  return std::format("-DGGEMS_RANDOM_ENGINE={}", GetKernelEngineId());
}

// -----------------------------------------------------------------------------

std::size_t GGEMSRandom::GetStateSize() const noexcept {
  switch (engine_) {
  case GGEMSRandomEngine::JKISS:
    return sizeof(GGEMSJKissState);
  case GGEMSRandomEngine::PCG32:
    return sizeof(GGEMSPCG32State);
  case GGEMSRandomEngine::Philox:
    return sizeof(GGEMSPhiloxState);
  }

  return 0U;
}

// -----------------------------------------------------------------------------

std::vector<std::string> GGEMSRandom::BuildSummaryLines() const {
  return {
      std::format("Random engine           : {}", GetEngineName()),
      std::format("Seed                    : {}", seed_),
      std::format("State size              : {} bytes", GetStateSize()),
      std::format("OpenCL engine id        : {}", GetKernelEngineId()),
      std::format("OpenCL build definition : {}", GetKernelBuildDefinition()),
      "kernel scalar API      : GGEMS_RndmUniform",
      "kernel vector API      : GGEMS_RndmUniform4"};
}

// -----------------------------------------------------------------------------

void GGEMSRandom::Verbose() const {
  for (std::string &line : BuildSummaryLines()) {
    GGEMS_INFO("Random", "{}", line);
  }
}

} // namespace ggems::core::random
