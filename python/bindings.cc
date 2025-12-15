#include <pybind11/pybind11.h>

namespace py = pybind11;

void BindCore(py::module_ &m);
void BindOpenCL(py::module_ &m);
void BindRun(py::module_ &m);

PYBIND11_MODULE(ggems, m) {
  m.doc() = R"pbdoc(
    GGEMS — GPU Geant4-based Monte Carlo Simulations
    =================================================
    Modular C++ engine accelerated with OpenCL and
    exposed to Python via Pybind11.
  )pbdoc";

  auto core = m.def_submodule("core", "GGEMS core module");
  auto opencl = m.def_submodule("opencl", "GGEMS OpenCL module");
  auto run = m.def_submodule("run", "GGEMS run module");

  BindCore(core);
  BindOpenCL(opencl);
  BindRun(run);

  m.attr("__version__") = "2.0.0";
  m.attr("__author__") =
      py::make_tuple("Didier Benoit <didier.benoit@inserm.fr>",
                     "Julien Bert <julien.bert@univ-brest.fr>");
  m.attr("__license__") = "GPLv3";
}
