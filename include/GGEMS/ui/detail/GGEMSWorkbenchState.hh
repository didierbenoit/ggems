#pragma once

#include <cstdint>

#include "GGEMSSceneCamera.hh"
#include "GGEMS/render/GGEMSParticleTrace.hh"

namespace ggems::ui::detail {

struct GGEMSWorkbenchState {
  enum class SceneSelection : std::uint8_t {
    None,
    World,
    Sources,
    Volumes,
    Materials,
    Tracks,
  };

  struct ViewportRequest {
    bool visible{false};
    std::uint32_t width{0U};
    std::uint32_t height{0U};
  };

  GGEMSSceneCamera camera;

  bool show_output_panel{true};
  bool show_status_panel{true};
  bool show_scene_panel{true};
  bool show_viewport_panel{true};
  bool show_inspector_panel{true};
  bool show_axes{true};

  render::GGEMSParticleTraceVisibility trace_visibility{};
  SceneSelection selected_scene_item{SceneSelection::World};
  ViewportRequest viewport{};

  std::uint32_t source_presentation_revision{0U};
};

} // namespace ggems::ui::detail
