#pragma once

#include <cstdint>
#include <type_traits>

namespace ggems::core::transport {

struct GGEMSTransportCounters {
  std::uint32_t next_primary_id{0U};

  std::uint32_t consumed_primary_count{0U};
  std::uint32_t completed_history_count{0U};
  std::uint32_t terminal_particle_count{0U};
  std::uint32_t created_secondary_count{0U};

  std::uint32_t aionino_to_gamma_count{0U};
  std::uint32_t gamma_to_electron_count{0U};
  std::uint32_t electron_to_electron_count{0U};

  std::uint32_t overflow_count{0U};
  std::uint32_t max_stack_depth{0U};
  std::uint32_t total_fake_step_count{0U};
};

static_assert(std::is_standard_layout_v<GGEMSTransportCounters>);
static_assert(std::is_trivially_copyable_v<GGEMSTransportCounters>);
static_assert(sizeof(GGEMSTransportCounters) == 44U);

} // namespace ggems::core::transport
