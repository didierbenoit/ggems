#include <pybind11/pybind11.h>

namespace py = pybind11;

void BindCore(py::module_ &module);
void BindOpenCL(py::module_ &module);
void BindRandom(py::module_ &module);
void BindRadionuclide(py::module_ &module);
void BindSource(py::module_ &module);
void BindRun(py::module_ &module);
void BindObserver(py::module_ &module);

#ifdef GGEMS_WITH_IMGUI
void BindGui(py::module_ &module);
#endif

PYBIND11_MODULE(ggems, module) {
  module.doc() = R"pbdoc(
    GGEMS — GPU Geant4-based Monte Carlo Simulations
    =================================================
    Modular C++ engine accelerated with OpenCL and
    exposed to Python via Pybind11.
  )pbdoc";

  auto core = module.def_submodule("core", "GGEMS core module");
  auto opencl = module.def_submodule("opencl", "GGEMS OpenCL module");
  auto random = module.def_submodule("rndm", "GGEMS random module");
  auto source = module.def_submodule("source", "GGEMS source module");
  auto run = module.def_submodule("run", "GGEMS run module");
  auto observer = module.def_submodule("observer", "GGEMS observer module");

#ifdef GGEMS_WITH_IMGUI
  auto gui = module.def_submodule("gui", "GGEMS graphical interface module");
#endif

  BindCore(core);
  BindOpenCL(opencl);
  BindRadionuclide(module);
  BindRandom(random);
  BindSource(source);
  module.attr("Source") = source.attr("GGEMSSource");
  BindRun(run);
  BindObserver(observer);

#ifdef GGEMS_WITH_IMGUI
  BindGui(gui);
#endif

  module.attr("__version__") = "2.0.0";
  module.attr("__author__") =
      py::make_tuple("Didier Benoit <didier.benoit@inserm.fr>",
                     "Julien Bert <julien.bert@univ-brest.fr>");
  module.attr("__license__") = "GPLv3";
}
