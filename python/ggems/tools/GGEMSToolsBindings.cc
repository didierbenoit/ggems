// ************************************************************************
// * This file is part of GGEMS.                                          *
// *                                                                      *
// * GGEMS is free software: you can redistribute it and/or modify        *
// * it under the terms of the GNU General Public License as published by *
// * the Free Software Foundation, either version 3 of the License, or    *
// * (at your option) any later version.                                  *
// *                                                                      *
// * GGEMS is distributed in the hope that it will be useful,             *
// * but WITHOUT ANY WARRANTY; without even the implied warranty of       *
// * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the        *
// * GNU General Public License for more details.                         *
// *                                                                      *
// * You should have received a copy of the GNU General Public License    *
// * along with GGEMS.  If not, see <https://www.gnu.org/licenses/>.      *
// *                                                                      *
// ************************************************************************

/*!
 * \file GGEMSToolsBindings.cc
 * \brief ...
 * \author
 * Julien BERT <julien.bert@univ-brest.fr>
 * Didier BENOIT <didier.benoit@inserm.fr>
 * \date 2025-10-07
 * \version 2.0
*/

/// \cond
#include <pybind11/pybind11.h>
/// \endcond

#include "GGEMS/tools/GGEMSLogger.hh"

void CallLog(void) {
  GGEMSLoggerManager::GetInstance();
  ; //GGEMSLoggerManager& logger = GGEMSLoggerManager::GetInstance();
}

namespace py = pybind11;

PYBIND11_MODULE(_tools, m, py::mod_gil_not_used(), py::multiple_interpreters::per_interpreter_gil()) {
  m.doc() = "Module calling tools ...";

  m.def("CallLog", &CallLog, "A function calling GGEMS Log");
}
