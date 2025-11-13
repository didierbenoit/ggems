// ============================================================================
//  @file      bindings.cc
//  @brief     Pybind11 root bindings for the GGEMS Python module.
//  @details   Defines the Python entry point for the GGEMS C++ framework.
//             GGEMS (GPU Geant4-based Monte Carlo Simulations) provides
//             a modular, GPU-accelerated simulation core accessible from
//             Python via this unified interface.
//
//             The submodules declared here correspond to the major
//             architectural components of GGEMS:
//               - core     : Base utilities (logging, exceptions, local state)
//               - opencl   : GPU platform and device management
//               - runtime  : Program, kernel, and execution environment
//               - ui       : Future Vulkan/ImGui graphical interface
// ============================================================================

#include <pybind11/pybind11.h>

namespace py = pybind11;

// ---------------------------------------------------------------------------
// Forward declarations of submodule initialisers
// ---------------------------------------------------------------------------
void GGEMSInitCore(py::module_ &m);
void GGEMSInitOpenCL(py::module_ &m);
void GGEMSInitRun(py::module_ &m);

// ---------------------------------------------------------------------------
// GGEMS Python module definition
// ---------------------------------------------------------------------------
PYBIND11_MODULE(ggems, m) {
  m.doc() = R"pbdoc(
    GGEMS — GPU Geant4-based Monte Carlo Simulations
    =================================================
    Modular C++23 engine accelerated with OpenCL and
    exposed to Python via Pybind11.
  )pbdoc";

  // --- Submodules ---------------------------------------------------------
  auto core =
      m.def_submodule("core", "GGEMS core utilities and system helpers");
  auto opencl =
      m.def_submodule("opencl", "OpenCL platform and device management");
  auto run = m.def_submodule("run", "GGEMS executor");

  // --- Initialise C++ bindings for each submodule -------------------------
  GGEMSInitCore(core);
  GGEMSInitOpenCL(opencl);
  GGEMSInitRun(run);

  m.attr("GGEMSLogger") = core.attr("GGEMSLogger");
  m.attr("set_detail_level") = core.attr("set_detail_level");
  m.attr("force_color") = core.attr("force_color");
  m.attr("attach_sink") = core.attr("attach_sink");

  m.attr("GGEMSOpenCL") = opencl.attr("GGEMSOpenCL");
  m.attr("print_platforms") = opencl.attr("print_platforms");
  m.attr("print_devices") = opencl.attr("print_devices");
  m.attr("clean") = opencl.attr("clean");
  m.attr("initialise") = opencl.attr("initialise");
  m.attr("select_devices") = opencl.attr("select_devices");

  m.attr("GGEMSRun") = run.attr("GGEMSRun");

  // --- Optional version info (for Python side introspection) --------------
  m.attr("__version__") = "2.0.0";
  m.attr("__author__") = "Didier Benoit";
  m.attr("__license__") = "GPLv3";
}
