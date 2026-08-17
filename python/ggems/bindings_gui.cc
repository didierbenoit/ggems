#ifdef GGEMS_WITH_IMGUI

#include <cstdint>
#include <limits>
#include <memory>
#include <string>

#include <pybind11/pybind11.h>

#include "GGEMS/GGEMSRun.hh"
#include "GGEMS/observer/GGEMSTransportObserver.hh"
#include "GGEMS/ui/GGEMSGuiApplication.hh"

namespace py = pybind11;

void BindGui(py::module_ &m) {
  py::class_<ggems::ui::GGEMSGuiApplication>(m, "GGEMSGuiApplication")

      .def(py::init<std::string, std::int32_t, std::int32_t>(),
           py::arg("title") = "GGEMS GuiMode", py::arg("width") = 1600,
           py::arg("height") = 900)

      .def("set_vulkan_device",
           py::overload_cast<std::string>(
               &ggems::ui::GGEMSGuiApplication::SetVulkanDevice),
           py::arg("selection"))

      .def(
          "set_vulkan_device",
          [](ggems::ui::GGEMSGuiApplication &application,
             std::int64_t enumeration_index) {
            if (enumeration_index < 0 ||
                enumeration_index >
                    static_cast<std::int64_t>(
                        std::numeric_limits<std::uint32_t>::max())) {
              throw py::value_error(
                  "Vulkan device enumeration index is outside uint32 range.");
            }
            application.SetVulkanDevice(
                static_cast<std::uint32_t>(enumeration_index));
          },
          py::arg("selection"))

      .def("initialize", &ggems::ui::GGEMSGuiApplication::Initialize)

      .def("run", &ggems::ui::GGEMSGuiApplication::Run,
           py::call_guard<py::gil_scoped_release>())

      .def("submit_last_run_source_snapshot",
           &ggems::ui::GGEMSGuiApplication::SubmitLastRunSourceSnapshot,
           py::arg("run"))

      .def(
          "submit_particle_traces_from_observer",
          [](ggems::ui::GGEMSGuiApplication &application,
             std::shared_ptr<
                 ggems::core::observer::GGEMSTransportObserver> const
                 &observer) {
            if (observer == nullptr) {
              throw py::value_error(
                  "submit_particle_traces_from_observer expects a non-null "
                  "GGEMSTransportObserver.");
            }
            application.SubmitParticleTracesFromObserver(*observer);
          },
          py::arg("observer"))

      .def("clear_particle_traces",
           &ggems::ui::GGEMSGuiApplication::ClearParticleTraces)

      .def("is_initialized", &ggems::ui::GGEMSGuiApplication::IsInitialized);
}

#endif
