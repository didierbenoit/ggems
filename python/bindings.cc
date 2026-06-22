#include <pybind11/pybind11.h>

namespace py = pybind11;

void BindCore(py::module_ &m);
void BindOpenCL(py::module_ &m);
void BindRandom(py::module_ &m);
void BindRun(py::module_ &m);

#ifdef GGEMS_WITH_IMGUI
void BindGui(py::module_ &m);
#endif

PYBIND11_MODULE(ggems, m) {
  m.doc() = R"pbdoc(
    GGEMS — GPU Geant4-based Monte Carlo Simulations
    =================================================
    Modular C++ engine accelerated with OpenCL and
    exposed to Python via Pybind11.
  )pbdoc";

  auto core = m.def_submodule("core", "GGEMS core module");
  auto opencl = m.def_submodule("opencl", "GGEMS OpenCL module");
  auto random = m.def_submodule("rndm", "GGEMS random module");
  auto run = m.def_submodule("run", "GGEMS run module");

#ifdef GGEMS_WITH_IMGUI
  auto gui = m.def_submodule("gui", "GGEMS graphical interface module");
#endif

  BindCore(core);
  BindOpenCL(opencl);
  BindRandom(random);
  BindRun(run);

#ifdef GGEMS_WITH_IMGUI
  BindGui(gui);
#endif

  m.attr("__version__") = "2.0.0";
  m.attr("__author__") =
      py::make_tuple("Didier Benoit <didier.benoit@inserm.fr>",
                     "Julien Bert <julien.bert@univ-brest.fr>");
  m.attr("__license__") = "GPLv3";
}
