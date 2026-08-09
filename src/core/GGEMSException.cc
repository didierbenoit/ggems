// ************************************************************************
// ************************************************************************


#include <cstdio>
#include <cstdlib>
#include <exception>
#include <string_view>

#include "GGEMS/core/GGEMSException.hh"

namespace {
/* --------------------------------------------- */
/* --------------------------------------------- */
/* --------------------------------------------- */

auto WriteEmergencyDiagnostic(std::string_view prefix,
                              std::string_view message = {}) noexcept -> void {
  if (!prefix.empty()) {
    std::fwrite(prefix.data(), sizeof(char), prefix.size(), stderr);
  }
  if (!message.empty()) {
    std::fwrite(message.data(), sizeof(char), message.size(), stderr);
  }
  std::fputc('\n', stderr);
}
} // namespace

/* --------------------------------------------- */
/* --------------------------------------------- */
/* --------------------------------------------- */

namespace ggems::core {
void TerminateHandler() noexcept {
  try {
    auto exception = std::current_exception();
    if (exception) {
      try {
        std::rethrow_exception(exception);
      } catch (GGEMSExceptionBase const &caught_exception) {
        WriteEmergencyDiagnostic({}, caught_exception.what());
      } catch (std::exception const &caught_exception) {
        WriteEmergencyDiagnostic("[std::exception] ", caught_exception.what());
      } catch (...) {
        WriteEmergencyDiagnostic("[Unknown exception]");
      }
    } else {
      WriteEmergencyDiagnostic(
          "[GGEMSException] Terminate called with no active exception");
    }
  } catch (...) {
    std::fputs("[GGEMSException] Exception escaped TerminateHandler\n", stderr);
  }
  std::abort();
}
} // namespace ggems::core
