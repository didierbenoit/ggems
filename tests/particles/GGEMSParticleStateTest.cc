#include <cstdlib>
#include <type_traits>

#include <gtest/gtest.h>

#include "GGEMS/particles/GGEMSParticleState.hh"

/* --------------------------------------------- */
/* --------------------------------------------- */
/* --------------------------------------------- */

TEST(GGEMSParticleState, IsKernelFriendly) {
  EXPECT_TRUE(
      std::is_standard_layout_v<ggems::core::particles::GGEMSParticleState>);
  EXPECT_TRUE(
      std::is_trivially_copyable_v<ggems::core::particles::GGEMSParticleState>);
  EXPECT_EQ(sizeof(ggems::core::particles::GGEMSParticleState), 120U);
}

/* --------------------------------------------- */
/* --------------------------------------------- */
/* --------------------------------------------- */

TEST(GGEMSParticleState, DefaultStateIsInactive) {
  ggems::core::particles::GGEMSParticleState particle{};

  EXPECT_EQ(particle.global_particle_id,
            ggems::core::particles::k_invalid_id_u64);
  EXPECT_EQ(particle.track_id, ggems::core::particles::k_invalid_id_u64);
  EXPECT_EQ(particle.parent_track_id, ggems::core::particles::k_invalid_id_u64);

  EXPECT_EQ(particle.particle_type,
            ggems::core::particles::ToKernelParticleType(
                ggems::core::particles::GGEMSParticleType::Unknown));

  EXPECT_EQ(particle.status,
            ggems::core::particles::ToKernelParticleStatus(
                ggems::core::particles::GGEMSParticleStatus::Inactive));

  EXPECT_EQ(particle.current_navigator_id,
            ggems::core::particles::k_invalid_id_u32);
  EXPECT_EQ(particle.current_volume_id,
            ggems::core::particles::k_invalid_id_u32);
  EXPECT_EQ(particle.material_id, ggems::core::particles::k_invalid_id_u32);
  EXPECT_EQ(particle.region_id, ggems::core::particles::k_invalid_id_u32);

  EXPECT_EQ(particle.time_ps, 0ULL);
  EXPECT_FLOAT_EQ(particle.direction_x, 0.0F);
  EXPECT_FLOAT_EQ(particle.direction_y, 0.0F);
  EXPECT_FLOAT_EQ(particle.direction_z, 1.0F);
  EXPECT_FLOAT_EQ(particle.direction_w, 0.0F);
  EXPECT_EQ(particle.energy_milli_eV, 0ULL);
  EXPECT_FLOAT_EQ(particle.weight, 1.0F);
}

/* --------------------------------------------- */
/* --------------------------------------------- */
/* --------------------------------------------- */

TEST(GGEMSParticleState, ParticleTypeValuesAreKernelCompatible) {
  EXPECT_EQ(ggems::core::particles::ToKernelParticleType(
                ggems::core::particles::GGEMSParticleType::Unknown),
            0U);

  EXPECT_EQ(ggems::core::particles::ToKernelParticleType(
                ggems::core::particles::GGEMSParticleType::Aionino),
            1U);

  EXPECT_EQ(ggems::core::particles::ToKernelParticleType(
                ggems::core::particles::GGEMSParticleType::Gamma),
            2U);

  EXPECT_EQ(ggems::core::particles::ToKernelParticleType(
                ggems::core::particles::GGEMSParticleType::Electron),
            3U);

  EXPECT_EQ(ggems::core::particles::ToKernelParticleType(
                ggems::core::particles::GGEMSParticleType::Positron),
            4U);

  EXPECT_EQ(ggems::core::particles::ToKernelParticleType(
                ggems::core::particles::GGEMSParticleType::Proton),
            5U);

  EXPECT_EQ(ggems::core::particles::ToKernelParticleType(
                ggems::core::particles::GGEMSParticleType::Neutron),
            6U);

  EXPECT_EQ(ggems::core::particles::ToKernelParticleType(
                ggems::core::particles::GGEMSParticleType::Alpha),
            7U);

  EXPECT_EQ(ggems::core::particles::ToKernelParticleStatus(
                ggems::core::particles::GGEMSParticleStatus::Inactive),
            0U);

  EXPECT_EQ(ggems::core::particles::ToKernelParticleStatus(
                ggems::core::particles::GGEMSParticleStatus::Alive),
            1U);

  EXPECT_EQ(ggems::core::particles::ToKernelParticleStatus(
                ggems::core::particles::GGEMSParticleStatus::Killed),
            2U);

  EXPECT_EQ(ggems::core::particles::ToKernelParticleStatus(
                ggems::core::particles::GGEMSParticleStatus::EscapedWorld),
            3U);

  EXPECT_EQ(ggems::core::particles::ToKernelParticleStatus(
                ggems::core::particles::GGEMSParticleStatus::Absorbed),
            4U);
}
