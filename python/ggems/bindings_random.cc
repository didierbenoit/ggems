#include <memory>
#include <string>

#include <pybind11/pybind11.h>
#include <pybind11/stl.h>

#include "GGEMS/core/random/GGEMSRandom.hh"
#include "GGEMS/core/random/GGEMSRandomEngine.hh"

namespace py = pybind11;

void BindRandom(py::module_ &m) {
  using ggems::core::random::GGEMSRandom;
  using ggems::core::random::GGEMSRandomEngine;

  py::enum_<GGEMSRandomEngine>(m, "GGEMSRandomEngine")
      .value("JKISS", GGEMSRandomEngine::JKISS)
      .value("PCG32", GGEMSRandomEngine::PCG32)
      .value("Philox", GGEMSRandomEngine::Philox);

  py::class_<GGEMSRandom, std::shared_ptr<GGEMSRandom>>(m, "GGEMSRandom")
      .def(py::init<>())

      .def(
          "set_engine",
          [](GGEMSRandom &self, std::string const &engine) -> GGEMSRandom & {
            return self.SetEngine(engine);
          },
          py::arg("engine"), py::return_value_policy::reference_internal)

      .def(
          "set_seed",
          [](GGEMSRandom &self, std::uint64_t seed) -> GGEMSRandom & {
            return self.SetSeed(seed);
          },
          py::arg("seed"), py::return_value_policy::reference_internal)

      .def("verbose", &GGEMSRandom::Verbose)

      .def("__repr__", [](GGEMSRandom const &random) {
        return std::format("<GGEMSRandom engine='{}' seed={}>",
                           random.GetEngineName(), random.GetSeed());
      });
}
