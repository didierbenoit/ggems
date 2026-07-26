#pragma once

#include <cstdint>

namespace ggems::core::particles {

struct GGEMSPrimaryStreamRunView {
  std::uint64_t run_id{0ULL};
  std::uint64_t source_primary_count{0ULL};
  std::uint64_t global_history_offset{0ULL};
};

class GGEMSPrimaryStream {
public:
  GGEMSPrimaryStream() = default;
  ~GGEMSPrimaryStream() = default;

  GGEMSPrimaryStream(GGEMSPrimaryStream const &) = delete;
  GGEMSPrimaryStream(GGEMSPrimaryStream &&) = delete;
  auto operator=(GGEMSPrimaryStream const &) -> GGEMSPrimaryStream & = delete;
  auto operator=(GGEMSPrimaryStream &&) -> GGEMSPrimaryStream & = delete;

  auto SetPrimaryCount(std::uint64_t primary_count) -> void;
  auto Initialise() -> void;

  [[nodiscard]] auto IsInitialised() const noexcept -> bool {
    return initialised_;
  }

  [[nodiscard]] auto GetPrimaryCount() const noexcept -> std::uint64_t {
    return primary_count_;
  }

  [[nodiscard]] auto PrepareRun(std::uint64_t run_id)
      -> GGEMSPrimaryStreamRunView;
  [[nodiscard]] auto PrepareRun(std::uint64_t run_id,
                                std::uint64_t primary_count)
      -> GGEMSPrimaryStreamRunView;

private:
  std::uint64_t primary_count_{4096ULL};
  std::uint64_t next_global_primary_id_{0ULL};
  bool initialised_{false};
  bool exhausted_{false};
};

} // namespace ggems::core::particles
