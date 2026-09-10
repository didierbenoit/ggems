#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <filesystem>
#include <format>
#include <string>
#include <utility>

#include <gtest/gtest.h>

#include "GGEMS/opencl/GGEMSOpenCLLaunchGeometry.hh"
#include "GGEMS/particles/GGEMSParticleState.hh"
#include "GGEMS/particles/GGEMSParticleTypes.hh"
#include "GGEMS/units/GGEMSBytesUnits.hh"
#include "GGEMS/opencl/GGEMSOpenCL.hh"
#include "GGEMS/opencl/GGEMSOpenCLExternal.hh"
#include "GGEMS/opencl/GGEMSOpenCLKernel.hh"
#include "GGEMS/opencl/GGEMSOpenCLSVMBuffer.hh"

namespace {

using ParticleState = ggems::core::particles::GGEMSParticleState;

constexpr std::size_t k_particle_count{1024U};
constexpr std::size_t k_local_size{64U};

constexpr std::uint64_t k_global_particle_offset{987654ULL};
constexpr std::uint64_t k_energy_micro_eV{511'000'000'000ULL};

/* --------------------------------------------- */
/* --------------------------------------------- */
/* --------------------------------------------- */

class GGEMSParticleDummyTrackingKernelTest : public ::testing::Test {
protected:
  static void SetUpTestSuite() {
    auto &opencl = ggems::ocl::GGEMSOpenCL::GetInstance();

    if (opencl.GetContext().empty()) {
      opencl.SelectDevices({"gpu"});
      opencl.Initialize();
    }

    ASSERT_FALSE(opencl.GetContext().empty());
  }

  static ggems::ocl::GGEMSOpenCLContext &GetContext() {
    auto &opencl = ggems::ocl::GGEMSOpenCL::GetInstance();

    return opencl.GetContext().front();
  }
};

} // namespace

/* --------------------------------------------- */
/* --------------------------------------------- */
/* --------------------------------------------- */

TEST_F(GGEMSParticleDummyTrackingKernelTest, KillsAlivePrimaryParticles) {
  auto &opencl = ggems::ocl::GGEMSOpenCL::GetInstance();
  auto &context = GetContext();

  std::filesystem::path kernel_root{GGEMS_TEST_KERNEL_ROOT};
  std::filesystem::path kernel_test_root = kernel_root / "tests";

  std::string build_options = std::format("-I{}", kernel_root.generic_string());

  std::size_t particle_bytes = k_particle_count * sizeof(ParticleState);

  auto particles_buffer =
      context.CreateSVMBuffer(ggems::units::Bytes{particle_bytes});

  auto active_count_buffer =
      context.CreateSVMBuffer(ggems::units::Bytes{sizeof(std::uint32_t)});

  auto *particles = static_cast<ParticleState *>(particles_buffer.GetData());
  auto *active_count =
      static_cast<std::uint32_t *>(active_count_buffer.GetData());

  particles_buffer.Map(CL_MAP_WRITE);
  std::fill(particles, particles + k_particle_count, ParticleState{});
  particles_buffer.Unmap();

  active_count_buffer.Map(CL_MAP_WRITE);
  active_count[0] = 999U;
  active_count_buffer.Unmap();

  auto const padded_global_work_size =
      ggems::ocl::detail::TryComputePaddedGlobalWorkSize(k_particle_count,
                                                         k_local_size);
  ASSERT_TRUE(padded_global_work_size.has_value());
  std::size_t const global_size = *padded_global_work_size;

  std::uint32_t particle_type = ggems::core::particles::ToKernelParticleType(
      ggems::core::particles::GGEMSParticleType::Aionino);

  {
    auto &program = opencl.GetOrCreateProgram(context, kernel_test_root,
                                              "particle_generate_dummy_primary",
                                              build_options);

    cl::Kernel raw_kernel =
        program.CreateKernel("particle_generate_dummy_primary");

    ggems::ocl::GGEMSOpenCLKernel kernel{context, std::move(raw_kernel),
                                         "particle_generate_dummy_primary"};

    kernel.SetArgSVMPointer(0U, particles);
    kernel.SetArg(1U, static_cast<cl_ulong>(k_global_particle_offset));
    kernel.SetArg(2U, static_cast<cl_uint>(k_particle_count));
    kernel.SetArg(3U, static_cast<cl_uint>(particle_type));
    kernel.SetArg(4U, static_cast<cl_ulong>(k_energy_micro_eV));

    kernel.Run({global_size}, {k_local_size});
  }

  {
    auto &program = opencl.GetOrCreateProgram(
        context, kernel_test_root, "particle_dummy_kill_alive", build_options);

    cl::Kernel raw_kernel = program.CreateKernel("particle_dummy_kill_alive");

    ggems::ocl::GGEMSOpenCLKernel kernel{context, std::move(raw_kernel),
                                         "particle_dummy_kill_alive"};

    kernel.SetArgSVMPointer(0U, particles);
    kernel.SetArgSVMPointer(1U, active_count);
    kernel.SetArg(2U, static_cast<cl_uint>(k_particle_count));

    kernel.Run({global_size}, {k_local_size});
  }

  active_count_buffer.Map(CL_MAP_READ);
  EXPECT_EQ(active_count[0], 0U);
  active_count_buffer.Unmap();

  particles_buffer.Map(CL_MAP_READ);

  for (std::size_t i = 0U; i < k_particle_count; ++i) {
    ParticleState const &particle = particles[i];

    EXPECT_EQ(particle.global_particle_id,
              k_global_particle_offset + static_cast<std::uint64_t>(i));

    EXPECT_EQ(particle.track_id,
              k_global_particle_offset + static_cast<std::uint64_t>(i));

    EXPECT_EQ(particle.parent_track_id,
              ggems::core::particles::k_invalid_id_u64);

    EXPECT_EQ(particle.particle_type, particle_type);

    EXPECT_EQ(particle.status,
              ggems::core::particles::ToKernelParticleStatus(
                  ggems::core::particles::GGEMSParticleStatus::Killed));

    EXPECT_EQ(particle.time_ps, 1ULL);
    EXPECT_EQ(particle.flags & 1U, 1U);

    EXPECT_EQ(particle.position_x_pm, 0LL);
    EXPECT_EQ(particle.position_y_pm, 0LL);
    EXPECT_EQ(particle.position_z_pm, 0LL);

    EXPECT_FLOAT_EQ(particle.direction_x, 0.0F);
    EXPECT_FLOAT_EQ(particle.direction_y, 0.0F);
    EXPECT_FLOAT_EQ(particle.direction_z, 1.0F);
    EXPECT_FLOAT_EQ(particle.direction_w, 0.0F);

    EXPECT_EQ(particle.energy_micro_eV, k_energy_micro_eV);
    EXPECT_FLOAT_EQ(particle.weight, 1.0F);
  }

  particles_buffer.Unmap();
}
