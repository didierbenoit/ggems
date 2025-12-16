#pragma once

/// \cond
#include <string_view>
/// \endcond

namespace ggems::render {
class GGEMSTerminalPresenter {
public:
  GGEMSTerminalPresenter() = default;
  ~GGEMSTerminalPresenter();

  GGEMSTerminalPresenter(GGEMSTerminalPresenter const &) = delete;
  GGEMSTerminalPresenter(GGEMSTerminalPresenter const &&) = delete;
  GGEMSTerminalPresenter &operator=(GGEMSTerminalPresenter const &) = delete;
  GGEMSTerminalPresenter &operator=(GGEMSTerminalPresenter const &&) = delete;

public:
  void Begin(bool use_alt_buffer = true) noexcept;
  void End() noexcept;
  void Present(std::string_view frame_utf8) noexcept;

private:
  bool started_{false};
  bool use_alt_buffer_{false};
};
} // namespace ggems::render
