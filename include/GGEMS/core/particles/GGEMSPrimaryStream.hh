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
  GGEMSPrimaryStream &operator=(GGEMSPrimaryStream const &) = delete;
  GGEMSPrimaryStream &operator=(GGEMSPrimaryStream &&) = delete;

public:
  void SetPrimaryCount(std::uint64_t primary_count);
  void Initialise();

  [[nodiscard]] bool IsInitialised() const noexcept { return initialised_; }

  [[nodiscard]] std::uint64_t GetPrimaryCount() const noexcept {
    return primary_count_;
  }

  [[nodiscard]] GGEMSPrimaryStreamRunView PrepareRun(std::uint64_t run_id);
  [[nodiscard]] GGEMSPrimaryStreamRunView
  PrepareRun(std::uint64_t run_id, std::uint64_t primary_count);

private:
  std::uint64_t primary_count_{4096ULL};
  std::uint64_t next_global_primary_id_{0ULL};
  bool initialised_{false};
  bool exhausted_{false};
};

} // namespace ggems::core::particles
