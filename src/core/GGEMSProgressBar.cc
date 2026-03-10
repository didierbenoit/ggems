/// \cond
#include <algorithm>
#include <cstdint>
#include <iostream>
#include <cmath>
/// \endcond

#include "GGEMS/core/GGEMSProgressBar.hh"
#include "GGEMS/core/GGEMSSystemUtils.hh"
#include "GGEMS/utf/GGEMSUTF.hh"
#include "GGEMS/core/units/GGEMSUnits.hh"

namespace ggems::core {
GGEMSProgressBar::Slot &
GGEMSProgressBar::AddSlot(std::string_view name, bool is_gpu,
                          std::array<cl_uchar, CL_LUID_SIZE_KHR> luid) {
  slots_.emplace_back();
  auto &slot = slots_.back();
  slot.name_ = name;
  slot.is_gpu_ = is_gpu;
  slot.luid_ = luid;
  slot.status_ = Slot::Status::Pending;
  return slot;
}

/* --------------------------------------------- */
/* --------------------------------------------- */
/* --------------------------------------------- */

GGEMSProgressBar::Slot &
GGEMSProgressBar::Slot::SetBatchesDone(std::uint64_t done) noexcept {
  batches_done_.store(done, std::memory_order_relaxed);
  return *this;
}

/* --------------------------------------------- */
/* --------------------------------------------- */
/* --------------------------------------------- */

GGEMSProgressBar::Slot &
GGEMSProgressBar::Slot::SetBatchesTotal(std::uint64_t total) noexcept {
  batches_total_.store(total, std::memory_order_relaxed);
  return *this;
}

/* --------------------------------------------- */
/* --------------------------------------------- */
/* --------------------------------------------- */

GGEMSProgressBar::Slot &
GGEMSProgressBar::Slot::SetParticleType(ParticleType p) noexcept {
  particle_type_ = p;
  return *this;
}

/* --------------------------------------------- */
/* --------------------------------------------- */
/* --------------------------------------------- */

GGEMSProgressBar::Slot &
GGEMSProgressBar::Slot::SetKernelName(std::string_view kernel_name) noexcept {
  kernel_name_ = kernel_name;
  return *this;
}

/* --------------------------------------------- */
/* --------------------------------------------- */
/* --------------------------------------------- */

GGEMSProgressBar::Slot &GGEMSProgressBar::Slot::SetIsGPU(bool is_gpu) noexcept {
  is_gpu_ = is_gpu;
  return *this;
}

/* --------------------------------------------- */
/* --------------------------------------------- */
/* --------------------------------------------- */

GGEMSProgressBar::Slot &
GGEMSProgressBar::Slot::SetETAPicoseconds(std::uint64_t eta_ps) noexcept {
  eta_ps_.store(eta_ps, std::memory_order_relaxed);
  return *this;
}

/* --------------------------------------------- */
/* --------------------------------------------- */
/* --------------------------------------------- */

GGEMSProgressBar::Slot &GGEMSProgressBar::Slot::SetBandwidthBytesPerPicosecond(
    long double bandwidth_byte_per_ps) noexcept {
  bandwidth_byte_per_ps_.store(bandwidth_byte_per_ps,
                               std::memory_order_relaxed);
  return *this;
}

/* --------------------------------------------- */
/* --------------------------------------------- */
/* --------------------------------------------- */

GGEMSProgressBar::Slot &
GGEMSProgressBar::Slot::SetStatus(Status status) noexcept {
  status_ = status;
  return *this;
}

// ----- Progress Bar ------------------------------

/* --------------------------------------------- */
/* --------------------------------------------- */
/* --------------------------------------------- */

GGEMSProgressBar::~GGEMSProgressBar() { Stop(); }

/* --------------------------------------------- */
/* --------------------------------------------- */
/* --------------------------------------------- */

GGEMSProgressBar::Slot &GGEMSProgressBar::GetSlot(std::size_t index) noexcept {
  return slots_[index];
}

/* --------------------------------------------- */
/* --------------------------------------------- */
/* --------------------------------------------- */

void GGEMSProgressBar::Start() {
  bool expected = false;
  if (!running_.compare_exchange_strong(expected, true,
                                        std::memory_order_acq_rel)) {
    return;
  }

  if (worker_.joinable()) {
    worker_.request_stop();
    worker_.join();
  }

  worker_ = std::jthread{[this](std::stop_token st) { RenderLoop(st); }};
  DisableTerminal();
}

/* --------------------------------------------- */
/* --------------------------------------------- */
/* --------------------------------------------- */

void GGEMSProgressBar::Stop() {
  using namespace std::chrono_literals;
  bool expected = true;
  if (!running_.compare_exchange_strong(expected, false,
                                        std::memory_order_acq_rel)) {
    return;
  }

  if (worker_.joinable()) {
    worker_.request_stop();
    worker_.join();
  }

  std::this_thread::sleep_for(1000ms);
  EnableTerminal();
}

/* --------------------------------------------- */
/* --------------------------------------------- */
/* --------------------------------------------- */

void GGEMSProgressBar::SetFrameRate(
    std::chrono::milliseconds min_frame_time,
    std::chrono::milliseconds max_frame_time) noexcept {
  if (min_frame_time <= std::chrono::milliseconds{0}) {
    min_frame_time = std::chrono::milliseconds{50};
  }
  if (max_frame_time < min_frame_time) {
    max_frame_time = min_frame_time;
  }

  min_frame_time_ = min_frame_time;
  max_frame_time_ = max_frame_time;
}

/* --------------------------------------------- */
/* --------------------------------------------- */
/* --------------------------------------------- */

void GGEMSProgressBar::DisableTerminal() {
  std::cout << "\033[?1049h";   // Alternative screen
  std::cout << "\033[2J\033[H"; // Clean screen and cursor to [0;0] position
  std::cout << "\033[?12l";     // Deactivate blinking cursor
  std::cout << "\033[H";
  std::cout << "\033[?25l"; // Deactivate cursor
}

/* --------------------------------------------- */
/* --------------------------------------------- */
/* --------------------------------------------- */

void GGEMSProgressBar::EnableTerminal() {
  std::cout << "\033[?1049l"; // Go back to terminal
  std::cout << "\033[?12h";   // Activate blinking cursor
  std::cout << "\033[?25h";   // Visible cursor
}

/* --------------------------------------------- */
/* --------------------------------------------- */
/* --------------------------------------------- */

void GGEMSProgressBar::RenderLoop(std::stop_token st) {
  using namespace std::chrono_literals;

  bool const &use_color = GGEMSLogger::GetInstance().UseColour();
  framebuffer_.SetUseColour(use_color);

  std::this_thread::sleep_for(10ms);

  while (!st.stop_requested() && running_.load(std::memory_order_acquire)) {
    Draw();
    ++frame_counter_;

    auto dynamic_frame_time = ComputeFrameTime();
    std::this_thread::sleep_for(dynamic_frame_time);
  }

  Draw();
}

/* --------------------------------------------- */
/* --------------------------------------------- */
/* --------------------------------------------- */

void GGEMSProgressBar::Draw() {
  if (slots_.empty()) {
    return;
  }

  PrepareFrame();
  framebuffer_.Clear(U' ', render::DEFAULT_FG);

  DrawHeader();
  DrawSlots();
  DrawSystemStats();

  FlushFrame();
}

/* --------------------------------------------- */
/* --------------------------------------------- */
/* --------------------------------------------- */

void GGEMSProgressBar::DrawHeader() {
  auto const &g = utf::Glyphs();
  auto &fb = framebuffer_;

  // Line 0
  fb.DrawRectBorder(center_x_, center_y_, content_width_, frame_height_,
                    render::DEFAULT_FG);

  // Line 1:
  std::u32string title_bar{g.title_bar};
  std::u32string title = title_bar + U" GGEMS - Particle Monitor " + title_bar;
  std::int16_t title_x = static_cast<std::int16_t>(
      (content_width_ - static_cast<std::int16_t>(title.size())) / 2);

  fb.DrawString(center_x_ + title_x, center_y_ + 1, title);
  fb.DrawChar(center_x_, center_y_ + 2, g.border_right);
  fb.DrawChar(static_cast<std::int16_t>(center_x_ + content_width_ - 1),
              static_cast<std::int16_t>(center_y_ + 2), g.border_left);
  fb.DrawHLine(center_x_ + 1, center_y_ + 2, content_width_ - 2, g.separator);
}

/* --------------------------------------------- */
/* --------------------------------------------- */
/* --------------------------------------------- */

void GGEMSProgressBar::PrepareFrame() {
  std::int16_t slot_rows =
      rows_per_slot_ * static_cast<std::int16_t>(slots_.size());

  framebuffer_.UpdateSizeIfNeeded();

  frame_height_ =
      static_cast<std::int16_t>(header_rows_ + slot_rows + footer_rows_);

  std::int16_t width = framebuffer_.GetWidth();
  std::int16_t height = framebuffer_.GetHeight();

  center_x_ = std::max<std::int16_t>(
      static_cast<std::int16_t>((width - content_width_) / 2), 0);
  center_y_ = std::max<std::int16_t>(
      static_cast<std::int16_t>((height - frame_height_) / 2), 0);

  framebuffer_.Resize(width, height);
}

/* --------------------------------------------- */
/* --------------------------------------------- */
/* --------------------------------------------- */

void GGEMSProgressBar::DrawSlots() {
  std::int16_t base_y = 4;

  for (std::size_t i = 0; i < slots_.size(); ++i) {
    DrawSingleSlot(i, base_y);
    base_y = static_cast<std::int16_t>(base_y + rows_per_slot_);
  }
}

void GGEMSProgressBar::DrawSingleSlot(std::size_t index, std::int16_t base_y) {
  auto const &g = utf::Glyphs();
  auto &fb = framebuffer_;
  auto const &s = slots_[index];
  bool const is_gpu = s.is_gpu_;

  render::ColourKey dev_colour =
      is_gpu ? render::GREEN_Neon : render::BLUE_Azure;
  render::ColourKey bar_colour =
      is_gpu ? render::GREEN_Neon_B : render::BLUE_Azure_B;

  // Line 0 : [Tn] Device
  {
    std::string header = std::format("[T{}] {}", index + 1, s.name_);
    std::u32string header32 = utf::UTF8ToUTF32(header);

    framebuffer_.DrawString(center_x_ + 2, center_y_ + base_y, header32,
                            dev_colour);
  }

  // Line 1 : Status + Bar + % + pulse + type + status text
  {
    std::int16_t y = base_y + 1;

    framebuffer_.DrawString(center_x_ + 2, center_y_ + y, U"Status:");

    // Building progress bar
    std::uint64_t done = s.batches_done_.load(std::memory_order_relaxed);
    std::uint64_t total = s.batches_total_.load(std::memory_order_relaxed);

    float progress = (total > 0ULL)
                         ? static_cast<float>(done) / static_cast<float>(total)
                         : 0.0f;

    auto bar_cells = BuildBar(progress);
    std::int16_t bar_x = 10;

    std::int16_t cx = bar_x;
    for (char32_t c : bar_cells) {
      fb.DrawChar(center_x_ + cx, center_y_ + y, c, bar_colour);
      ++cx;
    }

    // Percentage
    std::u32string pct32 = FormatPercentage(progress);
    fb.DrawString(static_cast<std::int16_t>(bar_x + 42 + center_x_),
                  center_y_ + y, pct32);

    // Pulse
    using diff_t = std::vector<char32_t>::difference_type;
    const diff_t shift = static_cast<diff_t>(frame_counter_ % 12);

    std::vector<char32_t> pulse = BuildPulse(s.particle_type_);
    std::rotate(pulse.begin(), pulse.end() - shift, pulse.end());
    render::ColourKey particle_color = Slot::ParticleColour(s.particle_type_);

    for (std::int16_t i = 0; auto const &p : pulse) {
      fb.DrawChar(static_cast<std::int16_t>(center_x_ + bar_x + 51 + i),
                  center_y_ + y, p, particle_color);
      ++i;
    }

    // Particle name
    std::int16_t pulse_size = static_cast<std::int16_t>(pulse.size());
    std::u32string particle_name = Slot::ParticleName(s.particle_type_);
    fb.DrawString(
        static_cast<std::int16_t>(center_x_ + bar_x + 51 + pulse_size + 1),
        center_y_ + y, particle_name, particle_color);
  }

  // Line 2
  {
    std::int16_t y = base_y + 2;
    fb.DrawChar(center_x_ + 2, center_y_ + y, g.sub_arrow);

    // long double bw =
    // s.bandwidth_byte_per_ps_.load(std::memory_order_relaxed);

    std::string bandwidth_kernel_txt =
        std::format("Kernel: {} ", s.kernel_name_);

    fb.DrawString(static_cast<std::int16_t>(center_x_ + 4), center_y_ + y,
                  utf::UTF8ToUTF32(bandwidth_kernel_txt));

    std::string status_txt =
        std::format("({})", utf::UTF32ToUTF8(Slot::StatusName(s.status_)));
    fb.DrawString(static_cast<std::int16_t>(
                      center_x_ + 4 +
                      static_cast<int16_t>(bandwidth_kernel_txt.size()) + 2),
                  center_y_ + y, utf::UTF8ToUTF32(status_txt),
                  Slot::StatusColour(s.status_));
  }

  // Line 3
  {
    std::int16_t y = base_y + 3;
    fb.DrawChar(center_x_ + 4, center_y_ + y, g.sub_arrow);
    std::uint64_t eta_ps = s.eta_ps_.load(std::memory_order_relaxed);
    std::string bat_txt = std::format(
        "Batches: {} / {} | ETA: {}",
        s.batches_done_.load(std::memory_order_relaxed),
        s.batches_total_.load(std::memory_order_relaxed), FormatETA(eta_ps));
    fb.DrawString(center_x_ + 6, center_y_ + y, utf::UTF8ToUTF32(bat_txt));
  }

  // Line 4 Process stats
  {
    std::int16_t y = base_y + 4;

    if (!is_gpu) { // For CPU Process
      CPUProcessUsage process_usage = GetProcessUsage();

      // CPU Slot Process
      fb.DrawString(center_x_ + 2, center_y_ + y, U"CPU:");
      std::uint8_t cpu_proc_percent = process_usage.cpu_percent;
      std::string cpu_proc_percent_text =
          std::format("{:3}%", cpu_proc_percent);
      fb.DrawString(center_x_ + 7, center_y_ + y,
                    utf::UTF8ToUTF32(cpu_proc_percent_text),
                    GetColourStatus(cpu_proc_percent));

      // RAM Process, working set size and private usage
      std::uint64_t working_set = process_usage.ram.working_set_size;
      std::uint64_t private_usage = process_usage.ram.private_mem;
      std::string ram_txt =
          std::format("Working set: {}, Private: {}", FormatMemory(working_set),
                      FormatMemory(private_usage));
      fb.DrawString(center_x_ + 14, center_y_ + y, utf::UTF8ToUTF32(ram_txt));
    } else { // For GPU Process
      auto const &luid = slots_[index].luid_;
      GPUsage gpu_usage = GetGPUsage(luid);

      fb.DrawString(center_x_ + 2, center_y_ + y, U"GPU:");
      std::uint8_t gpu_proc_percent = gpu_usage.gpu_percent;
      std::string gpu_proc_percent_text =
          std::format("{:3}%", gpu_proc_percent);
      fb.DrawString(center_x_ + 7, center_y_ + y,
                    utf::UTF8ToUTF32(gpu_proc_percent_text),
                    GetColourStatus(gpu_proc_percent));

      fb.DrawString(center_x_ + 12, center_y_ + y, U"| RAM:");
      std::uint8_t ram_percent = gpu_usage.ram.percent;
      std::string ram_percent_text = std::format("{:3}%", ram_percent);
      fb.DrawString(center_x_ + 19, center_y_ + y,
                    utf::UTF8ToUTF32(ram_percent_text),
                    GetColourStatus(ram_percent));

      std::uint64_t ram_total = gpu_usage.ram.total;
      std::uint64_t ram_used = gpu_usage.ram.used;
      std::string ram_txt = std::format("({}/{})", FormatMemory(ram_used),
                                        FormatMemory(ram_total));
      fb.DrawString(center_x_ + 24, center_y_ + y, utf::UTF8ToUTF32(ram_txt));
    }
  }
}

/* --------------------------------------------- */
/* --------------------------------------------- */
/* --------------------------------------------- */

void GGEMSProgressBar::DrawSystemStats() {
  auto &fb = framebuffer_;
  std::int16_t base_y = static_cast<std::int16_t>(
      header_rows_ + static_cast<std::int16_t>(slots_.size()) * rows_per_slot_);

  SystemUsage system_usage = GetSystemUsage();

  // CPU global
  fb.DrawString(center_x_ + 2, base_y + center_y_, U"CPU:");
  std::uint8_t cpu_percent = system_usage.cpu_percent;
  std::string cpu_percent_text = std::format("{:3}%", cpu_percent);
  fb.DrawString(center_x_ + 7, base_y + center_y_,
                utf::UTF8ToUTF32(cpu_percent_text),
                GetColourStatus(cpu_percent));

  // RAM global
  fb.DrawString(center_x_ + 12, base_y + center_y_, U"| RAM:");
  std::uint8_t ram_percent = system_usage.ram.percent;
  std::string ram_percent_text = std::format("{:3}%", ram_percent);
  fb.DrawString(center_x_ + 19, base_y + center_y_,
                utf::UTF8ToUTF32(ram_percent_text),
                GetColourStatus(ram_percent));

  std::uint64_t ram_total = system_usage.ram.total;
  std::uint64_t ram_used = system_usage.ram.used;
  std::string ram_txt =
      std::format("({}/{})", FormatMemory(ram_used), FormatMemory(ram_total));
  fb.DrawString(center_x_ + 24, base_y + center_y_, utf::UTF8ToUTF32(ram_txt));
}

/* --------------------------------------------- */
/* --------------------------------------------- */
/* --------------------------------------------- */

void GGEMSProgressBar::FlushFrame() {
  using namespace std::chrono_literals;
  std::string out = framebuffer_.Render();
  std::cout << "\033[H" << out << std::flush;
}

/* --------------------------------------------- */
/* --------------------------------------------- */
/* --------------------------------------------- */

std::chrono::milliseconds GGEMSProgressBar::ComputeFrameTime() const noexcept {
  using namespace std::chrono_literals;

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
}

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

std::vector<char32_t>
GGEMSProgressBar::BuildPulse(Slot::ParticleType particle_type) {
  char32_t halo_small{utf::Glyphs().pulse1};
  char32_t halo_large{utf::Glyphs().pulse3};
  char32_t empty{utf::Glyphs().pulse2};

  char32_t particle{Slot::ParticleSymbol(particle_type)};

  char32_t sign = U' ';
  if (Slot::ParticleName(particle_type) == U"electron") {
    sign = utf::Glyphs().minus;
  } else if (Slot::ParticleName(particle_type) == U"positron") {
    sign = utf::Glyphs().plus;
  }

  std::vector<char32_t> out(13, empty);

  out[0] = empty;
  out[1] = empty;
  out[2] = empty;
  out[3] = halo_small;
  out[4] = halo_large;

  // bloc particule (5–7)
  out[5] = U' ';
  out[6] = particle;
  out[7] = sign;

  out[8] = halo_large;
  out[9] = halo_small;

  out[10] = empty;
  out[11] = empty;
  out[12] = empty;

  return out;
}

/* --------------------------------------------- */
/* --------------------------------------------- */
/* --------------------------------------------- */

render::ColourKey
GGEMSProgressBar::GetColourStatus(std::uint8_t percent) noexcept {
  if (percent < 33) {
    return render::GREEN_Neon;
  } else if (percent < 64) {
    return render::YELLOW_Neon;
  } else if (percent < 95) {
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
} // namespace ggems::core
