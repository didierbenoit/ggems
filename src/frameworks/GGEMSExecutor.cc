#include "GGEMS/frameworks/GGEMSExecutor.hh"
#include "GGEMS/core/GGEMSLogger.hh"
#include "GGEMS/core/GGEMSMacros.hh"

namespace ggems::run {
GGEMSExecutor::GGEMSExecutor() {
  GGEMS_INFOEX("GGEMS", 3, "GGEMSExecutor created.");
}

GGEMSExecutor::~GGEMSExecutor() {
  GGEMS_INFOEX("GGEMS", 3, "GGEMSExecutor destroyed.");
}

void GGEMSExecutor::Initialize() { ; }

void GGEMSExecutor::Run() { ; }

void GGEMSExecutor::SelectDevices() { ; }

} // namespace ggems::run
