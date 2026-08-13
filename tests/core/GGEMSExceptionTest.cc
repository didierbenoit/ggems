#include <cstdint>
#include <string>
#include <string_view>

#include <gtest/gtest.h>

#include "GGEMS/core/GGEMSException.hh"

namespace {

[[noreturn]] auto ThrowRecoverableFromKnownLine(std::int32_t &expected_line)
    -> void {
  expected_line = static_cast<std::int32_t>(__LINE__ + 1);
  throw ggems::core::GGEMSRecoverable{"source-location marker"};
}

auto ExpectDiagnosticContains(std::string_view diagnostic,
                              std::string_view expected) -> void {
  EXPECT_NE(diagnostic.find(expected), std::string_view::npos);
}

} // namespace

// =============================================================================
// =============================================================================

TEST(GGEMSExceptionTest, ReportsCategoryAndMessageInDiagnostic) {
  {
    ggems::core::GGEMSRecoverable const exception{"recoverable marker"};
    auto const diagnostic = std::string_view{exception.what()};

    ExpectDiagnosticContains(diagnostic, "Recoverable");
    ExpectDiagnosticContains(diagnostic, "recoverable marker");
  }

  {
    ggems::core::GGEMSInternal const exception{"internal marker"};
    auto const diagnostic = std::string_view{exception.what()};

    ExpectDiagnosticContains(diagnostic, "Internal");
    ExpectDiagnosticContains(diagnostic, "internal marker");
  }

  {
    ggems::core::GGEMSFatal const exception{"fatal marker"};
    auto const diagnostic = std::string_view{exception.what()};

    ExpectDiagnosticContains(diagnostic, "Fatal");
    ExpectDiagnosticContains(diagnostic, "fatal marker");
  }
}

// =============================================================================
// =============================================================================

TEST(GGEMSExceptionTest, CapturesDirectThrowSiteInDiagnostic) {
  std::int32_t expected_line = 0;

  try {
    ThrowRecoverableFromKnownLine(expected_line);
    FAIL() << "Expected GGEMSRecoverable.";
  } catch (ggems::core::GGEMSRecoverable const &exception) {
    auto const diagnostic = std::string_view{exception.what()};

    ExpectDiagnosticContains(diagnostic, "source-location marker");
    ExpectDiagnosticContains(diagnostic, "GGEMSExceptionTest.cc");
    ExpectDiagnosticContains(diagnostic, "ThrowRecoverableFromKnownLine");
    ExpectDiagnosticContains(diagnostic, std::to_string(expected_line));
  }
}
