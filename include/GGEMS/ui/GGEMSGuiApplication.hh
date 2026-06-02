#pragma once

#include <cstdint>
#include <string>

struct GLFWwindow;

namespace ggems::ui {

class GGEMSGuiApplication {
public:
  explicit GGEMSGuiApplication(std::string title = "GGEMS GuiMode",
                               std::int32_t width = 1600,
                               std::int32_t height = 900);
  ~GGEMSGuiApplication() noexcept;

  GGEMSGuiApplication(GGEMSGuiApplication const &) = delete;
  GGEMSGuiApplication(GGEMSGuiApplication &&) = delete;
  GGEMSGuiApplication &operator=(GGEMSGuiApplication const &) = delete;
  GGEMSGuiApplication &operator=(GGEMSGuiApplication &&) = delete;

public:
  void Initialise();
  void Run();

  [[nodiscard]] bool IsInitialised() const noexcept;

private:
  void Shutdown() noexcept;

private:
  std::string title_;
  std::int32_t width_{0};
  std::int32_t height_{0};
  GLFWwindow *window_{nullptr};
  bool glfw_initialised_{false};
};

} // namespace ggems::ui
