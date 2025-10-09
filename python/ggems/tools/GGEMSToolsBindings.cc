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

#include <pybind11/pybind11.h>

#include "GGEMS/tools/GGEMSLogger.hh"

void GGEMSVerbosity(int level) {
  if (level > 4 || level < 0) level = 4;

  switch (level) {
    case 0:
    case 1:
      GGEMSLoggerManager::GetInstance().SetLevelInfos(gglog::Level::INFO);
      break;
    case 2:
      GGEMSLoggerManager::GetInstance().SetLevelInfos(gglog::Level::INFO2);
      break;
    case 3:
      GGEMSLoggerManager::GetInstance().SetLevelInfos(gglog::Level::INFO3);
      break;
    default:
      GGEMSLoggerManager::GetInstance().SetLevelInfos(gglog::Level::INFO4);
  }
}

void CallLog(void) {
  gglog::info() << "Test new GGEMS logger" << gglog::endl;
  gglog::info2() << "Test new GGEMS logger" << gglog::endl;
  gglog::info3() << "Test new GGEMS logger" << gglog::endl;
  gglog::info4() << "Test new GGEMS logger" << gglog::endl;
  gglog::debug() << "Test new GGEMS logger" << gglog::endl;
  gglog::warn() << "Test new GGEMS logger" << gglog::endl;
  gglog::err() << "Test new GGEMS logger" << gglog::endl;
  gglog::info("CLASS", "METHOD") << "Test new GGEMS logger" << gglog::endl;
  gglog::debug("CLASS", "METHOD") << "Test new GGEMS logger" << gglog::endl;
  gglog::err("CLASS", "METHOD") << "Test new GGEMS logger" << gglog::endl;
}

namespace py = pybind11;

PYBIND11_MODULE(_tools, m, py::mod_gil_not_used(), py::multiple_interpreters::per_interpreter_gil()) {
  m.doc() = "Module calling tools ...";

  m.def("GGEMSVerbosity", &GGEMSVerbosity, "Setting the level of verbosity in GGEMS from 1 to 4");
  m.def("CallLog", &CallLog, "A function calling GGEMS Log");
}
