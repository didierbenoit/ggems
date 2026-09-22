// *****************************************************************************
// * This file is part of GGEMS.                                               *
// *                                                                           *
// * SPDX-License-Identifier: GPL-3.0-or-later                                 *
// * Copyright (C) 2017-2026 CHRU de Brest, Université de Bretagne Occidentale,*
// * Inserm.                                                                   *
// *                                                                           *
// * GGEMS is free software: you can redistribute it and/or modify             *
// * it under the terms of the GNU General Public License as published by      *
// * the Free Software Foundation, either version 3 of the License, or         *
// * (at your option) any later version.                                       *
// *                                                                           *
// * GGEMS is distributed in the hope that it will be useful,                  *
// * but WITHOUT ANY WARRANTY; without even the implied warranty of            *
// * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the              *
// * GNU General Public License for more details.                              *
// *                                                                           *
// * You should have received a copy of the GNU General Public License         *
// * along with GGEMS. If not, see <https://www.gnu.org/licenses/>.            *
// *****************************************************************************

/*!
 * \file
 * \brief XXX
 *
 * \author Julien BERT <julien.bert@univ-brest.fr>
 * \author Didier BENOIT <didier.benoit@inserm.fr>
 */

#ifdef GGEMS_WITH_IMGUI

#include <cstdint>
#include <string>

#include <pybind11/pybind11.h>

#include "GGEMS/GGEMSRun.hh"
#include "GGEMS/observer/GGEMSTransportObserver.hh"
#include "GGEMS/ui/GGEMSGuiApplication.hh"

namespace py = pybind11;

namespace {

constexpr std::int32_t k_default_window_width{1600};
constexpr std::int32_t k_default_window_height{900};

} // namespace

// =============================================================================
// =============================================================================

void BindGui(py::module_ &module) {
  // === === ===
  py::class_<ggems::ui::GGEMSGuiApplication>(module, "GGEMSGuiApplication")
    .def(py::init<std::string, std::int32_t, std::int32_t>(),
         py::arg("title") = "GGEMS GuiMode",
         py::arg("width") = k_default_window_width,
         py::arg("height") = k_default_window_height)

    .def("set_vulkan_device",
         py::overload_cast<std::string>(
           &ggems::ui::GGEMSGuiApplication::SetVulkanDevice),
         py::arg("selection"))

    .def("set_vulkan_device",
         py::overload_cast<std::uint32_t>(
           &ggems::ui::GGEMSGuiApplication::SetVulkanDevice),
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
         std::shared_ptr<ggems::core::observer::GGEMSTransportObserver> const
           &observer) -> void {
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
