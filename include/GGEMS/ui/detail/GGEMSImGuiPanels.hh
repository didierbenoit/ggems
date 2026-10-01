#pragma once

#include <cstdint>

#include "GGEMSDeviceStatus.hh"
#include "GGEMSWorkbenchState.hh"
#include "GGEMS/sources/GGEMSSourceRunSnapshot.hh"

namespace ggems::ui::detail {

auto BuildStatusPanel(bool &show_window,
                      GGEMSDeviceStatusSnapshot const &device_status,
                      std::uint32_t swapchain_width,
                      std::uint32_t swapchain_height, bool show_axes,
                      bool particle_traces_visible) -> void;

auto BuildScenePanel(
  bool &show_window, GGEMSWorkbenchState &workbench,
  core::sources::GGEMSSourceRunSnapshot const *source_run_snapshot) -> void;

auto BuildInspectorPanel(bool &show_window,
                         GGEMSWorkbenchState::SceneSelection selection) -> void;

} // namespace ggems::ui::detail
