#include <algorithm>
#include <array>
#include <cstddef>
#include <cstdint>
#include <filesystem>
#include <format>
#include <string>
#include <utility>

#include <gtest/gtest.h>

#include "GGEMS/opencl/GGEMSOpenCL.hh"
#include "GGEMS/opencl/GGEMSOpenCLContext.hh"
#include "GGEMS/opencl/GGEMSOpenCLExternal.hh"
#include "GGEMS/opencl/GGEMSOpenCLKernel.hh"
#include "GGEMS/opencl/GGEMSOpenCLSVMBuffer.hh"
#include "GGEMS/particles/GGEMSParticleState.hh"
#include "GGEMS/units/GGEMSBytesUnits.hh"

namespace {

using ParticleState = ggems::core::particles::GGEMSParticleState;

struct ParticleStateAlignmentProbe {
  std::uint8_t prefix;
  ParticleState particle;
};

constexpr std::size_t k_layout_value_count{23U};
constexpr std::size_t k_particle_count{2U};
constexpr std::uint64_t k_probe_track_id{7ULL};
constexpr float k_probe_direction_w{0.5F};
constexpr std::uint64_t k_probe_energy_micro_ev{511'000'000'000ULL};

// =============================================================================
// =============================================================================

class GGEMSParticleStateKernelTest : public ::testing::Test {
protected:
  static auto SetUpTestSuite() -> void {
    auto &opencl = ggems::ocl::GGEMSOpenCL::GetInstance();

    if (opencl.GetContext().empty()) {
      opencl.SelectDevices({"gpu"});
      opencl.Initialize();
    }

    ASSERT_FALSE(opencl.GetContext().empty());
  }

  static auto GetContext() -> ggems::ocl::GGEMSOpenCLContext & {
    return ggems::ocl::GGEMSOpenCL::GetInstance().GetContext().front();
  }
};

} // namespace

// =============================================================================
// =============================================================================

TEST_F(GGEMSParticleStateKernelTest, HostAndKernelLayoutsMatch) {
  auto &opencl = ggems::ocl::GGEMSOpenCL::GetInstance();
  auto &context = GetContext();

  std::filesystem::path const kernel_root{GGEMS_TEST_KERNEL_ROOT};
  std::filesystem::path const kernel_test_root = kernel_root / "tests";

  std::string const build_options =
    std::format("-I{}", kernel_root.generic_string());

  auto layout_buffer = context.CreateSVMBuffer(
    ggems::units::Bytes{k_layout_value_count * sizeof(std::uint64_t)});
  auto particles_buffer = context.CreateSVMBuffer(
    ggems::units::Bytes{k_particle_count * sizeof(ParticleState)});

  auto *layout = static_cast<std::uint64_t *>(layout_buffer.GetData());
  auto *particles = static_cast<ParticleState *>(particles_buffer.GetData());

  layout_buffer.Map(CL_MAP_WRITE);
  std::fill_n(layout, k_layout_value_count, 0ULL);
  layout_buffer.Unmap();

  particles_buffer.Map(CL_MAP_WRITE);
  std::fill_n(particles, k_particle_count, ParticleState{});
  particles_buffer.Unmap();

  auto const &program = opencl.GetOrCreateProgram(
    context, kernel_test_root, "particle_state_abi_probe", build_options);

  cl::Kernel raw_kernel = program.CreateKernel("particle_state_abi_probe");

  ggems::ocl::GGEMSOpenCLKernel kernel{context, std::move(raw_kernel),
                                       "particle_state_abi_probe"};

  kernel.SetArgSVMPointer(0U, layout);
  kernel.SetArgSVMPointer(1U, particles);
  kernel.Run({1U}, {1U});

  std::array<std::uint64_t, k_layout_value_count> const expected_layout{
    sizeof(ParticleState),
    offsetof(ParticleState, global_particle_id),
    offsetof(ParticleState, track_id),
    offsetof(ParticleState, parent_track_id),
    offsetof(ParticleState, time_ps),
    offsetof(ParticleState, position_x_pm),
    offsetof(ParticleState, position_y_pm),
    offsetof(ParticleState, position_z_pm),
    offsetof(ParticleState, particle_type),
    offsetof(ParticleState, status),
    offsetof(ParticleState, generation),
    offsetof(ParticleState, flags),
    offsetof(ParticleState, current_navigator_id),
    offsetof(ParticleState, current_volume_id),
    offsetof(ParticleState, material_id),
    offsetof(ParticleState, region_id),
    offsetof(ParticleState, direction_x),
    offsetof(ParticleState, direction_y),
    offsetof(ParticleState, direction_z),
    offsetof(ParticleState, direction_w),
    offsetof(ParticleState, energy_micro_eV),
    sizeof(ParticleState),
    offsetof(ParticleStateAlignmentProbe, particle),
  };

  layout_buffer.Map(CL_MAP_READ);
  for (std::size_t index = 0U; index < expected_layout.size(); ++index) {
    EXPECT_EQ(layout[index], expected_layout[index]) << index;
  }
  layout_buffer.Unmap();

  particles_buffer.Map(CL_MAP_READ);
  EXPECT_EQ(particles[1U].track_id, k_probe_track_id);
  EXPECT_FLOAT_EQ(particles[1U].direction_w, k_probe_direction_w);
  EXPECT_EQ(particles[1U].energy_micro_eV, k_probe_energy_micro_ev);
  particles_buffer.Unmap();
}
