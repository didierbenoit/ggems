/// \cond
#include <algorithm>
#include <cstdint>
#include <cmath>
/// \endcond

#include "GGEMS/render/GGEMSProgressBar.hh"
#include "GGEMS/core/GGEMSSystemUtils.hh"
#include "GGEMS/utf/GGEMSUTF.hh"
#include "GGEMS/core/units/GGEMSUnits.hh"

namespace ggems::render {
GGEMSProgressBar::Slot &
GGEMSProgressBar::AddSlot(std::string_view name, bool is_gpu,
                          std::array<cl_uchar, CL_LUID_SIZE_KHR> luid) {
  slots_.emplace_back();
  auto &slot = slots_.back();
  slot.name = name;
  slot.is_gpu = is_gpu;
  slot.luid = luid;
  slot.status = Slot::Status::Pending;
  return slot;
}

/* --------------------------------------------- */
/* --------------------------------------------- */
/* --------------------------------------------- */

GGEMSProgressBar::Slot &
GGEMSProgressBar::Slot::SetBatchesDone(std::uint64_t done) noexcept {
  batches_done = done;
  return *this;
}

/* --------------------------------------------- */
/* --------------------------------------------- */
/* --------------------------------------------- */

GGEMSProgressBar::Slot &
GGEMSProgressBar::Slot::SetBatchesTotal(std::uint64_t total) noexcept {
  batches_total = total;
  return *this;
}

/* --------------------------------------------- */
/* --------------------------------------------- */
/* --------------------------------------------- */

GGEMSProgressBar::Slot &
GGEMSProgressBar::Slot::SetParticleType(ParticleType p) noexcept {
  particle_type = p;
  return *this;
}

/* --------------------------------------------- */
/* --------------------------------------------- */
/* --------------------------------------------- */

GGEMSProgressBar::Slot &
GGEMSProgressBar::Slot::SetKernelName(std::string_view kernel) noexcept {
  kernel_name = kernel;
  return *this;
}

/* --------------------------------------------- */
/* --------------------------------------------- */
/* --------------------------------------------- */

GGEMSProgressBar::Slot &GGEMSProgressBar::Slot::SetIsGPU(bool gpu) noexcept {
  is_gpu = gpu;
  return *this;
}

/* --------------------------------------------- */
/* --------------------------------------------- */
/* --------------------------------------------- */

GGEMSProgressBar::Slot &
GGEMSProgressBar::Slot::SetETAPicoseconds(std::uint64_t eta) noexcept {
  eta_ps = eta;
  return *this;
}

/* --------------------------------------------- */
/* --------------------------------------------- */
/* --------------------------------------------- */

GGEMSProgressBar::Slot &
GGEMSProgressBar::Slot::SetPercentVRAM(std::uint8_t percent) noexcept {
  vram_percent = percent;
  return *this;
}

/* --------------------------------------------- */
/* --------------------------------------------- */
/* --------------------------------------------- */

GGEMSProgressBar::Slot &
GGEMSProgressBar::Slot::SetTotalVRAM(std::uint64_t total) noexcept {
  vram_total = total;
  return *this;
}

/* --------------------------------------------- */
/* --------------------------------------------- */
/* --------------------------------------------- */

GGEMSProgressBar::Slot &
GGEMSProgressBar::Slot::SetAllocatedVRAM(std::uint64_t allocated) noexcept {
  vram_allocated = allocated;
  return *this;
}

/* --------------------------------------------- */
/* --------------------------------------------- */
/* --------------------------------------------- */

GGEMSProgressBar::Slot &GGEMSProgressBar::Slot::SetAllocationCountVRAM(
    std::size_t allocation_count) noexcept {
  vram_allocation_count = allocation_count;
  return *this;
}

/* --------------------------------------------- */
/* --------------------------------------------- */
/* --------------------------------------------- */

GGEMSProgressBar::Slot &GGEMSProgressBar::Slot::SetStatus(Status st) noexcept {
  status = st;
  return *this;
}

// ----- Progress Bar ------------------------------

/* --------------------------------------------- */
/* --------------------------------------------- */
/* --------------------------------------------- */

GGEMSProgressBar::Slot &GGEMSProgressBar::GetSlot(std::size_t index) noexcept {
  return slots_[index];
}

/* --------------------------------------------- */
/* --------------------------------------------- */
/* --------------------------------------------- */

GGEMSProgressBar::Slot const &
GGEMSProgressBar::GetSlot(std::size_t index) const noexcept {
  return slots_[index];
}

/* --------------------------------------------- */
/* --------------------------------------------- */
/* --------------------------------------------- */

std::size_t GGEMSProgressBar::GetSlotCount() const noexcept {
  return slots_.size();
}

/* --------------------------------------------- */
/* --------------------------------------------- */
/* --------------------------------------------- */

void GGEMSProgressBar::Clear() noexcept { slots_.clear(); }

/* --------------------------------------------- */
/* --------------------------------------------- */
/* --------------------------------------------- */

std::int16_t GGEMSProgressBar::GetHeight() const noexcept {
  if (slots_.empty()) {
    return footer_rows_;
  }
  return static_cast<std::int16_t>(
      static_cast<std::int16_t>(slots_.size()) * slot_rows_ + footer_rows_);
}

/* --------------------------------------------- */
/* --------------------------------------------- */
/* --------------------------------------------- */

void GGEMSProgressBar::Draw(GGEMSTerminalFramebuffer &framebuffer,
                            std::int16_t x, std::int16_t y,
                            std::int16_t width) const {
  if (width <= 0) {
    return;
  }

  std::int16_t row = y;
  for (auto const &slot : slots_) {
    DrawSingleSlot(framebuffer, slot, x, row, width);
    row = static_cast<std::int16_t>(row + slot_rows_);
  }

  DrawSystemStats(framebuffer, x, row, width);
}

/* --------------------------------------------- */
/* --------------------------------------------- */
/* --------------------------------------------- */

void GGEMSProgressBar::DrawSystemStats(GGEMSTerminalFramebuffer &framebuffer,
                                       std::int16_t x, std::int16_t y,
                                       std::int16_t width) const {
  if (width <= 0) {
    return;
  }

  core::SystemUsage system_usage = core::GetSystemUsage();

  // CPU global
  std::uint8_t cpu_percent = system_usage.cpu_percent;
  std::u32string cpu_percent_text =
      utf::UTF8ToUTF32(std::format("{:3}%", cpu_percent));

  framebuffer.DrawString(x, y, U"CPU:");
  framebuffer.DrawString(x + 5, y, cpu_percent_text,
                         GetColourStatus(cpu_percent));

  // RAM global
  std::uint8_t ram_percent = system_usage.ram.percent;
  std::u32string ram_percent_text =
      utf::UTF8ToUTF32(std::format("{:3}%", ram_percent));

  framebuffer.DrawString(x + 10, y, U"| VRAM:");
  framebuffer.DrawString(x + 18, y, ram_percent_text,
                         GetColourStatus(ram_percent));

  std::uint64_t ram_total = system_usage.ram.total;
  std::uint64_t ram_used = system_usage.ram.used;
  std::u32string ram_text = utf::UTF8ToUTF32(
      std::format("({}/{})", FormatMemory(ram_used), FormatMemory(ram_total)));
  framebuffer.DrawString(x + 23, y, ram_text);
}

/* --------------------------------------------- */
/* --------------------------------------------- */
/* --------------------------------------------- */

void GGEMSProgressBar::DrawSingleSlot(GGEMSTerminalFramebuffer &framebuffer,
                                      Slot const &slot, std::int16_t x,
                                      std::int16_t y,
                                      std::int16_t width) const {

  if (width <= 8) {
    return;
  }

  bool is_gpu = slot.is_gpu;
  ColourKey dev_colour = is_gpu ? GREEN_Neon : BLUE_Azure;
  ColourKey bar_colour = is_gpu ? GREEN_Neon_B : BLUE_Azure_B;

  // Line 0 : Device
  {
    std::u32string header = utf::UTF8ToUTF32(std::format("{}", slot.name));

    framebuffer.DrawString(x, y, header, dev_colour);
  }

  // Line 1 : Status + Bar + % + type + status text
  {
    framebuffer.DrawString(x, static_cast<std::int16_t>(y + 1), U"Status:");

    // Building progress bar
    std::uint64_t done = slot.batches_done;
    std::uint64_t total = slot.batches_total;

    float progress = (total > 0ULL)
                         ? static_cast<float>(done) / static_cast<float>(total)
                         : 0.0f;

    auto bar_cells = BuildBar(progress);
    std::int16_t bar_x = static_cast<std::int16_t>(x + 8);

    std::int16_t cx = bar_x;
    for (char32_t c : bar_cells) {
      framebuffer.DrawChar(cx, static_cast<std::int16_t>(y + 1), c, bar_colour);
      ++cx;
    }

    // Percentage
    std::u32string pct32 = FormatPercentage(progress);

    framebuffer.DrawString(static_cast<std::int16_t>(bar_x + 42),
                           static_cast<std::int16_t>(y + 1), pct32);

    // Particle name
    Slot::ParticleType particle_type = slot.particle_type;
    std::u32string particle_name = Slot::ParticleName(particle_type);
    char32_t particle_symbol{Slot::ParticleSymbol(particle_type)};
    ColourKey particle_color = Slot::ParticleColour(slot.particle_type);

    char32_t sign{};
    if (particle_name == U"electron") {
      sign = utf::Glyphs().minus;
    } else if (particle_name == U"positron") {
      sign = utf::Glyphs().plus;
    }

    std::u32string particle_info = U"(" + std::u32string(1, particle_symbol) +
                                   std::u32string(1, sign) + U") " +
                                   particle_name;

    framebuffer.DrawString(static_cast<std::int16_t>(bar_x + 52),
                           static_cast<std::int16_t>(y + 1), particle_info,
                           particle_color);
  }

  // Line 2
  {
    framebuffer.DrawChar(x, static_cast<std::int16_t>(y + 2),
                         utf::Glyphs().sub_arrow);

    std::u32string kernel_txt =
        utf::UTF8ToUTF32(std::format("Kernel: {} ", slot.kernel_name));

    framebuffer.DrawString(static_cast<std::int16_t>(x + 2),
                           static_cast<std::int16_t>(y + 2), kernel_txt);

    std::u32string status_txt = U"(" + Slot::StatusName(slot.status) + U")";

    framebuffer.DrawString(x + static_cast<std::int16_t>(
                                   static_cast<int16_t>(kernel_txt.size()) + 2),
                           y + 2, status_txt, Slot::StatusColour(slot.status));
  }

  // Line 3
  {
    framebuffer.DrawChar(static_cast<std::int16_t>(x + 4),
                         static_cast<std::int16_t>(y + 3),
                         utf::Glyphs().sub_arrow);

    std::u32string batch_txt = utf::UTF8ToUTF32(
        std::format("Batches: {} / {} | ETA: {}", slot.batches_done,
                    slot.batches_total, FormatETA(slot.eta_ps)));

    framebuffer.DrawString(static_cast<std::int16_t>(x + 6),
                           static_cast<std::int16_t>(y + 3), batch_txt);
  }

  // Line 4 Process stats
  {
    framebuffer.DrawString(x, static_cast<std::int16_t>(y + 4), U"VRAM:");

    std::u32string vram_percent_text =
        utf::UTF8ToUTF32(std::format("{:3}%", slot.vram_percent));

    framebuffer.DrawString(static_cast<std::int16_t>(x + 5),
                           static_cast<std::int16_t>(y + 4), vram_percent_text,
                           GetColourStatus(slot.vram_percent));

    std::uint64_t vram_total = slot.vram_total;
    std::uint64_t vram_used = slot.vram_allocated;
    std::size_t vram_allocation_count = slot.vram_allocation_count;

    std::u32string vram_stat_text = utf::UTF8ToUTF32(
        std::format("({}/{}) SVM buffers: {}", FormatMemory(vram_used),
                    FormatMemory(vram_total), vram_allocation_count));

    framebuffer.DrawString(static_cast<std::int16_t>(x + 10),
                           static_cast<std::int16_t>(y + 4), vram_stat_text);
  }
} // namespace ggems::render

/* --------------------------------------------- */
/* --------------------------------------------- */
/* --------------------------------------------- */

/*std::chrono::milliseconds GGEMSProgressBar::ComputeFrameTime() const noexcept
{ using namespace std::chrono_literals;

  if (slots_.empty())
    return min_frame_time_;

  std::uint64_t max_eta_ps = 0ULL;
  for (auto const &s : slots_) {
    std::uint64_t eta = s.eta_ps_.load(std::memory_order_relaxed);
    if (eta > max_eta_ps)
      max_eta_ps = eta;
  }

  // Thresholds (in picoseconds)
  constexpr uint64_t five_min_ps = 300'000'000'000'000ULL;        //   5 min
  constexpr uint64_t thirty_min_ps = 1'800'000'000'000'000ULL;    //  30 min
  constexpr uint64_t two_hours_ps = 7'200'000'000'000'000ULL;     //   2 h
  constexpr uint64_t twelve_hours_ps = 43'200'000'000'000'000ULL; //  12 h

  // Apply your exact “staircase” rules.
  if (max_eta_ps < five_min_ps) {
    return min_frame_time_;
  }
  if (max_eta_ps < thirty_min_ps) {
    return 15s;
  }
  if (max_eta_ps < two_hours_ps) {
    return 80s;
  }
  if (max_eta_ps < twelve_hours_ps) {
    return 150s;
  }

  return max_frame_time_;
}*/

/* --------------------------------------------- */
/* --------------------------------------------- */
/* --------------------------------------------- */

std::vector<char32_t> GGEMSProgressBar::BuildBar(float progress) {
  constexpr std::size_t width = 40U;

  char32_t full = utf::Glyphs().block_filled;
  char32_t empty = utf::Glyphs().block_empty;

  std::vector<char32_t> cells;
  cells.reserve(width);

  float clamped = std::clamp(progress, 0.0f, 1.0f);
  std::size_t filled =
      static_cast<std::size_t>(std::floor(clamped * static_cast<float>(width)));

  for (std::size_t i = 0; i < width; ++i) {
    cells.emplace_back(i < filled ? full : empty);
  }

  return cells;
}

/* --------------------------------------------- */
/* --------------------------------------------- */
/* --------------------------------------------- */

render::ColourKey
GGEMSProgressBar::GetColourStatus(std::uint8_t percent) noexcept {
  if (percent < 33U) {
    return render::GREEN_Neon;
  } else if (percent < 64U) {
    return render::YELLOW_Neon;
  } else if (percent < 95U) {
    return render::RED_Cherry;
  }
  return render::RED_Dark;
}

/* --------------------------------------------- */
/* --------------------------------------------- */
/* --------------------------------------------- */

std::u32string GGEMSProgressBar::FormatPercentage(float progress) {
  float clamped = std::clamp(progress, 0.0F, 1.0F);
  std::string txt = std::format("{:6.2f}%", clamped * 100.0F);
  return utf::UTF8ToUTF32(txt);
}

/* --------------------------------------------- */
/* --------------------------------------------- */
/* --------------------------------------------- */

std::string GGEMSProgressBar::FormatETA(std::uint64_t ps) {
  if (ps == 0) {
    return "--";
  }

  return HumanReadable(units::Time{ps}, 1, 5);
}

/* --------------------------------------------- */
/* --------------------------------------------- */
/* --------------------------------------------- */

std::string GGEMSProgressBar::FormatMemory(std::uint64_t bytes) {
  if (bytes == 0) {
    return "--";
  }

  return HumanReadable(units::Bytes{bytes}, 1, 4);
}

/* --------------------------------------------- */
/* --------------------------------------------- */
/* --------------------------------------------- */

std::string GGEMSProgressBar::FormatBandwidth(long double bytes_per_ps) {
  if (bytes_per_ps <= 0.0L) {
    return "--";
  }

  return HumanReadable(units::Bandwidth{bytes_per_ps}, 1, 5);
}
} // namespace ggems::render
