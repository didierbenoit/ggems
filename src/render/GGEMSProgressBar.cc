/// \cond
#include <algorithm>
#include <cstdint>
#include <cmath>
#include <format>
/// \endcond

#include "GGEMS/render/GGEMSProgressBar.hh"
#include "GGEMS/core/GGEMSSystemUtils.hh"
#include "GGEMS/utf/GGEMSUTF.hh"
#include "GGEMS/core/units/GGEMSUnits.hh"

namespace ggems::render {

/* --------------------------------------------- */
/* --------------------------------------------- */
/* --------------------------------------------- */

GGEMSProgressBar::Slot::Slot(std::string_view slot_name, bool gpu,
                             std::array<cl_uchar, CL_LUID_SIZE_KHR> slot_luid)
    : name{slot_name}, is_gpu{gpu}, luid{slot_luid} {}

/* --------------------------------------------- */
/* --------------------------------------------- */
/* --------------------------------------------- */

GGEMSProgressBar::Slot &
GGEMSProgressBar::AddSlot(std::string_view name, bool is_gpu,
                          std::array<cl_uchar, CL_LUID_SIZE_KHR> luid) {
  auto slot = std::make_unique<Slot>(name, is_gpu, luid);
  Slot &slot_ref = *slot;

  {
    std::scoped_lock lock{slots_mutex_};
    slots_.push_back(std::move(slot));
  }

  return slot_ref;
}

/* --------------------------------------------- */
/* --------------------------------------------- */
/* --------------------------------------------- */

GGEMSProgressBar::Slot &
GGEMSProgressBar::Slot::SetBatchesDone(std::uint64_t done) noexcept {
  std::scoped_lock lock{mutex};
  batches_done = done;
  return *this;
}

/* --------------------------------------------- */
/* --------------------------------------------- */
/* --------------------------------------------- */

GGEMSProgressBar::Slot &
GGEMSProgressBar::Slot::SetBatchesTotal(std::uint64_t total) noexcept {
  std::scoped_lock lock{mutex};
  batches_total = total;
  return *this;
}

/* --------------------------------------------- */
/* --------------------------------------------- */
/* --------------------------------------------- */

GGEMSProgressBar::Slot &
GGEMSProgressBar::Slot::SetParticleType(ParticleType p) noexcept {
  std::scoped_lock lock{mutex};
  particle_type = p;
  return *this;
}

/* --------------------------------------------- */
/* --------------------------------------------- */
/* --------------------------------------------- */

GGEMSProgressBar::Slot &
GGEMSProgressBar::Slot::SetKernelName(std::string_view kernel) noexcept {
  std::scoped_lock lock{mutex};
  kernel_name = kernel;
  return *this;
}

/* --------------------------------------------- */
/* --------------------------------------------- */
/* --------------------------------------------- */

GGEMSProgressBar::Slot &GGEMSProgressBar::Slot::SetIsGPU(bool gpu) noexcept {
  std::scoped_lock lock{mutex};
  is_gpu = gpu;
  return *this;
}

/* --------------------------------------------- */
/* --------------------------------------------- */
/* --------------------------------------------- */

GGEMSProgressBar::Slot &
GGEMSProgressBar::Slot::SetETAPicoseconds(std::uint64_t eta) noexcept {
  std::scoped_lock lock{mutex};
  eta_ps = eta;
  return *this;
}

/* --------------------------------------------- */
/* --------------------------------------------- */
/* --------------------------------------------- */

GGEMSProgressBar::Slot &
GGEMSProgressBar::Slot::SetPercentVRAM(std::uint8_t percent) noexcept {
  std::scoped_lock lock{mutex};
  vram_percent = percent;
  return *this;
}

/* --------------------------------------------- */
/* --------------------------------------------- */
/* --------------------------------------------- */

GGEMSProgressBar::Slot &
GGEMSProgressBar::Slot::SetTotalVRAM(std::uint64_t total) noexcept {
  std::scoped_lock lock{mutex};
  vram_total = total;
  return *this;
}

/* --------------------------------------------- */
/* --------------------------------------------- */
/* --------------------------------------------- */

GGEMSProgressBar::Slot &
GGEMSProgressBar::Slot::SetAllocatedVRAM(std::uint64_t allocated) noexcept {
  std::scoped_lock lock{mutex};
  vram_allocated = allocated;
  return *this;
}

/* --------------------------------------------- */
/* --------------------------------------------- */
/* --------------------------------------------- */

GGEMSProgressBar::Slot &GGEMSProgressBar::Slot::SetAllocationCountVRAM(
    std::size_t allocation_count) noexcept {
  std::scoped_lock lock{mutex};
  vram_allocation_count = allocation_count;
  return *this;
}

/* --------------------------------------------- */
/* --------------------------------------------- */
/* --------------------------------------------- */

GGEMSProgressBar::Slot &GGEMSProgressBar::Slot::SetStatus(Status st) noexcept {
  std::scoped_lock lock{mutex};
  status = st;
  return *this;
}

/* --------------------------------------------- */
/* --------------------------------------------- */
/* --------------------------------------------- */

GGEMSProgressBar::SlotSnapshot GGEMSProgressBar::Slot::GetSnapshot() const {
  std::scoped_lock lock{mutex};

  SlotSnapshot snapshot{};
  snapshot.name = name;
  snapshot.kernel_name = kernel_name;
  snapshot.status = status;
  snapshot.particle_type = particle_type;
  snapshot.is_gpu = is_gpu;
  snapshot.luid = luid;

  snapshot.batches_done = batches_done;
  snapshot.batches_total = batches_total;
  snapshot.eta_ps = eta_ps;

  snapshot.vram_percent = vram_percent;
  snapshot.vram_total = vram_total;
  snapshot.vram_allocated = vram_allocated;
  snapshot.vram_allocation_count = vram_allocation_count;

  return snapshot;
}

// ----- Progress Bar ------------------------------

/* --------------------------------------------- */
/* --------------------------------------------- */
/* --------------------------------------------- */

std::vector<GGEMSProgressBar::SlotSnapshot>
GGEMSProgressBar::GetSlotSnapshots() const {
  std::scoped_lock lock{slots_mutex_};

  std::vector<SlotSnapshot> snapshots{};
  snapshots.reserve(slots_.size());

  for (auto const &slot : slots_) {
    snapshots.push_back(slot->GetSnapshot());
  }

  return snapshots;
}

/* --------------------------------------------- */
/* --------------------------------------------- */
/* --------------------------------------------- */

GGEMSProgressBar::Slot &GGEMSProgressBar::GetSlot(std::size_t index) noexcept {
  std::scoped_lock lock{slots_mutex_};
  return *slots_[index];
}

/* --------------------------------------------- */
/* --------------------------------------------- */
/* --------------------------------------------- */

GGEMSProgressBar::Slot const &
GGEMSProgressBar::GetSlot(std::size_t index) const noexcept {
  std::scoped_lock lock{slots_mutex_};
  return *slots_[index];
}

/* --------------------------------------------- */
/* --------------------------------------------- */
/* --------------------------------------------- */

std::size_t GGEMSProgressBar::GetSlotCount() const noexcept {
  std::scoped_lock lock{slots_mutex_};
  return slots_.size();
}

/* --------------------------------------------- */
/* --------------------------------------------- */
/* --------------------------------------------- */

void GGEMSProgressBar::Clear() noexcept {
  std::scoped_lock lock{slots_mutex_};
  slots_.clear();
}

/* --------------------------------------------- */
/* --------------------------------------------- */
/* --------------------------------------------- */

/*std::int16_t GGEMSProgressBar::GetHeight() const noexcept {
  std::size_t slot_count = GetSlotCount();

  if (slot_count == 0U) {
    return 0;
  }

  return static_cast<std::int16_t>(
      static_cast<std::int16_t>(slot_count) * slot_rows_ + footer_rows_);
}*/

/* --------------------------------------------- */
/* --------------------------------------------- */
/* --------------------------------------------- */

/*void GGEMSProgressBar::Draw(GGEMSTerminalFramebuffer &framebuffer,
                            std::int16_t x, std::int16_t y,
                            std::int16_t width) const {
  std::vector<SlotSnapshot> snapshots = GetSlotSnapshots();

  if (width <= 0 || snapshots.empty()) {
    return;
  }

  std::int16_t row = y;
  for (auto const &slot : snapshots) {
    DrawSingleSlot(framebuffer, slot, x, row, width);
    row = static_cast<std::int16_t>(row + slot_rows_);
  }

  DrawSystemStats(framebuffer, x, row, width);
}*/

/* --------------------------------------------- */
/* --------------------------------------------- */
/* --------------------------------------------- */

std::vector<WrappedLine> GGEMSProgressBar::BuildLines() const {
  std::vector<SlotSnapshot> snapshots = GetSlotSnapshots();

  std::vector<WrappedLine> lines{};
  if (snapshots.empty()) {
    return lines;
  }

  auto append_utf32 = [](WrappedLine &line, std::u32string text,
                         ColourKey colour = DEFAULT_FG) {
    line.segments.emplace_back(
        VisualSegment{.text = std::move(text), .colour = colour});
  };

  auto append_utf8 = [&append_utf32](WrappedLine &line, std::string_view text,
                                     ColourKey colour = DEFAULT_FG) {
    append_utf32(line, utf::UTF8ToUTF32(text), colour);
  };

  for (SlotSnapshot const &slot : snapshots) {
    bool const is_gpu = slot.is_gpu;
    ColourKey const device_colour = is_gpu ? GREEN_Neon : BLUE_Azure;
    ColourKey const bar_colour = is_gpu ? GREEN_Neon_B : BLUE_Azure_B;

    // Device line.
    {
      WrappedLine line{};
      append_utf8(line, slot.name, device_colour);
      lines.emplace_back(std::move(line));
    }

    std::uint64_t const done = slot.batches_done;
    std::uint64_t const total = slot.batches_total;

    float const progress =
        (total > 0ULL) ? static_cast<float>(done) / static_cast<float>(total)
                       : 0.0F;

    // Status, progress bar, percentage and particle line.
    {
      WrappedLine line{};

      append_utf32(line, U"Status: ");

      std::vector<char32_t> const bar_cells = BuildBar(progress);
      std::u32string bar_text{};
      bar_text.reserve(bar_cells.size());

      for (char32_t const cell : bar_cells) {
        bar_text.push_back(cell);
      }

      append_utf32(line, std::move(bar_text), bar_colour);
      append_utf32(line, U" ");
      append_utf32(line, FormatPercentage(progress));

      Slot::ParticleType const particle_type = slot.particle_type;
      std::u32string const particle_name = Slot::ParticleName(particle_type);
      char32_t const particle_symbol = Slot::ParticleSymbol(particle_type);
      ColourKey const particle_colour = Slot::ParticleColour(particle_type);

      char32_t sign{};
      if (particle_name == U"electron") {
        sign = utf::Glyphs().minus;
      } else if (particle_name == U"positron") {
        sign = utf::Glyphs().plus;
      }

      std::u32string particle_info =
          U" (" + std::u32string(1, particle_symbol) +
          std::u32string(sign == U'\0' ? 0U : 1U, sign) + U") " + particle_name;

      append_utf32(line, std::move(particle_info), particle_colour);

      lines.emplace_back(std::move(line));
    }

    // Kernel and status line.
    {
      WrappedLine line{};

      append_utf8(line, std::format("  Kernel: {} ", slot.kernel_name));
      append_utf32(line, U"(");
      append_utf32(line, Slot::StatusName(slot.status),
                   Slot::StatusColour(slot.status));
      append_utf32(line, U")");

      lines.emplace_back(std::move(line));
    }

    // Batch and ETA line.
    {
      WrappedLine line{};

      append_utf8(line, std::format("    Batches: {} / {} | ETA: {}",
                                    slot.batches_done, slot.batches_total,
                                    FormatETA(slot.eta_ps)));

      lines.emplace_back(std::move(line));
    }

    // VRAM line.
    {
      WrappedLine line{};

      append_utf32(line, U"VRAM: ");
      append_utf8(line, std::format("{:3}%", slot.vram_percent),
                  GetColourStatus(slot.vram_percent));
      append_utf8(line, std::format(" ({}/{}) SVM buffers: {}",
                                    FormatMemory(slot.vram_allocated),
                                    FormatMemory(slot.vram_total),
                                    slot.vram_allocation_count));

      lines.emplace_back(std::move(line));
    }

    lines.emplace_back(WrappedLine{});
  }

  // System statistics line.
  {
    core::SystemUsage const system_usage = core::GetSystemUsage();

    WrappedLine line{};

    append_utf32(line, U"CPU: ");
    append_utf8(line, std::format("{:3}%", system_usage.cpu_percent),
                GetColourStatus(system_usage.cpu_percent));

    append_utf32(line, U" | RAM: ");
    append_utf8(line, std::format("{:3}%", system_usage.ram.percent),
                GetColourStatus(system_usage.ram.percent));

    append_utf8(line,
                std::format(" ({}/{})", FormatMemory(system_usage.ram.used),
                            FormatMemory(system_usage.ram.total)));

    lines.emplace_back(std::move(line));
  }

  return lines;
}

/* --------------------------------------------- */
/* --------------------------------------------- */
/* --------------------------------------------- */

/*void GGEMSProgressBar::DrawSystemStats(GGEMSTerminalFramebuffer &framebuffer,
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
}*/

/* --------------------------------------------- */
/* --------------------------------------------- */
/* --------------------------------------------- */

/*void GGEMSProgressBar::DrawSingleSlot(GGEMSTerminalFramebuffer &framebuffer,
                                      SlotSnapshot const &slot, std::int16_t x,
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
