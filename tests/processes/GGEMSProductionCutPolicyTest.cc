#include <array>
#include <cstddef>
#include <cstdint>
#include <optional>
#include <type_traits>
#include <tuple>

#include <gtest/gtest.h>

#include "GGEMS/GGEMSException.hh"
#include "GGEMS/processes/GGEMSProductionCutPolicy.hh"
#include "GGEMS/units/GGEMSLengthUnits.hh"

namespace {

// =============================================================================
// =============================================================================

namespace processes = ggems::core::processes;
namespace units = ggems::units;

using namespace ggems::units;
using Channel = processes::GGEMSProductionCutChannel;

// =============================================================================
// =============================================================================

auto Resolved(processes::GGEMSResolvedProductionCutLengths const &lengths,
              Channel channel) -> units::Length {
  return lengths[processes::ProductionCutChannelIndex(channel)];
}

// =============================================================================
// =============================================================================

auto MakeGlobalPolicy() -> processes::GGEMSProductionCutPolicy {
  return {
    .global =
      {
        .gamma = 1_mm,
        .electron = 2_mm,
        .positron = 3_mm,
        .proton = 4_mm,
      },
    .materials = {},
  };
}

} // namespace

// =============================================================================
// =============================================================================

TEST(GGEMSProductionCutPolicyTest, DefinesExactlyFourOrderedChannels) {
  static_assert(std::is_same_v<std::underlying_type_t<Channel>, std::uint8_t>);

  EXPECT_EQ(processes::k_production_cut_channels.size(), 4U);
  EXPECT_EQ(processes::k_production_cut_channels,
            (std::array{Channel::Gamma, Channel::Electron, Channel::Positron,
                        Channel::Proton}));

  for (std::size_t index = 0U;
       index < processes::k_production_cut_channels.size(); ++index) {
    EXPECT_EQ(processes::ProductionCutChannelIndex(
                processes::k_production_cut_channels[index]),
              index);
  }

  static_assert(
    std::tuple_size_v<processes::GGEMSResolvedProductionCutLengths> == 4U);
}

// =============================================================================
// =============================================================================

TEST(GGEMSProductionCutPolicyTest, AuthoringUsesCanonicalLength) {
  static_assert(
    std::is_same_v<decltype(processes::GGEMSProductionCutLengths::gamma),
                   std::optional<units::Length>>);
  static_assert(
    std::is_same_v<processes::GGEMSResolvedProductionCutLengths::value_type,
                   units::Length>);
}

// =============================================================================
// =============================================================================

TEST(GGEMSProductionCutPolicyTest, GlobalOnlyResolvesEveryChannel) {
  auto const resolved = processes::ResolveProductionCutLengths(
    MakeGlobalPolicy(), {.material_index = 0U, .volume = {}});

  EXPECT_EQ(Resolved(resolved, Channel::Gamma), 1_mm);
  EXPECT_EQ(Resolved(resolved, Channel::Electron), 2_mm);
  EXPECT_EQ(Resolved(resolved, Channel::Positron), 3_mm);
  EXPECT_EQ(Resolved(resolved, Channel::Proton), 4_mm);
}

// =============================================================================
// =============================================================================

TEST(GGEMSProductionCutPolicyTest, MaterialOverrideChangesOnlyItsChannel) {
  auto policy = MakeGlobalPolicy();
  policy.materials.push_back({
    .material_index = 2U,
    .lengths =
      {
        .gamma = std::nullopt,
        .electron = 20_um,
        .positron = std::nullopt,
        .proton = std::nullopt,
      },
  });

  auto const overridden = processes::ResolveProductionCutLengths(
    policy, {.material_index = 2U, .volume = {}});

  EXPECT_EQ(Resolved(overridden, Channel::Gamma), 1_mm);
  EXPECT_EQ(Resolved(overridden, Channel::Electron), 20_um);
  EXPECT_EQ(Resolved(overridden, Channel::Positron), 3_mm);
  EXPECT_EQ(Resolved(overridden, Channel::Proton), 4_mm);

  auto const other = processes::ResolveProductionCutLengths(
    policy, {.material_index = 1U, .volume = {}});

  EXPECT_EQ(Resolved(other, Channel::Electron), 2_mm);
}

// =============================================================================
// =============================================================================

TEST(GGEMSProductionCutPolicyTest, VolumeOverrideChangesOnlyItsChannel) {
  auto const resolved = processes::ResolveProductionCutLengths(
    MakeGlobalPolicy(), {
                          .material_index = 0U,
                          .volume =
                            {
                              .gamma = 7_um,
                              .electron = std::nullopt,
                              .positron = std::nullopt,
                              .proton = std::nullopt,
                            },
                        });

  EXPECT_EQ(Resolved(resolved, Channel::Gamma), 7_um);
  EXPECT_EQ(Resolved(resolved, Channel::Electron), 2_mm);
  EXPECT_EQ(Resolved(resolved, Channel::Positron), 3_mm);
  EXPECT_EQ(Resolved(resolved, Channel::Proton), 4_mm);
}

// =============================================================================
// =============================================================================

TEST(GGEMSProductionCutPolicyTest, PrecedenceIsVolumeMaterialGlobalPerChannel) {
  auto policy = MakeGlobalPolicy();
  policy.materials.push_back({
    .material_index = 5U,
    .lengths =
      {
        .gamma = 10_um,
        .electron = 20_um,
        .positron = 30_um,
        .proton = std::nullopt,
      },
  });

  auto const resolved = processes::ResolveProductionCutLengths(
    policy, {
              .material_index = 5U,
              .volume =
                {
                  .gamma = 100_nm,
                  .electron = std::nullopt,
                  .positron = std::nullopt,
                  .proton = 400_nm,
                },
            });

  EXPECT_EQ(Resolved(resolved, Channel::Gamma), 100_nm);   // Volume
  EXPECT_EQ(Resolved(resolved, Channel::Electron), 20_um); // Material
  EXPECT_EQ(Resolved(resolved, Channel::Positron), 30_um); // Material
  EXPECT_EQ(Resolved(resolved, Channel::Proton), 400_nm);  // Volume over Global
}

// =============================================================================
// =============================================================================

TEST(GGEMSProductionCutPolicyTest, ProvenanceDoesNotChangeResolvedLengths) {
  auto global_policy = MakeGlobalPolicy();
  global_policy.global.electron = 50_um;

  auto material_policy = MakeGlobalPolicy();
  material_policy.materials.push_back({
    .material_index = 0U,
    .lengths =
      {
        .gamma = std::nullopt,
        .electron = 50_um,
        .positron = std::nullopt,
        .proton = std::nullopt,
      },
  });

  auto const from_global = processes::ResolveProductionCutLengths(
    global_policy, {.material_index = 0U, .volume = {}});
  auto const from_material = processes::ResolveProductionCutLengths(
    material_policy, {.material_index = 0U, .volume = {}});
  auto const from_volume = processes::ResolveProductionCutLengths(
    MakeGlobalPolicy(), {
                          .material_index = 0U,
                          .volume =
                            {
                              .gamma = std::nullopt,
                              .electron = 50_um,
                              .positron = std::nullopt,
                              .proton = std::nullopt,
                            },
                        });

  EXPECT_EQ(from_global, from_material);
  EXPECT_EQ(from_global, from_volume);
}

// =============================================================================
// =============================================================================

TEST(GGEMSProductionCutPolicyTest, ReportsTheWinningScopeOfEveryChannel) {
  using Scope = processes::GGEMSProductionCutScope;

  auto policy = MakeGlobalPolicy();
  policy.materials.push_back({
    .material_index = 5U,
    .lengths =
      {
        .gamma = std::nullopt,
        .electron = 20_um,
        .positron = 30_um,
        .proton = std::nullopt,
      },
  });

  processes::GGEMSProductionCutContext const context{
    .material_index = 5U,
    .volume =
      {
        .gamma = std::nullopt,
        .electron = std::nullopt,
        .positron = 300_nm,
        .proton = std::nullopt,
      },
  };

  auto const resolved = processes::ResolveProductionCuts(policy, context);

  EXPECT_EQ(resolved.lengths, (processes::GGEMSResolvedProductionCutLengths{
                                1_mm, 20_um, 300_nm, 4_mm}));
  EXPECT_EQ(resolved.scopes, (std::array{Scope::Global, Scope::Material,
                                         Scope::Volume, Scope::Global}));
  EXPECT_EQ(resolved.lengths,
            processes::ResolveProductionCutLengths(policy, context));

  auto const unrelated = processes::ResolveProductionCuts(
    policy, {.material_index = 6U, .volume = {}});
  EXPECT_EQ(unrelated.scopes, (std::array{Scope::Global, Scope::Global,
                                          Scope::Global, Scope::Global}));
}

// =============================================================================
// =============================================================================

TEST(GGEMSProductionCutPolicyTest, AnOverrideWinsEvenWithTheGlobalValue) {
  using Scope = processes::GGEMSProductionCutScope;

  auto policy = MakeGlobalPolicy();
  policy.materials.push_back({
    .material_index = 0U,
    .lengths =
      {
        .gamma = 1_mm,
        .electron = std::nullopt,
        .positron = std::nullopt,
        .proton = std::nullopt,
      },
  });

  auto const resolved = processes::ResolveProductionCuts(
    policy, {
              .material_index = 0U,
              .volume =
                {
                  .gamma = std::nullopt,
                  .electron = 2_mm,
                  .positron = units::Length{.value = 0U},
                  .proton = std::nullopt,
                },
            });

  EXPECT_EQ(resolved.scopes[0], Scope::Material);
  EXPECT_EQ(resolved.scopes[1], Scope::Volume);
  EXPECT_EQ(resolved.scopes[2], Scope::Volume);
  EXPECT_EQ(resolved.lengths[2].value, 0U);
  EXPECT_EQ(resolved.scopes[3], Scope::Global);
}

// =============================================================================
// =============================================================================

TEST(GGEMSProductionCutPolicyTest, NamesChannelsAndScopes) {
  using Scope = processes::GGEMSProductionCutScope;

  EXPECT_EQ(processes::ProductionCutChannelName(Channel::Gamma), "Gamma");
  EXPECT_EQ(processes::ProductionCutChannelName(Channel::Proton), "Proton");
  EXPECT_EQ(processes::ProductionCutScopeName(Scope::Global), "Global");
  EXPECT_EQ(processes::ProductionCutScopeName(Scope::Material), "Material");
  EXPECT_EQ(processes::ProductionCutScopeName(Scope::Volume), "Volume");
}

// =============================================================================
// =============================================================================

TEST(GGEMSProductionCutPolicyTest, IncompleteGlobalPolicyIsRejected) {
  for (auto const channel : processes::k_production_cut_channels) {
    SCOPED_TRACE(processes::ProductionCutChannelIndex(channel));

    auto policy = MakeGlobalPolicy();
    switch (channel) {
    case Channel::Gamma:
      policy.global.gamma.reset();
      break;
    case Channel::Electron:
      policy.global.electron.reset();
      break;
    case Channel::Positron:
      policy.global.positron.reset();
      break;
    case Channel::Proton:
      policy.global.proton.reset();
      break;
    }

    // Overrides covering the missing channel do not replace Global coverage.
    EXPECT_THROW(static_cast<void>(processes::ResolveProductionCutLengths(
                   policy, {.material_index = 0U,
                            .volume = {.gamma = 1_mm,
                                       .electron = 1_mm,
                                       .positron = 1_mm,
                                       .proton = 1_mm}})),
                 ggems::core::GGEMSRecoverable);
  }
}

// =============================================================================
// =============================================================================

TEST(GGEMSProductionCutPolicyTest, DuplicateMaterialOverrideIsRejected) {
  auto policy = MakeGlobalPolicy();
  policy.materials.push_back({
    .material_index = 3U,
    .lengths =
      {
        .gamma = 1_um,
        .electron = std::nullopt,
        .positron = std::nullopt,
        .proton = std::nullopt,
      },
  });
  policy.materials.push_back({
    .material_index = 3U,
    .lengths =
      {
        .gamma = std::nullopt,
        .electron = std::nullopt,
        .positron = std::nullopt,
        .proton = 1_um,
      },
  });

  EXPECT_THROW(static_cast<void>(processes::ResolveProductionCutLengths(
                 policy, {.material_index = 0U, .volume = {}})),
               ggems::core::GGEMSRecoverable);
}
