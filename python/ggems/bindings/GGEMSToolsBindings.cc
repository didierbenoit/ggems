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

namespace py = pybind11;

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

void GGEMSInitTools(py::module_& m) {
  m.def("GGEMSVerbosity", &GGEMSVerbosity, "Setting the level of verbosity in GGEMS from 1 to 4");
}
