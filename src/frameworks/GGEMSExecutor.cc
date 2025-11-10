#include "GGEMS/frameworks/GGEMSExecutor.hh"
#include "GGEMS/core/GGEMSLogger.hh"
#include "GGEMS/core/GGEMSMacros.hh"

using namespace ggems;

GGEMSExecutor::GGEMSExecutor() {
  GGEMS_INFOEX("GGEMS", 3, "GGEMSExecutor created.");
}

GGEMSExecutor::~GGEMSExecutor() {
  GGEMS_INFOEX("GGEMS", 3, "GGEMSExecutor destroyed.");
}
