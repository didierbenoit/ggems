#include <gtest/gtest.h>

#include "GGEMS/ui/detail/GGEMSSceneCamera.hh"
#include "GGEMS/ui/detail/GGEMSWorkbenchState.hh"

namespace {

using Workbench = ggems::ui::detail::GGEMSWorkbenchState;

// =============================================================================
// =============================================================================

TEST(GGEMSWorkbenchState, DefaultsPresentEverythingWithCanonicalCamera) {
  Workbench const workbench{};

  EXPECT_TRUE(workbench.show_output_panel);
  EXPECT_TRUE(workbench.show_status_panel);
  EXPECT_TRUE(workbench.show_scene_panel);
  EXPECT_TRUE(workbench.show_viewport_panel);
  EXPECT_TRUE(workbench.show_inspector_panel);
  EXPECT_TRUE(workbench.show_axes);
  EXPECT_TRUE(workbench.trace_visibility.IsGlobalVisible());
  EXPECT_EQ(workbench.selected_scene_item, Workbench::SceneSelection::World);
  EXPECT_FALSE(workbench.viewport.visible);
  EXPECT_EQ(workbench.viewport.width, 0U);
  EXPECT_EQ(workbench.viewport.height, 0U);

  ggems::ui::detail::GGEMSSceneCamera reset_camera{};
  reset_camera.Orbit(30.0F, 10.0F);
  reset_camera.Reset();

  auto const initial_matrix = workbench.camera.BuildWorldToClipMatrix();
  auto const reset_matrix = reset_camera.BuildWorldToClipMatrix();
  EXPECT_EQ(initial_matrix.row_0, reset_matrix.row_0);
  EXPECT_EQ(initial_matrix.row_1, reset_matrix.row_1);
  EXPECT_EQ(initial_matrix.row_2, reset_matrix.row_2);
  EXPECT_EQ(initial_matrix.row_3, reset_matrix.row_3);
}

} // namespace
