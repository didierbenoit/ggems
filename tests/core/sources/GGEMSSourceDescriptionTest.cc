#include <array>
#include <cstdint>
#include <memory>
#include <string>
#include <string_view>
#include <numbers>

#include <gtest/gtest.h>

#include "GGEMS/core/particles/GGEMSParticleTypes.hh"
#include "GGEMS/core/sources/GGEMSSource.hh"
#include "GGEMS/core/sources/GGEMSSourceDescription.hh"
#include "GGEMS/core/sources/GGEMSSourceRunSnapshot.hh"
#include "GGEMS/core/units/GGEMSAngularUnits.hh"

namespace {

using GGEMSSource = ggems::core::sources::GGEMSSource;
using GGEMSSourcePtr = std::shared_ptr<GGEMSSource>;

// =============================================================================
// =============================================================================

constexpr std::string_view k_source_description{
    "Type: Analytic | Primary count: 7 | "
    "Particle: Electron (b-) | Emission: Point | Angular: Fixed | "
    "Energy: Mono (2.0000000 MeV) | "
    "Time: fixed at 0.0000000 ps | "
    "Position: (1.0000000 mm, -2.0000000 mm, 0.0000000 pm) | "
    "Axis Z: (1, 0, 0) | Weight: 0.25"};

// =============================================================================
// =============================================================================

constexpr std::string_view k_window_source_description{
    "Type: Analytic | Primary count: 7 | "
    "Particle: Electron (b-) | Emission: Point | Angular: Fixed | "
    "Energy: Mono (2.0000000 MeV) | "
    "Time window: [1.0000000 ns, 2.0000000 ns) | "
    "Position: (1.0000000 mm, -2.0000000 mm, 0.0000000 pm) | "
    "Axis Z: (1, 0, 0) | Weight: 0.25"};

// =============================================================================
// =============================================================================

constexpr std::string_view k_zero_primary_source_description{
    "Type: Analytic | Primary count: 0 | "
    "Particle: Electron (b-) | Emission: Point | Angular: Fixed | "
    "Energy: Mono (2.0000000 MeV) | "
    "Time: fixed at 0.0000000 ps | "
    "Position: (1.0000000 mm, -2.0000000 mm, 0.0000000 pm) | "
    "Axis Z: (1, 0, 0) | Weight: 0.25"};

// =============================================================================
// =============================================================================

constexpr std::string_view k_reconfigured_source_description{
    "Type: Analytic | Primary count: 11 | "
    "Particle: Gamma (g) | Emission: Point | Angular: Fixed | "
    "Energy: Mono (511.0000000 keV) | "
    "Time: fixed at 0.0000000 ps | "
    "Position: (0.0000000 pm, 0.0000000 pm, 1.0000000 um) | "
    "Axis Z: (0, 1, 0) | Weight: 0.5"};

// =============================================================================
// =============================================================================

[[nodiscard]] auto MakeConfiguredSource(std::uint64_t primary_count)
    -> GGEMSSourcePtr {
  auto source = std::make_shared<GGEMSSource>();

  source->SetPrimaryCount(primary_count)
      .SetAnalytic()
      .SetEmittedParticleType(
          ggems::core::particles::GGEMSParticleType::Electron)
      .SetEnergyMilliElectronVolt(2'000'000'000ULL)
      .SetPositionPicoMeter(1'000'000'000LL, -2'000'000'000LL, 0LL)
      .SetDirection(2.0F, 0.0F, 0.0F)
      .SetWeight(0.25F);

  return source;
}

// =============================================================================
// =============================================================================

auto ReconfigureSource(GGEMSSource &source) -> void {
  source.SetPrimaryCount(11ULL)
      .SetEmittedParticleType(ggems::core::particles::GGEMSParticleType::Gamma)
      .SetEnergyMilliElectronVolt(511'000'000ULL)
      .SetPositionPicoMeter(0LL, 0LL, 1'000'000LL)
      .SetDirection(0.0F, 2.0F, 0.0F)
      .SetWeight(0.5F);
}

// =============================================================================
// =============================================================================

auto ExpectContains(std::string const &description, std::string_view expected)
    -> void {
  EXPECT_NE(description.find(expected), std::string::npos)
      << "Description: " << description;
}
} // namespace

// =============================================================================
// =============================================================================

TEST(GGEMSSourceDescription, DescribesActiveSource) {
  auto source = MakeConfiguredSource(7ULL);

  EXPECT_EQ(ggems::core::sources::DescribeSource(source->BuildRecord(),
                                                 source->GetPrimaryCount()),
            k_source_description);
}

// =============================================================================
// =============================================================================

TEST(GGEMSSourceDescription, DescribesDisabledSource) {
  auto source = MakeConfiguredSource(0ULL);

  EXPECT_EQ(ggems::core::sources::DescribeSource(source->BuildRecord(),
                                                 source->GetPrimaryCount()),
            k_zero_primary_source_description);
}

// =============================================================================
// =============================================================================

TEST(GGEMSSourceDescription, DescribesSnapshotSlot) {
  std::array<GGEMSSourcePtr, 3U> const sources{MakeConfiguredSource(3ULL),
                                               MakeConfiguredSource(4ULL),
                                               MakeConfiguredSource(7ULL)};

  auto snapshot = ggems::core::sources::BuildSourceRunSnapshot(
      sources, {.start_ps = 1'000ULL, .stop_ps = 2'000ULL});

  EXPECT_EQ(ggems::core::sources::DescribeSourceRunSlot(
                2U, snapshot.GetRecords()[2U], snapshot.GetRanges()[2U]),
            std::string{"Source slot: 2 | Projection primary begin: 7 | "} +
                std::string{k_window_source_description});
}

// =============================================================================
// =============================================================================

TEST(GGEMSSourceDescription, PreservesZeroPrimarySlotWithoutCompaction) {
  std::array<GGEMSSourcePtr, 3U> const sources{MakeConfiguredSource(3ULL),
                                               MakeConfiguredSource(0ULL),
                                               MakeConfiguredSource(5ULL)};

  auto snapshot = ggems::core::sources::BuildSourceRunSnapshot(
      sources, {.start_ps = 1'000ULL, .stop_ps = 2'000ULL});

  ASSERT_EQ(snapshot.GetRecords().size(), 3U);
  ASSERT_EQ(snapshot.GetRanges().size(), 3U);

  std::string const disabled_slot = ggems::core::sources::DescribeSourceRunSlot(
      1U, snapshot.GetRecords()[1U], snapshot.GetRanges()[1U]);

  std::string const third_slot = ggems::core::sources::DescribeSourceRunSlot(
      2U, snapshot.GetRecords()[2U], snapshot.GetRanges()[2U]);

  ExpectContains(disabled_slot,
                 "Source slot: 1 | Projection primary begin: 3 | ");
  ExpectContains(disabled_slot, "Primary count: 0");
  ExpectContains(third_slot, "Source slot: 2 | Projection primary begin: 3 | ");
  ExpectContains(third_slot, "Primary count: 5");
}

// =============================================================================
// =============================================================================

TEST(GGEMSSourceDescription, DistinguishesDuplicateSourceSlots) {
  auto source = MakeConfiguredSource(7ULL);
  std::array<GGEMSSourcePtr, 2U> const sources{source, source};

  auto snapshot = ggems::core::sources::BuildSourceRunSnapshot(
      sources, {.start_ps = 1'000ULL, .stop_ps = 2'000ULL});

  std::string const slot_0 = ggems::core::sources::DescribeSourceRunSlot(
      0U, snapshot.GetRecords()[0U], snapshot.GetRanges()[0U]);

  std::string const slot_1 = ggems::core::sources::DescribeSourceRunSlot(
      1U, snapshot.GetRecords()[1U], snapshot.GetRanges()[1U]);

  EXPECT_EQ(slot_0,
            std::string{"Source slot: 0 | Projection primary begin: 0 | "} +
                std::string{k_window_source_description});

  EXPECT_EQ(slot_1,
            std::string{"Source slot: 1 | Projection primary begin: 7 | "} +
                std::string{k_window_source_description});

  EXPECT_NE(slot_0, slot_1);
}

// =============================================================================
// =============================================================================

TEST(GGEMSSourceDescription, ReflectsSequentialSourceMutation) {
  auto source = MakeConfiguredSource(7ULL);

  std::string const description_before = ggems::core::sources::DescribeSource(
      source->BuildRecord(), source->GetPrimaryCount());

  ReconfigureSource(*source);

  std::string const description_after = ggems::core::sources::DescribeSource(
      source->BuildRecord(), source->GetPrimaryCount());

  EXPECT_EQ(description_before, k_source_description);
  EXPECT_EQ(description_after, k_reconfigured_source_description);
  EXPECT_NE(description_before, description_after);
}

// =============================================================================
// =============================================================================

TEST(GGEMSSourceDescription, DescribesOwnedSnapshotAfterSourceMutation) {
  auto source = MakeConfiguredSource(7ULL);
  std::array<GGEMSSourcePtr, 1U> const sources{source};

  auto snapshot = ggems::core::sources::BuildSourceRunSnapshot(
      sources, {.start_ps = 1'000ULL, .stop_ps = 2'000ULL});

  ReconfigureSource(*source);

  EXPECT_EQ(ggems::core::sources::DescribeSourceRunSlot(
                0U, snapshot.GetRecords()[0U], snapshot.GetRanges()[0U]),
            std::string{"Source slot: 0 | Projection primary begin: 0 | "} +
                std::string{k_window_source_description});

  EXPECT_EQ(ggems::core::sources::DescribeSource(source->BuildRecord(),
                                                 source->GetPrimaryCount()),
            k_reconfigured_source_description);
}

// =============================================================================
// =============================================================================

TEST(GGEMSSourceDescription, DistinguishesStaticSourceFromRunWindow) {
  auto source = MakeConfiguredSource(7ULL);

  EXPECT_EQ(ggems::core::sources::DescribeSource(source->BuildRecord(),
                                                 source->GetPrimaryCount()),
            k_source_description);

  auto const snapshot = ggems::core::sources::BuildSourceRunSnapshot(
      *source, {.start_ps = 1'000ULL, .stop_ps = 2'000ULL});
  ASSERT_EQ(snapshot.GetRecords().size(), 1U);

  EXPECT_EQ(ggems::core::sources::DescribeSource(snapshot.GetRecords()[0U],
                                                 source->GetPrimaryCount()),
            k_window_source_description);
  EXPECT_EQ(source->BuildRecord().time_start_ps, 0ULL);
  EXPECT_EQ(source->BuildRecord().time_stop_ps, 0ULL);
}

// =============================================================================
// =============================================================================

TEST(GGEMSSourceDescription, DescribesGeometryAndFocusedDistribution) {
  auto source = MakeConfiguredSource(7ULL);
  source->SetEllipseEmissionPicoMeter(10'000'000'000ULL, 5'000'000'000ULL)
      .SetFocusedAngularDistributionPicoMeter(0LL, 0LL, 100'000'000'000LL);

  std::string const description = ggems::core::sources::DescribeSource(
      source->BuildRecord(), source->GetPrimaryCount());

  ExpectContains(description, "Emission: Ellipse | Diameter: 10.0000000 mm x "
                              "5.0000000 mm");
  ExpectContains(description, "Angular: Focused");
  ExpectContains(description,
                 "Focus: (0.0000000 pm, 0.0000000 pm, 100.0000000 mm)");
}

// =============================================================================
// =============================================================================

TEST(GGEMSSourceDescription, DescribesVolumeGeometryMetadata) {
  auto source = MakeConfiguredSource(7ULL);

  source->SetBoxEmissionPicoMeter(10'000'000'000ULL, 5'000'000'000ULL,
                                  2'000'000'000ULL);
  std::string box = ggems::core::sources::DescribeSource(
      source->BuildRecord(), source->GetPrimaryCount());
  ExpectContains(box, "Emission: Box | Size: 10.0000000 mm x 5.0000000 mm x "
                      "2.0000000 mm");

  source->SetSphereEmissionPicoMeter(8'000'000'000ULL);
  std::string sphere = ggems::core::sources::DescribeSource(
      source->BuildRecord(), source->GetPrimaryCount());
  ExpectContains(sphere, "Emission: Sphere | Diameter: 8.0000000 mm");

  source->SetCylinderEmissionPicoMeter(6'000'000'000ULL, 12'000'000'000ULL);
  std::string cylinder = ggems::core::sources::DescribeSource(
      source->BuildRecord(), source->GetPrimaryCount());
  ExpectContains(cylinder, "Emission: Cylinder | Diameter: 6.0000000 mm | "
                           "Height: 12.0000000 mm");
}

// =============================================================================
// =============================================================================

TEST(GGEMSSourceDescription, DescribesFullAndBoundedIsotropicDomainsInDegrees) {
  constexpr long double k_pi{std::numbers::pi_v<long double>};
  auto source = MakeConfiguredSource(7ULL);
  source->SetIsotropicAngularDistribution();

  std::string full_sphere = ggems::core::sources::DescribeSource(
      source->BuildRecord(), source->GetPrimaryCount());
  ExpectContains(full_sphere, "Angular: Isotropic | Domain: Full sphere");

  source->SetIsotropicAngularDistribution(
      ggems::units::MakeRadians(0.25L * k_pi),
      ggems::units::MakeRadians(0.5L * k_pi),
      ggems::units::MakeRadians(-0.25L * k_pi),
      ggems::units::MakeRadians(0.25L * k_pi));
  std::string bounded = ggems::core::sources::DescribeSource(
      source->BuildRecord(), source->GetPrimaryCount());

  ExpectContains(bounded, "Angular: Isotropic | Theta:");
  ExpectContains(bounded, "Phi:");
  ExpectContains(bounded, "deg");
  EXPECT_EQ(bounded.find("cos"), std::string::npos);
}
