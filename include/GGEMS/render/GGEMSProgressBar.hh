#pragma once

/// \cond
#include <vector>
#include <array>
#include <memory>
#include <mutex>
#include <string>
#include <string_view>
/// \endcond

#include "GGEMS/render/GGEMSColourNames.hh"
#include "GGEMS/utf/GGEMSGlyphs.hh"
#include "GGEMS/frameworks/GGEMSOpenCLExternal.hh"
#include "GGEMS/render/GGEMSVisualLine.hh"

namespace ggems::render {

class GGEMSProgressBar {
public:
  struct SlotSnapshot;

  struct Slot {
    enum class ParticleType : std::uint8_t {
      Gamma,
      Proton,
      Electron,
      Positron,
      Neutron,
      Alpha,
      Aionino
    };

    enum class Status : std::uint8_t { Pending = 0, Running, Finished, Failed };

    static constexpr char32_t ParticleSymbol(ParticleType p) {
      switch (p) {
      case ParticleType::Gamma:
        return utf::Glyphs().gamma;
      case ParticleType::Proton:
        return utf::Glyphs().proton;
      case ParticleType::Electron:
        return utf::Glyphs().electron;
      case ParticleType::Positron:
        return utf::Glyphs().electron;
      case ParticleType::Neutron:
        return utf::Glyphs().neutron;
      case ParticleType::Alpha:
        return utf::Glyphs().alpha;
      case ParticleType::Aionino:
        return utf::Glyphs().aionino;
      }
      return U'?';
    }

    static constexpr ColourKey ParticleColour(ParticleType p) {
      switch (p) {
      case ParticleType::Gamma:
        return YELLOW_Gold_B;
      case ParticleType::Proton:
        return RED_Crimson_B;
      case ParticleType::Electron:
        return BLUE_Dodger_B;
      case ParticleType::Positron:
        return MAGENTA_Fuchsia_B;
      case ParticleType::Neutron:
        return CYAN_Frost_B;
      case ParticleType::Alpha:
        return CYAN_Marine_B;
      case ParticleType::Aionino:
        return BLUE_Ice_B;
      }
      return DEFAULT_FG;
    }

    static constexpr std::u32string ParticleName(ParticleType p) {
      switch (p) {
      case ParticleType::Gamma:
        return U"gamma";
      case ParticleType::Proton:
        return U"proton";
      case ParticleType::Electron:
        return U"electron";
      case ParticleType::Positron:
        return U"positron";
      case ParticleType::Neutron:
        return U"neutron";
      case ParticleType::Alpha:
        return U"alpha";
      case ParticleType::Aionino:
        return U"aionino";
      }
      return U"unknown";
    }

    static constexpr ColourKey StatusColour(Status status) {
      switch (status) {
      case Status::Pending:
        return CYAN_Frost;
      case Status::Running:
        return GREEN_Neon;
      case Status::Finished:
        return BLUE_Azure;
      case Status::Failed:
        return RED_Cherry;
      }
      return DEFAULT_FG;
    }

    static constexpr std::u32string StatusName(Status status) {
      switch (status) {
      case Status::Pending:
        return U"pending";
      case Status::Running:
        return U"running";
      case Status::Finished:
        return U"finished";
      case Status::Failed:
        return U"failed";
      }
      return U"undefined";
    }

    Slot(std::string_view slot_name, bool gpu,
         std::array<cl_uchar, CL_LUID_SIZE_KHR> slot_luid);

    Slot &SetKernelName(std::string_view kernel) noexcept;
    Slot &SetStatus(Status st) noexcept;
    Slot &SetParticleType(ParticleType p) noexcept;
    Slot &SetBatchesDone(std::uint64_t done) noexcept;
    Slot &SetBatchesTotal(std::uint64_t total) noexcept;
    Slot &SetETAPicoseconds(std::uint64_t eta) noexcept;
    Slot &SetIsGPU(bool gpu) noexcept;
    Slot &SetPercentVRAM(std::uint8_t percent) noexcept;
    Slot &SetTotalVRAM(std::uint64_t total) noexcept;
    Slot &SetAllocatedVRAM(std::uint64_t allocated) noexcept;
    Slot &SetAllocationCountVRAM(std::size_t allocation_count) noexcept;

    [[nodiscard]] SlotSnapshot GetSnapshot() const;

    std::string name{""};
    std::string kernel_name{""};
    Status status{Status::Pending};
    ParticleType particle_type{ParticleType::Gamma};
    bool is_gpu{false};
    std::array<cl_uchar, CL_LUID_SIZE_KHR> luid{};

    std::uint64_t batches_done{0ULL};
    std::uint64_t batches_total{0ULL};
    std::uint64_t eta_ps{0ULL};

    std::uint8_t vram_percent{0};
    std::uint64_t vram_total{0LL};
    std::uint64_t vram_allocated{0LL};
    std::size_t vram_allocation_count{0};

  private:
    mutable std::mutex mutex;
  };

  struct SlotSnapshot {
    std::string name{};
    std::string kernel_name{};
    Slot::Status status{Slot::Status::Pending};
    Slot::ParticleType particle_type{Slot::ParticleType::Gamma};
    bool is_gpu{false};
    std::array<cl_uchar, CL_LUID_SIZE_KHR> luid{};

    std::uint64_t batches_done{0ULL};
    std::uint64_t batches_total{0LL};
    std::uint64_t eta_ps{0ULL};

    std::uint8_t vram_percent{0U};
    std::uint64_t vram_total{0ULL};
    std::uint64_t vram_allocated{0ULL};
    std::size_t vram_allocation_count{0U};
  };

public:
  GGEMSProgressBar() = default;
  ~GGEMSProgressBar() = default;

  GGEMSProgressBar(GGEMSProgressBar const &) = delete;
  GGEMSProgressBar(GGEMSProgressBar &&) = delete;
  GGEMSProgressBar &operator=(GGEMSProgressBar const &) = delete;
  GGEMSProgressBar &operator=(GGEMSProgressBar &&) = delete;

public:
  Slot &AddSlot(std::string_view name, bool is_gpu,
                std::array<cl_uchar, CL_LUID_SIZE_KHR> luid);

  Slot &GetSlot(std::size_t index) noexcept;
  Slot const &GetSlot(std::size_t index) const noexcept;

  [[nodiscard]] std::size_t GetSlotCount() const noexcept;
  [[nodiscard]] std::vector<SlotSnapshot> GetSlotSnapshots() const;

  void Clear() noexcept;

  //[[nodiscard]] std::int16_t GetHeight() const noexcept;

  [[nodiscard]] std::vector<WrappedLine> BuildLines() const;

private:
  [[nodiscard]] static std::vector<char32_t> BuildBar(float progress);

  [[nodiscard]] static render::ColourKey
  GetColourStatus(std::uint8_t percent) noexcept;

  [[nodiscard]] static std::u32string FormatPercentage(float progress);
  [[nodiscard]] static std::string FormatETA(std::uint64_t ps);
  [[nodiscard]] static std::string FormatMemory(std::uint64_t bytes);
  [[nodiscard]] static std::string FormatBandwidth(long double bytes_per_ps);

private:
  std::vector<std::unique_ptr<Slot>> slots_;
  mutable std::mutex slots_mutex_;
};
} // namespace ggems::render
