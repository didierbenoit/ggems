#include <pybind11/pybind11.h>

#include "GGEMS/core/GGEMSLogger.hh"

namespace py = pybind11;

void GGEMSVerbosity(int level) {
  if (level > 4 || level < 0) level = 4;

/*  switch (level) {
    case 0:
    case 1:
      GGEMSLogger::GetInstance().SetLevelInfos(gglog::Level::INFO);
      break;
    case 2:
      GGEMSLogger::GetInstance().SetLevelInfos(gglog::Level::INFO2);
      break;
    case 3:
      GGEMSLogger::GetInstance().SetLevelInfos(gglog::Level::INFO3);
      break;
    default:
      GGEMSLogger::GetInstance().SetLevelInfos(gglog::Level::INFO4);
  }*/
}

void GGEMSInitCore(py::module_ &m) {
  m.def("ggems_verbosity", &GGEMSVerbosity, "Setting the level of verbosity in GGEMS from 1 to 4");
}
